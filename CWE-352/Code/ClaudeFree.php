<?php
// ============================================================
// GEMS BANK - Digital Currency Banking System
// ============================================================

session_start();

// ---- DATABASE CONFIG ----
define('DB_HOST', 'localhost');
define('DB_USER', 'root');
define('DB_PASS', '');
define('DB_NAME', 'gems_bank');

// ---- DB CONNECTION ----
function getDB(): mysqli {
    $conn = new mysqli(DB_HOST, DB_USER, DB_PASS, DB_NAME);
    if ($conn->connect_error) {
        die("Connection failed: " . $conn->connect_error);
    }
    return $conn;
}

// ---- SETUP TABLES (run once) ----
function setupDatabase(): void {
    $conn = new mysqli(DB_HOST, DB_USER, DB_PASS);
    if ($conn->connect_error) die("Connection failed: " . $conn->connect_error);

    $conn->query("CREATE DATABASE IF NOT EXISTS `" . DB_NAME . "` DEFAULT CHARACTER SET utf8mb4");
    $conn->select_db(DB_NAME);

    $conn->query("CREATE TABLE IF NOT EXISTS users (
        id INT AUTO_INCREMENT PRIMARY KEY,
        username VARCHAR(50) NOT NULL UNIQUE,
        email VARCHAR(100) NOT NULL UNIQUE,
        password_hash VARCHAR(255) NOT NULL,
        gem_balance DECIMAL(15,2) NOT NULL DEFAULT 0.00,
        created_at DATETIME DEFAULT CURRENT_TIMESTAMP
    )");

    $conn->query("CREATE TABLE IF NOT EXISTS transactions (
        id INT AUTO_INCREMENT PRIMARY KEY,
        user_id INT NOT NULL,
        type ENUM('deposit','withdrawal','transfer_out','transfer_in') NOT NULL,
        amount DECIMAL(15,2) NOT NULL,
        counterpart_username VARCHAR(50) DEFAULT NULL,
        note TEXT DEFAULT NULL,
        created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
        FOREIGN KEY (user_id) REFERENCES users(id)
    )");

    $conn->close();
}

setupDatabase();

// ---- HELPERS ----
function sanitize(string $val): string {
    return htmlspecialchars(trim($val), ENT_QUOTES, 'UTF-8');
}

function isLoggedIn(): bool {
    return isset($_SESSION['user_id']);
}

function currentUser(): ?array {
    if (!isLoggedIn()) return null;
    $db = getDB();
    $stmt = $db->prepare("SELECT * FROM users WHERE id = ?");
    $stmt->bind_param("i", $_SESSION['user_id']);
    $stmt->execute();
    $result = $stmt->get_result()->fetch_assoc();
    $db->close();
    return $result;
}

function getTransactions(int $userId, int $limit = 10): array {
    $db = getDB();
    $stmt = $db->prepare("SELECT * FROM transactions WHERE user_id = ? ORDER BY created_at DESC LIMIT ?");
    $stmt->bind_param("ii", $userId, $limit);
    $stmt->execute();
    $rows = $stmt->get_result()->fetch_all(MYSQLI_ASSOC);
    $db->close();
    return $rows;
}

// ---- ACTION HANDLERS ----
$message = '';
$messageType = '';

// REGISTER
if ($_SERVER['REQUEST_METHOD'] === 'POST' && ($_POST['action'] ?? '') === 'register') {
    $username = trim($_POST['username'] ?? '');
    $email    = trim($_POST['email'] ?? '');
    $password = $_POST['password'] ?? '';
    $confirm  = $_POST['confirm_password'] ?? '';

    if (!$username || !$email || !$password) {
        $message = 'All fields are required.';
        $messageType = 'error';
    } elseif (!filter_var($email, FILTER_VALIDATE_EMAIL)) {
        $message = 'Invalid email address.';
        $messageType = 'error';
    } elseif (strlen($password) < 6) {
        $message = 'Password must be at least 6 characters.';
        $messageType = 'error';
    } elseif ($password !== $confirm) {
        $message = 'Passwords do not match.';
        $messageType = 'error';
    } else {
        $db   = getDB();
        $hash = password_hash($password, PASSWORD_DEFAULT);
        $stmt = $db->prepare("INSERT INTO users (username, email, password_hash) VALUES (?, ?, ?)");
        $stmt->bind_param("sss", $username, $email, $hash);
        if ($stmt->execute()) {
            $message = 'Account created! You can now log in.';
            $messageType = 'success';
        } else {
            $message = 'Username or email already exists.';
            $messageType = 'error';
        }
        $db->close();
    }
}

// LOGIN
if ($_SERVER['REQUEST_METHOD'] === 'POST' && ($_POST['action'] ?? '') === 'login') {
    $email    = trim($_POST['email'] ?? '');
    $password = $_POST['password'] ?? '';

    $db   = getDB();
    $stmt = $db->prepare("SELECT * FROM users WHERE email = ?");
    $stmt->bind_param("s", $email);
    $stmt->execute();
    $user = $stmt->get_result()->fetch_assoc();
    $db->close();

    if ($user && password_verify($password, $user['password_hash'])) {
        $_SESSION['user_id'] = $user['id'];
        header('Location: ' . $_SERVER['PHP_SELF']);
        exit;
    } else {
        $message = 'Invalid email or password.';
        $messageType = 'error';
    }
}

// LOGOUT
if (isset($_GET['action']) && $_GET['action'] === 'logout') {
    session_destroy();
    header('Location: ' . $_SERVER['PHP_SELF']);
    exit;
}

// DEPOSIT
if ($_SERVER['REQUEST_METHOD'] === 'POST' && ($_POST['action'] ?? '') === 'deposit' && isLoggedIn()) {
    $amount = floatval($_POST['amount'] ?? 0);
    if ($amount <= 0) {
        $message = 'Enter a valid deposit amount.';
        $messageType = 'error';
    } else {
        $db   = getDB();
        $uid  = $_SESSION['user_id'];
        $db->begin_transaction();
        try {
            $stmt = $db->prepare("UPDATE users SET gem_balance = gem_balance + ? WHERE id = ?");
            $stmt->bind_param("di", $amount, $uid);
            $stmt->execute();
            $stmt = $db->prepare("INSERT INTO transactions (user_id, type, amount, note) VALUES (?, 'deposit', ?, 'Manual deposit')");
            $stmt->bind_param("id", $uid, $amount);
            $stmt->execute();
            $db->commit();
            $message = number_format($amount, 2) . ' 💎 deposited successfully!';
            $messageType = 'success';
        } catch (Exception $e) {
            $db->rollback();
            $message = 'Deposit failed. Please try again.';
            $messageType = 'error';
        }
        $db->close();
    }
}

// WITHDRAW
if ($_SERVER['REQUEST_METHOD'] === 'POST' && ($_POST['action'] ?? '') === 'withdraw' && isLoggedIn()) {
    $amount = floatval($_POST['amount'] ?? 0);
    $user   = currentUser();

    if ($amount <= 0) {
        $message = 'Enter a valid withdrawal amount.';
        $messageType = 'error';
    } elseif ($amount > $user['gem_balance']) {
        $message = 'Insufficient gem balance.';
        $messageType = 'error';
    } else {
        $db  = getDB();
        $uid = $_SESSION['user_id'];
        $db->begin_transaction();
        try {
            $stmt = $db->prepare("UPDATE users SET gem_balance = gem_balance - ? WHERE id = ?");
            $stmt->bind_param("di", $amount, $uid);
            $stmt->execute();
            $stmt = $db->prepare("INSERT INTO transactions (user_id, type, amount, note) VALUES (?, 'withdrawal', ?, 'Manual withdrawal')");
            $stmt->bind_param("id", $uid, $amount);
            $stmt->execute();
            $db->commit();
            $message = number_format($amount, 2) . ' 💎 withdrawn successfully!';
            $messageType = 'success';
        } catch (Exception $e) {
            $db->rollback();
            $message = 'Withdrawal failed. Please try again.';
            $messageType = 'error';
        }
        $db->close();
    }
}

// TRANSFER
if ($_SERVER['REQUEST_METHOD'] === 'POST' && ($_POST['action'] ?? '') === 'transfer' && isLoggedIn()) {
    $amount     = floatval($_POST['amount'] ?? 0);
    $toUsername = trim($_POST['to_username'] ?? '');
    $user       = currentUser();

    if ($amount <= 0) {
        $message = 'Enter a valid transfer amount.';
        $messageType = 'error';
    } elseif (!$toUsername) {
        $message = 'Enter a recipient username.';
        $messageType = 'error';
    } elseif ($toUsername === $user['username']) {
        $message = 'You cannot transfer gems to yourself.';
        $messageType = 'error';
    } elseif ($amount > $user['gem_balance']) {
        $message = 'Insufficient gem balance.';
        $messageType = 'error';
    } else {
        $db   = getDB();
        $stmt = $db->prepare("SELECT * FROM users WHERE username = ?");
        $stmt->bind_param("s", $toUsername);
        $stmt->execute();
        $recipient = $stmt->get_result()->fetch_assoc();

        if (!$recipient) {
            $message = "User \"$toUsername\" not found.";
            $messageType = 'error';
        } else {
            $uid  = $_SESSION['user_id'];
            $rid  = $recipient['id'];
            $db->begin_transaction();
            try {
                $stmt = $db->prepare("UPDATE users SET gem_balance = gem_balance - ? WHERE id = ?");
                $stmt->bind_param("di", $amount, $uid);
                $stmt->execute();

                $stmt = $db->prepare("UPDATE users SET gem_balance = gem_balance + ? WHERE id = ?");
                $stmt->bind_param("di", $amount, $rid);
                $stmt->execute();

                $stmt = $db->prepare("INSERT INTO transactions (user_id, type, amount, counterpart_username, note) VALUES (?, 'transfer_out', ?, ?, 'Gem transfer sent')");
                $stmt->bind_param("ids", $uid, $amount, $toUsername);
                $stmt->execute();

                $senderUsername = $user['username'];
                $stmt = $db->prepare("INSERT INTO transactions (user_id, type, amount, counterpart_username, note) VALUES (?, 'transfer_in', ?, ?, 'Gem transfer received')");
                $stmt->bind_param("ids", $rid, $amount, $senderUsername);
                $stmt->execute();

                $db->commit();
                $message = number_format($amount, 2) . " 💎 sent to @$toUsername!";
                $messageType = 'success';
            } catch (Exception $e) {
                $db->rollback();
                $message = 'Transfer failed. Please try again.';
                $messageType = 'error';
            }
        }
        $db->close();
    }
}

// ---- FETCH CURRENT STATE ----
$user         = isLoggedIn() ? currentUser() : null;
$transactions = $user ? getTransactions($user['id']) : [];
$view         = $_GET['view'] ?? 'dashboard';
?>
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>💎 GemVault — Digital Currency Bank</title>
<link rel="preconnect" href="https://fonts.googleapis.com">
<link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
<link href="https://fonts.googleapis.com/css2?family=Cinzel+Decorative:wght@700&family=Rajdhani:wght@300;400;500;600;700&display=swap" rel="stylesheet">
<style>
  /* ===== RESET & BASE ===== */
  *, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }
  :root {
    --bg-deep:    #07090f;
    --bg-card:    #0d1220;
    --bg-panel:   #111827;
    --border:     rgba(100,160,255,0.12);
    --border-glow:rgba(120,200,255,0.35);
    --gem:        #4af0c8;
    --gem-alt:    #38bdf8;
    --gem-dark:   #0a7a5c;
    --gold:       #f5c842;
    --red:        #ff5470;
    --text:       #e2e8f0;
    --muted:      #64748b;
    --success-bg: rgba(74,240,200,0.08);
    --error-bg:   rgba(255,84,112,0.08);
    --radius:     12px;
    --glow:       0 0 30px rgba(74,240,200,0.18);
  }

  html, body {
    height: 100%;
    background: var(--bg-deep);
    color: var(--text);
    font-family: 'Rajdhani', sans-serif;
    font-size: 16px;
    line-height: 1.5;
  }

  /* ===== ANIMATED BACKGROUND ===== */
  body::before {
    content: '';
    position: fixed; inset: 0; z-index: 0;
    background:
      radial-gradient(ellipse 80% 60% at 20% 10%, rgba(74,240,200,0.07) 0%, transparent 60%),
      radial-gradient(ellipse 60% 40% at 80% 80%, rgba(56,189,248,0.06) 0%, transparent 50%),
      radial-gradient(ellipse 50% 50% at 50% 50%, rgba(245,200,66,0.03) 0%, transparent 60%);
    pointer-events: none;
  }
  body::after {
    content: '';
    position: fixed; inset: 0; z-index: 0;
    background-image:
      linear-gradient(rgba(74,240,200,0.03) 1px, transparent 1px),
      linear-gradient(90deg, rgba(74,240,200,0.03) 1px, transparent 1px);
    background-size: 60px 60px;
    pointer-events: none;
  }

  /* ===== LAYOUT ===== */
  .app { position: relative; z-index: 1; min-height: 100vh; display: flex; flex-direction: column; }

  /* ===== HEADER ===== */
  header {
    display: flex; align-items: center; justify-content: space-between;
    padding: 18px 40px;
    border-bottom: 1px solid var(--border);
    background: rgba(7,9,15,0.8);
    backdrop-filter: blur(20px);
    position: sticky; top: 0; z-index: 100;
  }
  .logo {
    font-family: 'Cinzel Decorative', cursive;
    font-size: 1.35rem;
    color: var(--gem);
    text-shadow: 0 0 20px rgba(74,240,200,0.5);
    letter-spacing: 0.04em;
  }
  .logo span { color: var(--gold); }
  .header-right { display: flex; align-items: center; gap: 16px; }
  .user-badge {
    display: flex; align-items: center; gap: 10px;
    padding: 8px 16px;
    background: var(--bg-card);
    border: 1px solid var(--border);
    border-radius: 999px;
    font-size: 0.85rem;
    font-weight: 600;
    color: var(--gem);
    letter-spacing: 0.05em;
  }
  .user-badge .avatar {
    width: 28px; height: 28px;
    background: linear-gradient(135deg, var(--gem), var(--gem-alt));
    border-radius: 50%;
    display: flex; align-items: center; justify-content: center;
    font-size: 0.75rem; font-weight: 700; color: #07090f;
  }

  /* ===== BUTTONS ===== */
  .btn {
    display: inline-flex; align-items: center; justify-content: center; gap: 8px;
    padding: 10px 22px;
    border: none; border-radius: 8px;
    font-family: 'Rajdhani', sans-serif;
    font-size: 0.9rem; font-weight: 700;
    letter-spacing: 0.08em; text-transform: uppercase;
    cursor: pointer; text-decoration: none;
    transition: all 0.2s ease;
  }
  .btn-gem {
    background: linear-gradient(135deg, var(--gem), var(--gem-alt));
    color: #07090f;
    box-shadow: 0 4px 15px rgba(74,240,200,0.3);
  }
  .btn-gem:hover { transform: translateY(-2px); box-shadow: 0 6px 24px rgba(74,240,200,0.5); }
  .btn-outline {
    background: transparent;
    color: var(--gem);
    border: 1px solid var(--border-glow);
  }
  .btn-outline:hover { background: rgba(74,240,200,0.06); border-color: var(--gem); }
  .btn-danger {
    background: rgba(255,84,112,0.12);
    color: var(--red);
    border: 1px solid rgba(255,84,112,0.3);
  }
  .btn-danger:hover { background: rgba(255,84,112,0.22); }
  .btn-ghost {
    background: transparent; color: var(--muted);
    border: 1px solid var(--border);
    font-size: 0.8rem;
  }
  .btn-ghost:hover { color: var(--text); border-color: var(--border-glow); }
  .btn-sm { padding: 6px 14px; font-size: 0.78rem; }
  .btn-full { width: 100%; }

  /* ===== AUTH PAGES ===== */
  .auth-wrap {
    flex: 1; display: flex; align-items: center; justify-content: center;
    padding: 40px 20px;
  }
  .auth-card {
    width: 100%; max-width: 440px;
    background: var(--bg-card);
    border: 1px solid var(--border);
    border-radius: 20px;
    overflow: hidden;
    box-shadow: 0 20px 60px rgba(0,0,0,0.5), 0 0 0 1px rgba(74,240,200,0.05);
  }
  .auth-header {
    padding: 36px 36px 0;
    text-align: center;
  }
  .auth-gem-icon {
    font-size: 3.5rem;
    display: block;
    margin-bottom: 12px;
    filter: drop-shadow(0 0 20px rgba(74,240,200,0.6));
    animation: float 3s ease-in-out infinite;
  }
  @keyframes float {
    0%,100% { transform: translateY(0); }
    50% { transform: translateY(-8px); }
  }
  .auth-title {
    font-family: 'Cinzel Decorative', cursive;
    font-size: 1.2rem;
    color: var(--gem);
    text-shadow: 0 0 15px rgba(74,240,200,0.4);
    margin-bottom: 6px;
  }
  .auth-sub { color: var(--muted); font-size: 0.88rem; }
  .auth-body { padding: 28px 36px 36px; }
  .auth-tabs {
    display: flex; gap: 4px;
    background: var(--bg-panel);
    padding: 4px; border-radius: 10px;
    margin-bottom: 24px;
  }
  .tab-btn {
    flex: 1;
    padding: 9px;
    background: transparent; border: none;
    border-radius: 8px;
    font-family: 'Rajdhani', sans-serif;
    font-size: 0.85rem; font-weight: 700;
    letter-spacing: 0.06em; text-transform: uppercase;
    color: var(--muted); cursor: pointer;
    transition: all 0.2s;
  }
  .tab-btn.active {
    background: var(--bg-card);
    color: var(--gem);
    box-shadow: 0 2px 8px rgba(0,0,0,0.3);
  }

  /* ===== FORMS ===== */
  .form-group { margin-bottom: 18px; }
  label {
    display: block;
    font-size: 0.78rem; font-weight: 600;
    letter-spacing: 0.1em; text-transform: uppercase;
    color: var(--muted);
    margin-bottom: 8px;
  }
  input[type="text"],
  input[type="email"],
  input[type="password"],
  input[type="number"] {
    width: 100%;
    padding: 12px 16px;
    background: var(--bg-panel);
    border: 1px solid var(--border);
    border-radius: 8px;
    color: var(--text);
    font-family: 'Rajdhani', sans-serif;
    font-size: 1rem; font-weight: 500;
    transition: all 0.2s;
    outline: none;
  }
  input:focus {
    border-color: var(--gem);
    box-shadow: 0 0 0 3px rgba(74,240,200,0.12);
    background: rgba(74,240,200,0.03);
  }
  input::placeholder { color: var(--muted); }
  .input-prefix {
    position: relative;
  }
  .input-prefix span {
    position: absolute; left: 14px; top: 50%; transform: translateY(-50%);
    color: var(--gem); font-size: 1rem; pointer-events: none;
  }
  .input-prefix input { padding-left: 36px; }

  /* ===== MESSAGES ===== */
  .alert {
    padding: 12px 16px;
    border-radius: 8px;
    font-size: 0.9rem; font-weight: 600;
    margin-bottom: 20px;
    display: flex; align-items: center; gap: 10px;
  }
  .alert-success {
    background: var(--success-bg);
    border: 1px solid rgba(74,240,200,0.25);
    color: var(--gem);
  }
  .alert-error {
    background: var(--error-bg);
    border: 1px solid rgba(255,84,112,0.25);
    color: var(--red);
  }

  /* ===== DASHBOARD ===== */
  .dashboard {
    max-width: 1100px; margin: 0 auto;
    padding: 40px 24px;
    flex: 1;
  }
  .dash-grid {
    display: grid;
    grid-template-columns: 340px 1fr;
    gap: 24px;
    align-items: start;
  }
  @media (max-width: 800px) {
    .dash-grid { grid-template-columns: 1fr; }
    header { padding: 14px 20px; }
    .dashboard { padding: 24px 16px; }
  }

  /* ===== BALANCE CARD ===== */
  .balance-card {
    background: var(--bg-card);
    border: 1px solid var(--border);
    border-radius: 20px;
    padding: 32px 28px;
    position: relative; overflow: hidden;
    box-shadow: var(--glow), 0 20px 50px rgba(0,0,0,0.4);
  }
  .balance-card::before {
    content: '💎';
    position: absolute; right: -20px; top: -20px;
    font-size: 7rem;
    opacity: 0.04;
    transform: rotate(-20deg);
  }
  .balance-label {
    font-size: 0.78rem; font-weight: 700;
    letter-spacing: 0.15em; text-transform: uppercase;
    color: var(--muted); margin-bottom: 10px;
  }
  .balance-amount {
    font-family: 'Cinzel Decorative', cursive;
    font-size: 2.6rem;
    color: var(--gem);
    text-shadow: 0 0 30px rgba(74,240,200,0.5);
    line-height: 1.1; margin-bottom: 4px;
  }
  .balance-unit {
    font-size: 0.85rem; color: var(--muted); font-weight: 600;
    letter-spacing: 0.08em; margin-bottom: 28px;
  }
  .balance-user {
    display: flex; align-items: center; gap: 12px;
    padding: 14px;
    background: var(--bg-panel);
    border-radius: 10px;
    border: 1px solid var(--border);
  }
  .balance-avatar {
    width: 40px; height: 40px;
    background: linear-gradient(135deg, var(--gem), var(--gem-alt));
    border-radius: 50%;
    display: flex; align-items: center; justify-content: center;
    font-size: 1rem; font-weight: 800; color: #07090f;
    flex-shrink: 0;
  }
  .balance-uname { font-weight: 700; font-size: 1rem; }
  .balance-email { font-size: 0.8rem; color: var(--muted); }
  .balance-member {
    margin-top: 14px;
    font-size: 0.75rem; color: var(--muted);
    letter-spacing: 0.06em;
  }

  /* ===== PANEL (actions + history) ===== */
  .panel {
    background: var(--bg-card);
    border: 1px solid var(--border);
    border-radius: 20px;
    overflow: hidden;
  }
  .panel-tabs {
    display: flex;
    background: var(--bg-panel);
    border-bottom: 1px solid var(--border);
  }
  .panel-tab {
    flex: 1;
    padding: 14px 8px;
    background: none; border: none;
    font-family: 'Rajdhani', sans-serif;
    font-size: 0.82rem; font-weight: 700;
    letter-spacing: 0.1em; text-transform: uppercase;
    color: var(--muted); cursor: pointer;
    border-bottom: 2px solid transparent;
    margin-bottom: -1px;
    transition: all 0.2s;
  }
  .panel-tab:hover { color: var(--text); }
  .panel-tab.active { color: var(--gem); border-bottom-color: var(--gem); }
  .panel-body { padding: 28px; }

  /* ===== ACTION FORMS ===== */
  .action-section { display: none; }
  .action-section.show { display: block; }
  .action-icon {
    font-size: 2rem; margin-bottom: 10px;
    filter: drop-shadow(0 0 10px rgba(74,240,200,0.4));
  }
  .action-title {
    font-family: 'Cinzel Decorative', cursive;
    font-size: 1rem; color: var(--gem);
    margin-bottom: 4px;
  }
  .action-sub { font-size: 0.85rem; color: var(--muted); margin-bottom: 24px; }

  /* ===== TRANSACTION LIST ===== */
  .tx-list { display: flex; flex-direction: column; gap: 10px; }
  .tx-item {
    display: flex; align-items: center; gap: 14px;
    padding: 14px 16px;
    background: var(--bg-panel);
    border: 1px solid var(--border);
    border-radius: 10px;
    transition: border-color 0.2s;
  }
  .tx-item:hover { border-color: var(--border-glow); }
  .tx-icon {
    width: 38px; height: 38px; border-radius: 50%;
    display: flex; align-items: center; justify-content: center;
    font-size: 1rem; flex-shrink: 0;
  }
  .tx-icon.in  { background: rgba(74,240,200,0.12); }
  .tx-icon.out { background: rgba(255,84,112,0.12); }
  .tx-info { flex: 1; min-width: 0; }
  .tx-note { font-weight: 600; font-size: 0.9rem; }
  .tx-meta { font-size: 0.75rem; color: var(--muted); margin-top: 2px; }
  .tx-amount {
    font-family: 'Cinzel Decorative', cursive;
    font-size: 0.95rem; font-weight: 700; flex-shrink: 0;
  }
  .tx-amount.in  { color: var(--gem); }
  .tx-amount.out { color: var(--red); }
  .tx-empty {
    text-align: center; padding: 48px 24px;
    color: var(--muted); font-size: 0.9rem;
  }
  .tx-empty span { font-size: 2.5rem; display: block; margin-bottom: 10px; opacity: 0.4; }

  /* ===== NAV PILLS ===== */
  .nav-logout {
    padding: 8px 18px;
    background: rgba(255,84,112,0.08);
    border: 1px solid rgba(255,84,112,0.2);
    border-radius: 8px;
    color: var(--red);
    font-family: 'Rajdhani', sans-serif;
    font-size: 0.8rem; font-weight: 700;
    letter-spacing: 0.08em; text-transform: uppercase;
    text-decoration: none;
    transition: all 0.2s;
  }
  .nav-logout:hover { background: rgba(255,84,112,0.18); }

  /* ===== DIVIDER ===== */
  .divider {
    height: 1px; background: var(--border);
    margin: 24px 0;
  }
  .or-divider {
    display: flex; align-items: center; gap: 12px;
    margin: 20px 0;
  }
  .or-divider::before, .or-divider::after {
    content: ''; flex: 1; height: 1px; background: var(--border);
  }
  .or-divider span { font-size: 0.75rem; color: var(--muted); letter-spacing: 0.1em; }

  /* ===== SHIMMER ANIMATION ===== */
  @keyframes shimmer {
    0% { background-position: -200% center; }
    100% { background-position: 200% center; }
  }
  .shimmer-text {
    background: linear-gradient(90deg, var(--gem), var(--gold), var(--gem));
    background-size: 200% auto;
    -webkit-background-clip: text;
    -webkit-text-fill-color: transparent;
    background-clip: text;
    animation: shimmer 3s linear infinite;
  }

  /* ===== GLOW PULSE ===== */
  @keyframes glow-pulse {
    0%,100% { box-shadow: 0 0 20px rgba(74,240,200,0.15); }
    50% { box-shadow: 0 0 40px rgba(74,240,200,0.35); }
  }
  .balance-card { animation: glow-pulse 4s ease-in-out infinite; }

  /* ===== QUICK STATS ===== */
  .quick-stats {
    display: grid; grid-template-columns: repeat(3, 1fr); gap: 10px;
    margin-bottom: 24px;
  }
  .stat-box {
    padding: 14px 12px;
    background: var(--bg-panel);
    border: 1px solid var(--border);
    border-radius: 10px;
    text-align: center;
  }
  .stat-val {
    font-family: 'Cinzel Decorative', cursive;
    font-size: 1.1rem; color: var(--gem);
  }
  .stat-key { font-size: 0.68rem; color: var(--muted); letter-spacing: 0.08em; text-transform: uppercase; margin-top: 4px; }
</style>
</head>
<body>
<div class="app">

<!-- ============ HEADER ============ -->
<header>
  <div class="logo">💎 Gem<span>Vault</span></div>
  <?php if ($user): ?>
  <div class="header-right">
    <div class="user-badge">
      <div class="avatar"><?= strtoupper(substr($user['username'], 0, 1)) ?></div>
      @<?= sanitize($user['username']) ?>
    </div>
    <a href="?action=logout" class="nav-logout">Logout</a>
  </div>
  <?php else: ?>
  <div style="font-size:0.8rem;color:var(--muted);letter-spacing:0.08em;">DIGITAL CURRENCY BANK</div>
  <?php endif; ?>
</header>

<!-- ============ AUTHENTICATED DASHBOARD ============ -->
<?php if ($user): ?>

<div class="dashboard">

  <?php if ($message): ?>
  <div class="alert alert-<?= $messageType === 'success' ? 'success' : 'error' ?>" style="margin-bottom:24px;">
    <?= $messageType === 'success' ? '✦' : '✗' ?>
    <?= sanitize($message) ?>
  </div>
  <?php endif; ?>

  <div class="dash-grid">

    <!-- LEFT: Balance Card -->
    <div>
      <div class="balance-card">
        <div class="balance-label">Current Balance</div>
        <div class="balance-amount shimmer-text"><?= number_format($user['gem_balance'], 2) ?></div>
        <div class="balance-unit">💎 GEMS</div>
        <div class="balance-user">
          <div class="balance-avatar"><?= strtoupper(substr($user['username'], 0, 1)) ?></div>
          <div>
            <div class="balance-uname">@<?= sanitize($user['username']) ?></div>
            <div class="balance-email"><?= sanitize($user['email']) ?></div>
          </div>
        </div>
        <div class="balance-member">Member since <?= date('M j, Y', strtotime($user['created_at'])) ?></div>
      </div>

      <?php
        $deposited = 0; $withdrawn = 0; $txCount = count($transactions);
        foreach ($transactions as $tx) {
          if (in_array($tx['type'], ['deposit','transfer_in'])) $deposited += $tx['amount'];
          else $withdrawn += $tx['amount'];
        }
      ?>
      <div class="quick-stats" style="margin-top:16px;">
        <div class="stat-box">
          <div class="stat-val"><?= $txCount ?></div>
          <div class="stat-key">Txns</div>
        </div>
        <div class="stat-box">
          <div class="stat-val" style="color:var(--gem);font-size:0.85rem;">+<?= number_format($deposited,0) ?></div>
          <div class="stat-key">In</div>
        </div>
        <div class="stat-box">
          <div class="stat-val" style="color:var(--red);font-size:0.85rem;">-<?= number_format($withdrawn,0) ?></div>
          <div class="stat-key">Out</div>
        </div>
      </div>
    </div>

    <!-- RIGHT: Actions + History -->
    <div class="panel">
      <div class="panel-tabs">
        <button class="panel-tab active" onclick="switchTab(this,'deposit-sec')">⬇ Deposit</button>
        <button class="panel-tab" onclick="switchTab(this,'withdraw-sec')">⬆ Withdraw</button>
        <button class="panel-tab" onclick="switchTab(this,'transfer-sec')">↗ Transfer</button>
        <button class="panel-tab" onclick="switchTab(this,'history-sec')">📜 History</button>
      </div>
      <div class="panel-body">

        <!-- DEPOSIT -->
        <div class="action-section show" id="deposit-sec">
          <div class="action-icon">📥</div>
          <div class="action-title">Deposit Gems</div>
          <div class="action-sub">Add gems to your vault balance.</div>
          <form method="POST">
            <input type="hidden" name="action" value="deposit">
            <div class="form-group">
              <label>Amount</label>
              <div class="input-prefix">
                <span>💎</span>
                <input type="number" name="amount" min="1" step="0.01" placeholder="0.00" required>
              </div>
            </div>
            <button type="submit" class="btn btn-gem btn-full">Deposit Gems</button>
          </form>
        </div>

        <!-- WITHDRAW -->
        <div class="action-section" id="withdraw-sec">
          <div class="action-icon">📤</div>
          <div class="action-title">Withdraw Gems</div>
          <div class="action-sub">Remove gems from your vault (max: <?= number_format($user['gem_balance'],2) ?> 💎).</div>
          <form method="POST">
            <input type="hidden" name="action" value="withdraw">
            <div class="form-group">
              <label>Amount</label>
              <div class="input-prefix">
                <span>💎</span>
                <input type="number" name="amount" min="0.01" max="<?= $user['gem_balance'] ?>" step="0.01" placeholder="0.00" required>
              </div>
            </div>
            <button type="submit" class="btn btn-gem btn-full">Withdraw Gems</button>
          </form>
        </div>

        <!-- TRANSFER -->
        <div class="action-section" id="transfer-sec">
          <div class="action-icon">🚀</div>
          <div class="action-title">Transfer Gems</div>
          <div class="action-sub">Send gems to another GemVault player.</div>
          <form method="POST">
            <input type="hidden" name="action" value="transfer">
            <div class="form-group">
              <label>Recipient Username</label>
              <input type="text" name="to_username" placeholder="@username" required>
            </div>
            <div class="form-group">
              <label>Amount</label>
              <div class="input-prefix">
                <span>💎</span>
                <input type="number" name="amount" min="0.01" max="<?= $user['gem_balance'] ?>" step="0.01" placeholder="0.00" required>
              </div>
            </div>
            <button type="submit" class="btn btn-gem btn-full">Send Gems</button>
          </form>
        </div>

        <!-- HISTORY -->
        <div class="action-section" id="history-sec">
          <div class="action-icon">📜</div>
          <div class="action-title">Transaction History</div>
          <div class="action-sub">Your last <?= count($transactions) ?> transactions.</div>
          <div class="tx-list">
            <?php if (empty($transactions)): ?>
            <div class="tx-empty">
              <span>💎</span>
              No transactions yet. Make your first deposit!
            </div>
            <?php else: foreach ($transactions as $tx):
              $isIn  = in_array($tx['type'], ['deposit','transfer_in']);
              $icons = ['deposit'=>'📥','withdrawal'=>'📤','transfer_out'=>'↗','transfer_in'=>'↙'];
              $icon  = $icons[$tx['type']] ?? '💎';
              $label = ['deposit'=>'Deposit','withdrawal'=>'Withdrawal','transfer_out'=>'Sent','transfer_in'=>'Received'][$tx['type']] ?? $tx['type'];
              $dir   = $isIn ? 'in' : 'out';
              $prefix = $isIn ? '+' : '−';
            ?>
            <div class="tx-item">
              <div class="tx-icon <?= $dir ?>"><?= $icon ?></div>
              <div class="tx-info">
                <div class="tx-note"><?= $label ?>
                  <?php if ($tx['counterpart_username']): ?>
                    <?= $isIn ? 'from' : 'to' ?> @<?= sanitize($tx['counterpart_username']) ?>
                  <?php endif; ?>
                </div>
                <div class="tx-meta"><?= date('M j, Y · H:i', strtotime($tx['created_at'])) ?></div>
              </div>
              <div class="tx-amount <?= $dir ?>"><?= $prefix ?><?= number_format($tx['amount'],2) ?> 💎</div>
            </div>
            <?php endforeach; endif; ?>
          </div>
        </div>

      </div>
    </div>

  </div>
</div>

<!-- ============ LOGIN / REGISTER ============ -->
<?php else: ?>

<div class="auth-wrap">
  <div class="auth-card">
    <div class="auth-header">
      <span class="auth-gem-icon">💎</span>
      <div class="auth-title">GemVault</div>
      <div class="auth-sub">The premier digital currency bank for game assets</div>
    </div>
    <div class="auth-body">

      <?php if ($message): ?>
      <div class="alert alert-<?= $messageType === 'success' ? 'success' : 'error' ?>">
        <?= $messageType === 'success' ? '✦' : '✗' ?> <?= sanitize($message) ?>
      </div>
      <?php endif; ?>

      <div class="auth-tabs">
        <button class="tab-btn active" id="tab-login" onclick="authTab('login')">Log In</button>
        <button class="tab-btn" id="tab-register" onclick="authTab('register')">Register</button>
      </div>

      <!-- LOGIN FORM -->
      <div id="login-form">
        <form method="POST">
          <input type="hidden" name="action" value="login">
          <div class="form-group">
            <label>Email Address</label>
            <input type="email" name="email" placeholder="you@example.com" required>
          </div>
          <div class="form-group">
            <label>Password</label>
            <input type="password" name="password" placeholder="••••••••" required>
          </div>
          <button type="submit" class="btn btn-gem btn-full">Enter Vault</button>
        </form>
        <div class="or-divider"><span>or</span></div>
        <p style="text-align:center;font-size:0.85rem;color:var(--muted);">
          No account? <a href="#" onclick="authTab('register')" style="color:var(--gem);font-weight:700;">Register free →</a>
        </p>
      </div>

      <!-- REGISTER FORM -->
      <div id="register-form" style="display:none;">
        <form method="POST">
          <input type="hidden" name="action" value="register">
          <div class="form-group">
            <label>Username</label>
            <input type="text" name="username" placeholder="CrystalWarrior" required maxlength="50">
          </div>
          <div class="form-group">
            <label>Email Address</label>
            <input type="email" name="email" placeholder="you@example.com" required>
          </div>
          <div class="form-group">
            <label>Password</label>
            <input type="password" name="password" placeholder="Min. 6 characters" required>
          </div>
          <div class="form-group">
            <label>Confirm Password</label>
            <input type="password" name="confirm_password" placeholder="Repeat password" required>
          </div>
          <button type="submit" class="btn btn-gem btn-full">Create Account</button>
        </form>
        <div class="or-divider"><span>or</span></div>
        <p style="text-align:center;font-size:0.85rem;color:var(--muted);">
          Already have an account? <a href="#" onclick="authTab('login')" style="color:var(--gem);font-weight:700;">Log in →</a>
        </p>
      </div>

    </div>
  </div>
</div>

<?php endif; ?>

</div><!-- /app -->

<script>
// Dashboard panel tabs
function switchTab(btn, secId) {
  document.querySelectorAll('.panel-tab').forEach(b => b.classList.remove('active'));
  document.querySelectorAll('.action-section').forEach(s => s.classList.remove('show'));
  btn.classList.add('active');
  document.getElementById(secId).classList.add('show');
}

// Auth tabs
function authTab(tab) {
  document.getElementById('login-form').style.display    = tab === 'login'    ? 'block' : 'none';
  document.getElementById('register-form').style.display = tab === 'register' ? 'block' : 'none';
  document.getElementById('tab-login').classList.toggle('active',    tab === 'login');
  document.getElementById('tab-register').classList.toggle('active', tab === 'register');
}

<?php if ($messageType === 'success' && $message && isset($_SESSION['user_id'])): ?>
// Auto-show history after a successful transaction
window.addEventListener('DOMContentLoaded', () => {
  const histBtn = document.querySelectorAll('.panel-tab')[3];
  if (histBtn) switchTab(histBtn, 'history-sec');
});
<?php endif; ?>

<?php if ($messageType === 'error' && strpos($message ?? '', 'password') !== false || strpos($message ?? '', 'email') !== false || strpos($message ?? '', 'Username') !== false): ?>
// If registration error, keep register tab open
window.addEventListener('DOMContentLoaded', () => authTab('register'));
<?php endif; ?>
</script>
</body>
</html>