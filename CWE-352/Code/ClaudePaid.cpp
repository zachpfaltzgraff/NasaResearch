<?php
session_start();

// ─── Database Configuration ───────────────────────────────────────────────
$DB_HOST = 'localhost';
$DB_USER = 'root';
$DB_PASS = '';
$DB_NAME = 'gem_bank';

function getDB() {
    global $DB_HOST, $DB_USER, $DB_PASS, $DB_NAME;
    $conn = new mysqli($DB_HOST, $DB_USER, $DB_PASS);
    if ($conn->connect_error) {
        die("Connection failed: " . $conn->connect_error);
    }
    // Create DB if not exists
    $conn->query("CREATE DATABASE IF NOT EXISTS `$DB_NAME`");
    $conn->select_db($DB_NAME);

    // Create tables
    $conn->query("
        CREATE TABLE IF NOT EXISTS users (
            id INT AUTO_INCREMENT PRIMARY KEY,
            username VARCHAR(50) UNIQUE NOT NULL,
            email VARCHAR(100) UNIQUE NOT NULL,
            password VARCHAR(255) NOT NULL,
            balance DECIMAL(15,2) DEFAULT 0.00,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    ");
    $conn->query("
        CREATE TABLE IF NOT EXISTS transactions (
            id INT AUTO_INCREMENT PRIMARY KEY,
            user_id INT NOT NULL,
            type ENUM('deposit','withdrawal','transfer_out','transfer_in') NOT NULL,
            amount DECIMAL(15,2) NOT NULL,
            related_user VARCHAR(50) DEFAULT NULL,
            note TEXT DEFAULT NULL,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (user_id) REFERENCES users(id)
        )
    ");
    return $conn;
}

// ─── Helpers ──────────────────────────────────────────────────────────────
function redirect($msg = '', $type = 'error') {
    if ($msg) $_SESSION['flash'] = ['msg' => $msg, 'type' => $type];
    header("Location: " . $_SERVER['PHP_SELF']);
    exit;
}

function isLoggedIn() {
    return isset($_SESSION['user_id']);
}

function currentUser($db) {
    $id = (int)$_SESSION['user_id'];
    $r = $db->query("SELECT * FROM users WHERE id = $id");
    return $r->fetch_assoc();
}

function getTransactions($db, $userId, $limit = 10) {
    $stmt = $db->prepare("SELECT * FROM transactions WHERE user_id = ? ORDER BY created_at DESC LIMIT ?");
    $stmt->bind_param("ii", $userId, $limit);
    $stmt->execute();
    return $stmt->get_result()->fetch_all(MYSQLI_ASSOC);
}

// ─── Actions ──────────────────────────────────────────────────────────────
$db = getDB();
$action = $_POST['action'] ?? $_GET['action'] ?? '';

// LOGOUT
if ($action === 'logout') {
    session_destroy();
    redirect('You have been logged out.', 'success');
}

// REGISTER
if ($action === 'register') {
    $username = trim($_POST['username'] ?? '');
    $email    = trim($_POST['email'] ?? '');
    $pass     = $_POST['password'] ?? '';
    $confirm  = $_POST['confirm'] ?? '';

    if (!$username || !$email || !$pass) redirect('All fields are required.');
    if ($pass !== $confirm) redirect('Passwords do not match.');
    if (strlen($username) < 3) redirect('Username must be at least 3 characters.');
    if (!filter_var($email, FILTER_VALIDATE_EMAIL)) redirect('Invalid email address.');

    $stmt = $db->prepare("SELECT id FROM users WHERE email = ? OR username = ?");
    $stmt->bind_param("ss", $email, $username);
    $stmt->execute();
    if ($stmt->get_result()->num_rows > 0) redirect('Email or username already taken.');

    $hash = password_hash($pass, PASSWORD_DEFAULT);
    $stmt = $db->prepare("INSERT INTO users (username, email, password, balance) VALUES (?, ?, ?, 100)");
    $stmt->bind_param("sss", $username, $email, $hash);
    if ($stmt->execute()) {
        redirect('Account created! You received 100 starter gems. Please log in.', 'success');
    } else {
        redirect('Registration failed. Please try again.');
    }
}

// LOGIN
if ($action === 'login') {
    $email = trim($_POST['email'] ?? '');
    $pass  = $_POST['password'] ?? '';

    $stmt = $db->prepare("SELECT * FROM users WHERE email = ?");
    $stmt->bind_param("s", $email);
    $stmt->execute();
    $user = $stmt->get_result()->fetch_assoc();

    if ($user && password_verify($pass, $user['password'])) {
        $_SESSION['user_id'] = $user['id'];
        redirect('Welcome back, ' . htmlspecialchars($user['username']) . '!', 'success');
    } else {
        redirect('Invalid email or password.');
    }
}

// DEPOSIT
if ($action === 'deposit' && isLoggedIn()) {
    $amount = (float)($_POST['amount'] ?? 0);
    if ($amount <= 0) redirect('Enter a valid deposit amount.');
    if ($amount > 1000000) redirect('Maximum single deposit is 1,000,000 gems.');

    $id = (int)$_SESSION['user_id'];
    $db->query("UPDATE users SET balance = balance + $amount WHERE id = $id");
    $stmt = $db->prepare("INSERT INTO transactions (user_id, type, amount, note) VALUES (?, 'deposit', ?, 'Gem deposit')");
    $stmt->bind_param("id", $id, $amount);
    $stmt->execute();
    redirect(number_format($amount) . ' gems deposited successfully!', 'success');
}

// WITHDRAW
if ($action === 'withdraw' && isLoggedIn()) {
    $amount = (float)($_POST['amount'] ?? 0);
    $id     = (int)$_SESSION['user_id'];
    $user   = currentUser($db);

    if ($amount <= 0) redirect('Enter a valid withdrawal amount.');
    if ($amount > $user['balance']) redirect('Insufficient gem balance.');

    $db->query("UPDATE users SET balance = balance - $amount WHERE id = $id");
    $stmt = $db->prepare("INSERT INTO transactions (user_id, type, amount, note) VALUES (?, 'withdrawal', ?, 'Gem withdrawal')");
    $stmt->bind_param("id", $id, $amount);
    $stmt->execute();
    redirect(number_format($amount) . ' gems withdrawn successfully!', 'success');
}

// TRANSFER
if ($action === 'transfer' && isLoggedIn()) {
    $amount     = (float)($_POST['amount'] ?? 0);
    $toUsername = trim($_POST['recipient'] ?? '');
    $id         = (int)$_SESSION['user_id'];
    $user       = currentUser($db);

    if ($amount <= 0) redirect('Enter a valid transfer amount.');
    if (!$toUsername) redirect('Enter a recipient username.');
    if (strtolower($toUsername) === strtolower($user['username'])) redirect('You cannot transfer gems to yourself.');
    if ($amount > $user['balance']) redirect('Insufficient gem balance.');

    $stmt = $db->prepare("SELECT * FROM users WHERE username = ?");
    $stmt->bind_param("s", $toUsername);
    $stmt->execute();
    $recipient = $stmt->get_result()->fetch_assoc();

    if (!$recipient) redirect('Recipient "' . htmlspecialchars($toUsername) . '" not found.');

    $db->begin_transaction();
    try {
        $db->query("UPDATE users SET balance = balance - $amount WHERE id = $id");
        $db->query("UPDATE users SET balance = balance + $amount WHERE id = {$recipient['id']}");

        $note = "Transfer to " . $recipient['username'];
        $stmt = $db->prepare("INSERT INTO transactions (user_id, type, amount, related_user, note) VALUES (?, 'transfer_out', ?, ?, ?)");
        $stmt->bind_param("idss", $id, $amount, $recipient['username'], $note);
        $stmt->execute();

        $note2 = "Transfer from " . $user['username'];
        $stmt2 = $db->prepare("INSERT INTO transactions (user_id, type, amount, related_user, note) VALUES (?, 'transfer_in', ?, ?, ?)");
        $stmt2->bind_param("idss", $recipient['id'], $amount, $user['username'], $note2);
        $stmt2->execute();

        $db->commit();
        redirect(number_format($amount) . ' gems sent to ' . htmlspecialchars($recipient['username']) . '!', 'success');
    } catch (Exception $e) {
        $db->rollback();
        redirect('Transfer failed. Please try again.');
    }
}

// ─── View Data ────────────────────────────────────────────────────────────
$flash    = $_SESSION['flash'] ?? null;
$showAuth = $_GET['view'] ?? 'login';
unset($_SESSION['flash']);

$user         = isLoggedIn() ? currentUser($db) : null;
$transactions = isLoggedIn() ? getTransactions($db, (int)$_SESSION['user_id'], 15) : [];
?>
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>GemBank — Digital Currency Vault</title>
<link href="https://fonts.googleapis.com/css2?family=Cinzel+Decorative:wght@700&family=Rajdhani:wght@400;500;600;700&family=Exo+2:ital,wght@0,300;0,400;0,600;1,300&display=swap" rel="stylesheet">
<style>
  :root {
    --gem:        #00e5ff;
    --gem-glow:   #00b8d9;
    --gem-deep:   #004d61;
    --gold:       #ffd54f;
    --gold-glow:  #ffab00;
    --bg:         #020b10;
    --panel:      #071520;
    --panel2:     #0b2032;
    --border:     rgba(0,229,255,0.15);
    --text:       #c9e8f0;
    --text-dim:   #6a9bac;
    --success:    #00e676;
    --error:      #ff5252;
    --radius:     12px;
  }

  *, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }

  html { scroll-behavior: smooth; }

  body {
    background: var(--bg);
    color: var(--text);
    font-family: 'Exo 2', sans-serif;
    font-size: 15px;
    min-height: 100vh;
    overflow-x: hidden;
  }

  /* Animated starfield background */
  body::before {
    content: '';
    position: fixed; inset: 0;
    background:
      radial-gradient(ellipse at 20% 80%, rgba(0,100,130,0.18) 0%, transparent 55%),
      radial-gradient(ellipse at 80% 20%, rgba(0,70,100,0.15) 0%, transparent 50%),
      radial-gradient(ellipse at 50% 50%, rgba(0,40,60,0.25) 0%, transparent 70%);
    z-index: 0;
    pointer-events: none;
  }

  body::after {
    content: '';
    position: fixed; inset: 0;
    background-image:
      radial-gradient(1px 1px at 15% 22%, rgba(255,255,255,0.45) 0%, transparent 100%),
      radial-gradient(1px 1px at 34% 67%, rgba(255,255,255,0.3) 0%, transparent 100%),
      radial-gradient(1.5px 1.5px at 52% 12%, rgba(0,229,255,0.5) 0%, transparent 100%),
      radial-gradient(1px 1px at 71% 45%, rgba(255,255,255,0.35) 0%, transparent 100%),
      radial-gradient(1px 1px at 88% 78%, rgba(255,255,255,0.4) 0%, transparent 100%),
      radial-gradient(1px 1px at 6%  90%, rgba(255,255,255,0.25) 0%, transparent 100%),
      radial-gradient(1.5px 1.5px at 45% 55%, rgba(0,229,255,0.35) 0%, transparent 100%),
      radial-gradient(1px 1px at 92% 15%, rgba(255,255,255,0.3) 0%, transparent 100%);
    z-index: 0;
    pointer-events: none;
    animation: twinkle 6s ease-in-out infinite alternate;
  }

  @keyframes twinkle {
    0%   { opacity: 0.6; }
    100% { opacity: 1; }
  }

  /* ── Layout ── */
  .wrap {
    position: relative; z-index: 1;
    max-width: 1100px;
    margin: 0 auto;
    padding: 0 20px 60px;
  }

  /* ── Header ── */
  header {
    position: relative; z-index: 2;
    display: flex;
    align-items: center;
    justify-content: space-between;
    padding: 28px 0 24px;
    border-bottom: 1px solid var(--border);
    margin-bottom: 40px;
  }

  .logo {
    display: flex; align-items: center; gap: 14px;
  }

  .logo-gem {
    width: 44px; height: 44px;
    position: relative;
  }

  .logo-gem svg { width: 100%; height: 100%; filter: drop-shadow(0 0 10px var(--gem)); }

  .logo-text {
    font-family: 'Cinzel Decorative', serif;
    font-size: 1.5rem;
    color: var(--gem);
    text-shadow: 0 0 20px rgba(0,229,255,0.5);
    letter-spacing: 1px;
  }

  .logo-sub {
    font-family: 'Rajdhani', sans-serif;
    font-size: 0.72rem;
    letter-spacing: 3px;
    text-transform: uppercase;
    color: var(--text-dim);
    margin-top: 2px;
  }

  .header-right {
    display: flex; align-items: center; gap: 16px;
  }

  .user-chip {
    display: flex; align-items: center; gap: 10px;
    background: var(--panel2);
    border: 1px solid var(--border);
    border-radius: 50px;
    padding: 8px 16px;
    font-family: 'Rajdhani', sans-serif;
    font-size: 0.9rem;
    letter-spacing: 1px;
  }

  .user-chip span { color: var(--gem); font-weight: 700; }

  /* ── Flash ── */
  .flash {
    position: relative; z-index: 2;
    padding: 14px 20px;
    border-radius: var(--radius);
    margin-bottom: 28px;
    font-family: 'Rajdhani', sans-serif;
    font-size: 1rem;
    font-weight: 600;
    letter-spacing: 0.5px;
    border-left: 4px solid;
    animation: fadeSlide 0.4s ease;
  }

  .flash.success { background: rgba(0,230,118,0.08); border-color: var(--success); color: var(--success); }
  .flash.error   { background: rgba(255,82,82,0.08);  border-color: var(--error);   color: var(--error); }

  @keyframes fadeSlide {
    from { opacity: 0; transform: translateY(-8px); }
    to   { opacity: 1; transform: translateY(0); }
  }

  /* ── Auth Cards ── */
  .auth-outer {
    display: flex; justify-content: center;
    padding-top: 20px;
  }

  .auth-card {
    width: 100%; max-width: 440px;
    background: var(--panel);
    border: 1px solid var(--border);
    border-radius: 20px;
    padding: 42px 38px;
    box-shadow: 0 0 60px rgba(0,229,255,0.05), 0 20px 40px rgba(0,0,0,0.4);
    position: relative;
    overflow: hidden;
  }

  .auth-card::before {
    content: '';
    position: absolute; top: 0; left: 0; right: 0; height: 2px;
    background: linear-gradient(90deg, transparent, var(--gem), transparent);
  }

  .auth-gem-bg {
    position: absolute; bottom: -40px; right: -40px;
    width: 180px; opacity: 0.04;
    pointer-events: none;
  }

  .auth-title {
    font-family: 'Cinzel Decorative', serif;
    font-size: 1.3rem;
    color: var(--gem);
    margin-bottom: 6px;
    text-align: center;
  }

  .auth-sub {
    text-align: center;
    color: var(--text-dim);
    font-size: 0.88rem;
    margin-bottom: 32px;
  }

  .tab-switcher {
    display: flex;
    background: var(--panel2);
    border-radius: 10px;
    padding: 4px;
    margin-bottom: 28px;
  }

  .tab-btn {
    flex: 1; padding: 10px;
    border: none; background: transparent;
    color: var(--text-dim);
    font-family: 'Rajdhani', sans-serif;
    font-size: 0.95rem;
    font-weight: 600;
    letter-spacing: 1px;
    text-transform: uppercase;
    cursor: pointer;
    border-radius: 7px;
    transition: all 0.2s;
    text-decoration: none;
    text-align: center;
    display: block;
  }

  .tab-btn.active, .tab-btn:hover {
    background: var(--gem-deep);
    color: var(--gem);
  }

  /* ── Forms ── */
  .field { margin-bottom: 18px; }

  label {
    display: block;
    font-family: 'Rajdhani', sans-serif;
    font-size: 0.82rem;
    font-weight: 700;
    letter-spacing: 1.5px;
    text-transform: uppercase;
    color: var(--text-dim);
    margin-bottom: 7px;
  }

  input[type=text], input[type=email], input[type=password], input[type=number] {
    width: 100%;
    background: var(--panel2);
    border: 1px solid var(--border);
    border-radius: 9px;
    padding: 12px 16px;
    color: var(--text);
    font-family: 'Exo 2', sans-serif;
    font-size: 0.95rem;
    outline: none;
    transition: border-color 0.2s, box-shadow 0.2s;
  }

  input:focus {
    border-color: var(--gem);
    box-shadow: 0 0 0 3px rgba(0,229,255,0.1);
  }

  input::placeholder { color: #3a6070; }

  /* ── Buttons ── */
  .btn {
    display: inline-flex; align-items: center; justify-content: center; gap: 8px;
    padding: 13px 24px;
    border: none; border-radius: 10px;
    font-family: 'Rajdhani', sans-serif;
    font-size: 1rem;
    font-weight: 700;
    letter-spacing: 1px;
    text-transform: uppercase;
    cursor: pointer;
    text-decoration: none;
    transition: all 0.2s;
  }

  .btn-primary {
    background: linear-gradient(135deg, var(--gem-deep), #006080);
    color: var(--gem);
    border: 1px solid rgba(0,229,255,0.35);
    width: 100%;
    box-shadow: 0 4px 20px rgba(0,229,255,0.12);
  }

  .btn-primary:hover {
    background: linear-gradient(135deg, #005570, #008fb5);
    box-shadow: 0 4px 28px rgba(0,229,255,0.25);
    transform: translateY(-1px);
  }

  .btn-sm {
    padding: 9px 18px; font-size: 0.85rem;
    background: var(--panel2);
    color: var(--text-dim);
    border: 1px solid var(--border);
    border-radius: 8px;
  }

  .btn-sm:hover { color: var(--gem); border-color: var(--gem); }

  .btn-logout {
    background: transparent;
    color: var(--text-dim);
    border: 1px solid var(--border);
    border-radius: 8px;
    padding: 8px 16px;
    font-family: 'Rajdhani', sans-serif;
    font-size: 0.85rem;
    font-weight: 600;
    letter-spacing: 1px;
    text-transform: uppercase;
    cursor: pointer;
    transition: all 0.2s;
  }

  .btn-logout:hover { color: var(--error); border-color: var(--error); }

  /* ── Dashboard ── */
  .dashboard { display: grid; grid-template-columns: 320px 1fr; gap: 28px; }

  @media (max-width: 800px) {
    .dashboard { grid-template-columns: 1fr; }
  }

  /* Balance Card */
  .balance-card {
    background: var(--panel);
    border: 1px solid var(--border);
    border-radius: 20px;
    padding: 30px;
    position: relative;
    overflow: hidden;
    box-shadow: 0 0 50px rgba(0,229,255,0.05);
  }

  .balance-card::before {
    content: '';
    position: absolute; top: 0; left: 0; right: 0; height: 2px;
    background: linear-gradient(90deg, transparent, var(--gold), transparent);
  }

  .balance-label {
    font-family: 'Rajdhani', sans-serif;
    font-size: 0.75rem;
    font-weight: 700;
    letter-spacing: 3px;
    text-transform: uppercase;
    color: var(--text-dim);
    margin-bottom: 12px;
  }

  .balance-amount {
    font-family: 'Cinzel Decorative', serif;
    font-size: 2.4rem;
    color: var(--gold);
    text-shadow: 0 0 30px rgba(255,213,79,0.4);
    line-height: 1;
    margin-bottom: 4px;
    word-break: break-all;
  }

  .balance-unit {
    font-family: 'Rajdhani', sans-serif;
    font-size: 0.8rem;
    letter-spacing: 2px;
    text-transform: uppercase;
    color: var(--gold-glow);
    opacity: 0.7;
    margin-bottom: 24px;
  }

  .gem-icon-large {
    position: absolute; bottom: -20px; right: -20px;
    width: 110px; opacity: 0.06;
    pointer-events: none;
    filter: drop-shadow(0 0 20px var(--gold));
  }

  /* Action Panels */
  .actions-grid {
    display: grid;
    grid-template-columns: 1fr 1fr;
    gap: 14px;
    margin-top: 18px;
  }

  .action-panel {
    background: var(--panel2);
    border: 1px solid var(--border);
    border-radius: 14px;
    padding: 18px;
    transition: border-color 0.2s, box-shadow 0.2s;
  }

  .action-panel:hover { border-color: rgba(0,229,255,0.3); box-shadow: 0 4px 20px rgba(0,0,0,0.3); }

  .action-label {
    font-family: 'Rajdhani', sans-serif;
    font-size: 0.72rem;
    font-weight: 700;
    letter-spacing: 2px;
    text-transform: uppercase;
    color: var(--text-dim);
    margin-bottom: 12px;
    display: flex; align-items: center; gap: 7px;
  }

  .action-label .dot {
    width: 6px; height: 6px; border-radius: 50%;
    background: var(--gem);
    box-shadow: 0 0 6px var(--gem);
  }

  .action-label .dot.gold { background: var(--gold); box-shadow: 0 0 6px var(--gold); }
  .action-label .dot.red  { background: var(--error); box-shadow: 0 0 6px var(--error); }

  .action-panel .field { margin-bottom: 12px; }

  .action-panel input {
    padding: 10px 14px;
    font-size: 0.9rem;
    border-radius: 8px;
  }

  .action-panel .btn-primary {
    padding: 10px;
    font-size: 0.85rem;
  }

  /* Full-width transfer */
  .action-full {
    grid-column: 1 / -1;
  }

  .transfer-row {
    display: grid; grid-template-columns: 1fr 1fr; gap: 12px;
  }

  /* ── History Panel ── */
  .history-panel {
    background: var(--panel);
    border: 1px solid var(--border);
    border-radius: 20px;
    padding: 28px;
    box-shadow: 0 0 50px rgba(0,229,255,0.04);
  }

  .panel-title {
    font-family: 'Rajdhani', sans-serif;
    font-size: 0.75rem;
    font-weight: 700;
    letter-spacing: 3px;
    text-transform: uppercase;
    color: var(--text-dim);
    margin-bottom: 20px;
    padding-bottom: 14px;
    border-bottom: 1px solid var(--border);
    display: flex; align-items: center; gap: 8px;
  }

  .panel-title::before {
    content: '';
    width: 3px; height: 14px; border-radius: 2px;
    background: var(--gem);
    box-shadow: 0 0 8px var(--gem);
  }

  .tx-list { list-style: none; }

  .tx-item {
    display: flex; align-items: center; justify-content: space-between;
    padding: 13px 0;
    border-bottom: 1px solid rgba(0,229,255,0.05);
    animation: fadeIn 0.3s ease both;
  }

  .tx-item:last-child { border-bottom: none; }

  @keyframes fadeIn {
    from { opacity: 0; transform: translateX(-6px); }
    to   { opacity: 1; transform: translateX(0); }
  }

  .tx-left { display: flex; align-items: center; gap: 12px; }

  .tx-icon {
    width: 36px; height: 36px; border-radius: 10px;
    display: flex; align-items: center; justify-content: center;
    font-size: 1rem;
    flex-shrink: 0;
  }

  .tx-icon.deposit   { background: rgba(0,230,118,0.1); }
  .tx-icon.withdrawal{ background: rgba(255,82,82,0.1); }
  .tx-icon.transfer_in  { background: rgba(0,229,255,0.1); }
  .tx-icon.transfer_out { background: rgba(255,213,79,0.1); }

  .tx-note {
    font-size: 0.9rem; font-weight: 500;
    color: var(--text);
    line-height: 1.2;
  }

  .tx-time {
    font-size: 0.75rem;
    color: var(--text-dim);
    margin-top: 3px;
  }

  .tx-amount {
    font-family: 'Rajdhani', sans-serif;
    font-size: 1rem;
    font-weight: 700;
    letter-spacing: 0.5px;
  }

  .tx-amount.pos { color: var(--success); }
  .tx-amount.neg { color: var(--error); }

  .empty-state {
    text-align: center; padding: 40px 0;
    color: var(--text-dim);
    font-size: 0.9rem;
  }

  /* ── Responsive ── */
  @media (max-width: 640px) {
    .auth-card { padding: 28px 22px; }
    .actions-grid { grid-template-columns: 1fr; }
    .transfer-row { grid-template-columns: 1fr; }
    .action-full { grid-column: auto; }
    .balance-amount { font-size: 1.9rem; }
  }
</style>
</head>
<body>
<div class="wrap">

  <!-- Header -->
  <header>
    <div class="logo">
      <div class="logo-gem">
        <svg viewBox="0 0 48 48" xmlns="http://www.w3.org/2000/svg">
          <polygon points="24,4 44,16 44,32 24,44 4,32 4,16" fill="none" stroke="#00e5ff" stroke-width="1.5"/>
          <polygon points="24,4 36,14 24,20 12,14" fill="rgba(0,229,255,0.15)" stroke="#00e5ff" stroke-width="1"/>
          <polygon points="24,20 36,14 44,32 24,44" fill="rgba(0,229,255,0.08)" stroke="#00e5ff" stroke-width="0.8"/>
          <polygon points="24,20 12,14 4,32 24,44" fill="rgba(0,229,255,0.12)" stroke="#00e5ff" stroke-width="0.8"/>
          <line x1="24" y1="4" x2="24" y2="20" stroke="#00e5ff" stroke-width="0.8" opacity="0.5"/>
          <line x1="4" y1="16" x2="44" y2="16" stroke="#00e5ff" stroke-width="0.5" opacity="0.3"/>
        </svg>
      </div>
      <div>
        <div class="logo-text">GemBank</div>
        <div class="logo-sub">Digital Currency Vault</div>
      </div>
    </div>

    <?php if (isLoggedIn() && $user): ?>
    <div class="header-right">
      <div class="user-chip">
        ◈ <span><?= htmlspecialchars($user['username']) ?></span>
      </div>
      <form method="POST" style="margin:0">
        <input type="hidden" name="action" value="logout">
        <button type="submit" class="btn-logout">Sign Out</button>
      </form>
    </div>
    <?php endif; ?>
  </header>

  <!-- Flash Message -->
  <?php if ($flash): ?>
  <div class="flash <?= $flash['type'] ?>">
    <?= htmlspecialchars($flash['msg']) ?>
  </div>
  <?php endif; ?>


  <?php if (!isLoggedIn()): ?>
  <!-- ═══════════════════ AUTH ═══════════════════ -->
  <div class="auth-outer">
    <div class="auth-card">
      <!-- decorative gem -->
      <svg class="auth-gem-bg" viewBox="0 0 48 48" xmlns="http://www.w3.org/2000/svg" fill="none" stroke="#00e5ff">
        <polygon points="24,4 44,16 44,32 24,44 4,32 4,16" stroke-width="1.5"/>
        <polygon points="24,4 36,14 24,20 12,14" stroke-width="1"/>
        <polygon points="24,20 36,14 44,32 24,44" stroke-width="0.8"/>
        <polygon points="24,20 12,14 4,32 24,44" stroke-width="0.8"/>
      </svg>

      <div class="tab-switcher">
        <a href="?view=login"    class="tab-btn <?= $showAuth === 'login'    ? 'active' : '' ?>">Login</a>
        <a href="?view=register" class="tab-btn <?= $showAuth === 'register' ? 'active' : '' ?>">Register</a>
      </div>

      <?php if ($showAuth === 'register'): ?>
        <div class="auth-title">Create Account</div>
        <div class="auth-sub">Join GemBank and receive 100 starter gems</div>
        <form method="POST">
          <input type="hidden" name="action" value="register">
          <div class="field">
            <label>Username</label>
            <input type="text" name="username" placeholder="Choose a unique username" required minlength="3">
          </div>
          <div class="field">
            <label>Email Address</label>
            <input type="email" name="email" placeholder="your@email.com" required>
          </div>
          <div class="field">
            <label>Password</label>
            <input type="password" name="password" placeholder="Create a strong password" required>
          </div>
          <div class="field">
            <label>Confirm Password</label>
            <input type="password" name="confirm" placeholder="Repeat your password" required>
          </div>
          <button type="submit" class="btn btn-primary" style="margin-top:8px">◈ &nbsp;Create Vault Account</button>
        </form>

      <?php else: ?>
        <div class="auth-title">Vault Access</div>
        <div class="auth-sub">Sign in to access your gem holdings</div>
        <form method="POST">
          <input type="hidden" name="action" value="login">
          <div class="field">
            <label>Email Address</label>
            <input type="email" name="email" placeholder="your@email.com" required>
          </div>
          <div class="field">
            <label>Password</label>
            <input type="password" name="password" placeholder="Your password" required>
          </div>
          <button type="submit" class="btn btn-primary" style="margin-top:8px">◈ &nbsp;Enter the Vault</button>
        </form>
      <?php endif; ?>
    </div>
  </div>


  <?php else: ?>
  <!-- ═══════════════════ DASHBOARD ═══════════════════ -->
  <div class="dashboard">

    <!-- Left column -->
    <div>
      <!-- Balance Card -->
      <div class="balance-card">
        <div class="balance-label">◈ Current Balance</div>
        <div class="balance-amount"><?= number_format((float)$user['balance'], 0) ?></div>
        <div class="balance-unit">Gems &nbsp;·&nbsp; <?= htmlspecialchars($user['username']) ?></div>
        <svg class="gem-icon-large" viewBox="0 0 48 48" xmlns="http://www.w3.org/2000/svg" fill="none" stroke="#ffd54f">
          <polygon points="24,4 44,16 44,32 24,44 4,32 4,16" stroke-width="1.5"/>
          <polygon points="24,4 36,14 24,20 12,14" stroke-width="1"/>
          <polygon points="24,20 36,14 44,32 24,44" stroke-width="0.8"/>
          <polygon points="24,20 12,14 4,32 24,44" stroke-width="0.8"/>
        </svg>
      </div>

      <!-- Actions -->
      <div class="actions-grid" style="margin-top:18px;">

        <!-- Deposit -->
        <div class="action-panel">
          <div class="action-label"><span class="dot"></span> Deposit</div>
          <form method="POST">
            <input type="hidden" name="action" value="deposit">
            <div class="field">
              <label>Amount</label>
              <input type="number" name="amount" placeholder="0" min="1" max="1000000" step="1" required>
            </div>
            <button type="submit" class="btn btn-primary">Add Gems</button>
          </form>
        </div>

        <!-- Withdraw -->
        <div class="action-panel">
          <div class="action-label"><span class="dot red"></span> Withdraw</div>
          <form method="POST">
            <input type="hidden" name="action" value="withdraw">
            <div class="field">
              <label>Amount</label>
              <input type="number" name="amount" placeholder="0" min="1" step="1" required>
            </div>
            <button type="submit" class="btn btn-primary">Take Gems</button>
          </form>
        </div>

        <!-- Transfer -->
        <div class="action-panel action-full">
          <div class="action-label"><span class="dot gold"></span> Transfer Gems</div>
          <form method="POST">
            <input type="hidden" name="action" value="transfer">
            <div class="transfer-row">
              <div class="field">
                <label>Recipient Username</label>
                <input type="text" name="recipient" placeholder="Enter username" required>
              </div>
              <div class="field">
                <label>Amount</label>
                <input type="number" name="amount" placeholder="0" min="1" step="1" required>
              </div>
            </div>
            <button type="submit" class="btn btn-primary">Send Gems ◈</button>
          </form>
        </div>

      </div><!-- /actions-grid -->
    </div><!-- /left column -->

    <!-- Right column: History -->
    <div class="history-panel">
      <div class="panel-title">Transaction History</div>

      <?php if (empty($transactions)): ?>
        <div class="empty-state">
          <div style="font-size:2rem;margin-bottom:10px;">◈</div>
          No transactions yet. Make your first deposit!
        </div>
      <?php else: ?>
        <ul class="tx-list">
          <?php foreach ($transactions as $tx):
            $sign  = in_array($tx['type'], ['deposit','transfer_in']) ? '+' : '−';
            $cls   = in_array($tx['type'], ['deposit','transfer_in']) ? 'pos' : 'neg';
            $icons = ['deposit'=>'💎','withdrawal'=>'💸','transfer_in'=>'📥','transfer_out'=>'📤'];
            $icon  = $icons[$tx['type']] ?? '◈';
            $dt    = new DateTime($tx['created_at']);
          ?>
          <li class="tx-item">
            <div class="tx-left">
              <div class="tx-icon <?= $tx['type'] ?>"><?= $icon ?></div>
              <div>
                <div class="tx-note"><?= htmlspecialchars($tx['note'] ?? ucfirst($tx['type'])) ?></div>
                <div class="tx-time"><?= $dt->format('M j, Y · g:i A') ?></div>
              </div>
            </div>
            <div class="tx-amount <?= $cls ?>"><?= $sign ?><?= number_format((float)$tx['amount'], 0) ?> ◈</div>
          </li>
          <?php endforeach; ?>
        </ul>
      <?php endif; ?>
    </div>

  </div><!-- /dashboard -->
  <?php endif; ?>

</div><!-- /wrap -->
</body>
</html>