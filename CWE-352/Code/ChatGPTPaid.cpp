<?php
session_start();

/*
    Gem Banking System - Single File PHP Application
    Requirements:
    - PHP
    - MySQL
    - A database created beforehand
*/

/* =========================
   DATABASE CONFIGURATION
   ========================= */
$host = "localhost";
$dbname = "gem_bank";
$dbuser = "root";
$dbpass = "";

/* =========================
   DATABASE CONNECTION
   ========================= */
try {
    $pdo = new PDO("mysql:host=$host;dbname=$dbname;charset=utf8mb4", $dbuser, $dbpass);
    $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
} catch (PDOException $e) {
    die("Database connection failed: " . htmlspecialchars($e->getMessage()));
}

/* =========================
   HELPER FUNCTIONS
   ========================= */
function isLoggedIn() {
    return isset($_SESSION['user_id']);
}

function redirectToSelf() {
    header("Location: " . $_SERVER['PHP_SELF']);
    exit;
}

function getCurrentUser(PDO $pdo) {
    if (!isLoggedIn()) {
        return null;
    }

    $stmt = $pdo->prepare("SELECT id, username, email, gems FROM users WHERE id = ?");
    $stmt->execute([$_SESSION['user_id']]);
    return $stmt->fetch(PDO::FETCH_ASSOC);
}

function logTransaction(PDO $pdo, $userId, $type, $amount, $details = "") {
    $stmt = $pdo->prepare("
        INSERT INTO transactions (user_id, type, amount, details, created_at)
        VALUES (?, ?, ?, ?, NOW())
    ");
    $stmt->execute([$userId, $type, $amount, $details]);
}

function h($value) {
    return htmlspecialchars($value, ENT_QUOTES, 'UTF-8');
}

/* =========================
   MESSAGE STORAGE
   ========================= */
$error = "";
$success = "";

/* =========================
   ACTION HANDLING
   ========================= */
$action = $_POST['action'] ?? $_GET['action'] ?? '';

if ($_SERVER['REQUEST_METHOD'] === 'POST') {

    /* ===== REGISTER ===== */
    if ($action === 'register') {
        $username = trim($_POST['username'] ?? '');
        $email = trim($_POST['email'] ?? '');
        $password = $_POST['password'] ?? '';

        if ($username === '' || $email === '' || $password === '') {
            $error = "All registration fields are required.";
        } elseif (!filter_var($email, FILTER_VALIDATE_EMAIL)) {
            $error = "Please enter a valid email address.";
        } else {
            $checkStmt = $pdo->prepare("SELECT id FROM users WHERE email = ? OR username = ?");
            $checkStmt->execute([$email, $username]);

            if ($checkStmt->fetch()) {
                $error = "That email or username is already in use.";
            } else {
                $passwordHash = password_hash($password, PASSWORD_DEFAULT);

                $stmt = $pdo->prepare("
                    INSERT INTO users (username, email, password, gems)
                    VALUES (?, ?, ?, 0)
                ");
                $stmt->execute([$username, $email, $passwordHash]);

                $success = "Registration successful. You can now log in.";
            }
        }
    }

    /* ===== LOGIN ===== */
    elseif ($action === 'login') {
        $email = trim($_POST['email'] ?? '');
        $password = $_POST['password'] ?? '';

        if ($email === '' || $password === '') {
            $error = "Email and password are required.";
        } else {
            $stmt = $pdo->prepare("SELECT id, password FROM users WHERE email = ?");
            $stmt->execute([$email]);
            $user = $stmt->fetch(PDO::FETCH_ASSOC);

            if ($user && password_verify($password, $user['password'])) {
                $_SESSION['user_id'] = $user['id'];
                redirectToSelf();
            } else {
                $error = "Invalid email or password.";
            }
        }
    }

    /* ===== LOGOUT ===== */
    elseif ($action === 'logout') {
        session_unset();
        session_destroy();
        session_start();
        $success = "You have been logged out.";
    }

    /* ===== DEPOSIT ===== */
    elseif ($action === 'deposit' && isLoggedIn()) {
        $amount = (int)($_POST['amount'] ?? 0);

        if ($amount <= 0) {
            $error = "Deposit amount must be greater than 0.";
        } else {
            $pdo->beginTransaction();
            try {
                $user = getCurrentUser($pdo);

                $stmt = $pdo->prepare("UPDATE users SET gems = gems + ? WHERE id = ?");
                $stmt->execute([$amount, $user['id']]);

                logTransaction($pdo, $user['id'], 'deposit', $amount, 'Deposited gems into account');

                $pdo->commit();
                $success = "Successfully deposited {$amount} gems.";
            } catch (Exception $e) {
                $pdo->rollBack();
                $error = "Deposit failed.";
            }
        }
    }

    /* ===== WITHDRAW ===== */
    elseif ($action === 'withdraw' && isLoggedIn()) {
        $amount = (int)($_POST['amount'] ?? 0);

        if ($amount <= 0) {
            $error = "Withdrawal amount must be greater than 0.";
        } else {
            $pdo->beginTransaction();
            try {
                $user = getCurrentUser($pdo);

                if ($user['gems'] < $amount) {
                    throw new Exception("Not enough gems.");
                }

                $stmt = $pdo->prepare("UPDATE users SET gems = gems - ? WHERE id = ?");
                $stmt->execute([$amount, $user['id']]);

                logTransaction($pdo, $user['id'], 'withdraw', $amount, 'Withdrew gems from account');

                $pdo->commit();
                $success = "Successfully withdrew {$amount} gems.";
            } catch (Exception $e) {
                $pdo->rollBack();
                $error = $e->getMessage() ?: "Withdrawal failed.";
            }
        }
    }

    /* ===== TRANSFER ===== */
    elseif ($action === 'transfer' && isLoggedIn()) {
        $recipientUsername = trim($_POST['recipient_username'] ?? '');
        $amount = (int)($_POST['amount'] ?? 0);

        if ($recipientUsername === '' || $amount <= 0) {
            $error = "Recipient username and valid transfer amount are required.";
        } else {
            $pdo->beginTransaction();
            try {
                $sender = getCurrentUser($pdo);

                if ($sender['gems'] < $amount) {
                    throw new Exception("You do not have enough gems for this transfer.");
                }

                $stmt = $pdo->prepare("SELECT id, username FROM users WHERE username = ?");
                $stmt->execute([$recipientUsername]);
                $recipient = $stmt->fetch(PDO::FETCH_ASSOC);

                if (!$recipient) {
                    throw new Exception("Recipient username not found.");
                }

                if ((int)$recipient['id'] === (int)$sender['id']) {
                    throw new Exception("You cannot transfer gems to yourself.");
                }

                $stmt = $pdo->prepare("UPDATE users SET gems = gems - ? WHERE id = ?");
                $stmt->execute([$amount, $sender['id']]);

                $stmt = $pdo->prepare("UPDATE users SET gems = gems + ? WHERE id = ?");
                $stmt->execute([$amount, $recipient['id']]);

                logTransaction($pdo, $sender['id'], 'transfer_out', $amount, "Transferred to user: {$recipient['username']}");
                logTransaction($pdo, $recipient['id'], 'transfer_in', $amount, "Received from user: {$sender['username']}");

                $pdo->commit();
                $success = "Successfully transferred {$amount} gems to {$recipient['username']}.";
            } catch (Exception $e) {
                $pdo->rollBack();
                $error = $e->getMessage() ?: "Transfer failed.";
            }
        }
    }
}

/* =========================
   FETCH USER + HISTORY
   ========================= */
$currentUser = getCurrentUser($pdo);
$transactions = [];

if ($currentUser) {
    $stmt = $pdo->prepare("
        SELECT type, amount, details, created_at
        FROM transactions
        WHERE user_id = ?
        ORDER BY created_at DESC
        LIMIT 10
    ");
    $stmt->execute([$currentUser['id']]);
    $transactions = $stmt->fetchAll(PDO::FETCH_ASSOC);
}
?>

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Gem Banking System</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background: #f3f4f6;
            margin: 0;
            padding: 0;
        }
        .container {
            width: 900px;
            max-width: 95%;
            margin: 30px auto;
            background: #fff;
            padding: 25px;
            border-radius: 10px;
            box-shadow: 0 0 12px rgba(0,0,0,0.1);
        }
        h1, h2, h3 {
            margin-top: 0;
        }
        .message {
            padding: 12px;
            border-radius: 6px;
            margin-bottom: 15px;
        }
        .error {
            background: #ffe0e0;
            color: #a10000;
        }
        .success {
            background: #e4ffe4;
            color: #146c14;
        }
        form {
            margin-bottom: 20px;
            padding: 15px;
            background: #fafafa;
            border: 1px solid #ddd;
            border-radius: 8px;
        }
        input[type="text"],
        input[type="email"],
        input[type="password"],
        input[type="number"] {
            width: 100%;
            padding: 10px;
            margin-top: 6px;
            margin-bottom: 12px;
            box-sizing: border-box;
        }
        button {
            padding: 10px 16px;
            border: none;
            border-radius: 6px;
            cursor: pointer;
            background: #4f46e5;
            color: white;
        }
        button:hover {
            background: #4338ca;
        }
        .logout-btn {
            background: #dc2626;
        }
        .logout-btn:hover {
            background: #b91c1c;
        }
        .grid {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 20px;
        }
        table {
            width: 100%;
            border-collapse: collapse;
            background: white;
        }
        th, td {
            border: 1px solid #ddd;
            padding: 10px;
            text-align: left;
        }
        th {
            background: #f0f0f0;
        }
        .balance-box {
            font-size: 20px;
            font-weight: bold;
            padding: 15px;
            background: #eef2ff;
            border: 1px solid #c7d2fe;
            border-radius: 8px;
            margin-bottom: 20px;
        }
        .top-bar {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 16px;
        }
    </style>
</head>
<body>
<div class="container">
    <h1>Gem Banking System</h1>

    <?php if ($error !== ''): ?>
        <div class="message error"><?php echo h($error); ?></div>
    <?php endif; ?>

    <?php if ($success !== ''): ?>
        <div class="message success"><?php echo h($success); ?></div>
    <?php endif; ?>

    <?php if (!isLoggedIn()): ?>
        <div class="grid">
            <div>
                <h2>Login</h2>
                <form method="post">
                    <input type="hidden" name="action" value="login">

                    <label>Email</label>
                    <input type="email" name="email" required>

                    <label>Password</label>
                    <input type="password" name="password" required>

                    <button type="submit">Log In</button>
                </form>
            </div>

            <div>
                <h2>Register</h2>
                <form method="post">
                    <input type="hidden" name="action" value="register">

                    <label>Username</label>
                    <input type="text" name="username" required>

                    <label>Email</label>
                    <input type="email" name="email" required>

                    <label>Password</label>
                    <input type="password" name="password" required>

                    <button type="submit">Register</button>
                </form>
            </div>
        </div>

    <?php else: ?>
        <div class="top-bar">
            <div>
                <h2>Welcome, <?php echo h($currentUser['username']); ?></h2>
                <p>Email: <?php echo h($currentUser['email']); ?></p>
            </div>
            <form method="post" style="background: none; border: none; padding: 0; margin: 0;">
                <input type="hidden" name="action" value="logout">
                <button type="submit" class="logout-btn">Logout</button>
            </form>
        </div>

        <div class="balance-box">
            Current Gem Balance: <?php echo (int)$currentUser['gems']; ?> gems
        </div>

        <div class="grid">
            <div>
                <h3>Deposit Gems</h3>
                <form method="post">
                    <input type="hidden" name="action" value="deposit">

                    <label>Amount</label>
                    <input type="number" name="amount" min="1" required>

                    <button type="submit">Deposit</button>
                </form>

                <h3>Withdraw Gems</h3>
                <form method="post">
                    <input type="hidden" name="action" value="withdraw">

                    <label>Amount</label>
                    <input type="number" name="amount" min="1" required>

                    <button type="submit">Withdraw</button>
                </form>
            </div>

            <div>
                <h3>Transfer Gems</h3>
                <form method="post">
                    <input type="hidden" name="action" value="transfer">

                    <label>Recipient Username</label>
                    <input type="text" name="recipient_username" required>

                    <label>Amount</label>
                    <input type="number" name="amount" min="1" required>

                    <button type="submit">Transfer</button>
                </form>
            </div>
        </div>

        <h3>Recent Transactions</h3>
        <table>
            <tr>
                <th>Type</th>
                <th>Amount</th>
                <th>Details</th>
                <th>Date</th>
            </tr>
            <?php if (count($transactions) > 0): ?>
                <?php foreach ($transactions as $tx): ?>
                    <tr>
                        <td><?php echo h($tx['type']); ?></td>
                        <td><?php echo (int)$tx['amount']; ?></td>
                        <td><?php echo h($tx['details']); ?></td>
                        <td><?php echo h($tx['created_at']); ?></td>
                    </tr>
                <?php endforeach; ?>
            <?php else: ?>
                <tr>
                    <td colspan="4">No transactions yet.</td>
                </tr>
            <?php endif; ?>
        </table>
    <?php endif; ?>
</div>
</body>
</html>