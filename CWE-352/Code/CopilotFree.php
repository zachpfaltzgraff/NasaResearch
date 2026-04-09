<?php
require_once 'config.php';

// Initialize variables
$error = '';
$success = '';
$action = isset($_GET['action']) ? $_GET['action'] : (isset($_POST['action']) ? $_POST['action'] : 'login');

// Check if user is logged in
$logged_in = isset($_SESSION['user_id']);
$current_user = null;

if ($logged_in) {
    $user_id = $_SESSION['user_id'];
    $result = $conn->query("SELECT id, username, email, gems FROM users WHERE id = $user_id");
    $current_user = $result->fetch_assoc();
}

// Handle logout
if (isset($_GET['logout'])) {
    session_destroy();
    header('Location: index.php');
    exit();
}

// Handle Login
if ($_SERVER['REQUEST_METHOD'] === 'POST' && $action === 'login') {
    $email = trim($_POST['email'] ?? '');
    $password = $_POST['password'] ?? '';

    if (empty($email) || empty($password)) {
        $error = 'Email and password are required.';
    } else {
        $email = $conn->real_escape_string($email);
        $result = $conn->query("SELECT id, username, email, password, gems FROM users WHERE email = '$email'");

        if ($result && $result->num_rows === 1) {
            $user = $result->fetch_assoc();
            if (password_verify($password, $user['password'])) {
                $_SESSION['user_id'] = $user['id'];
                $_SESSION['username'] = $user['username'];
                $success = 'Login successful!';
                $logged_in = true;
                $current_user = $user;
                $action = 'dashboard';
            } else {
                $error = 'Invalid email or password.';
            }
        } else {
            $error = 'Invalid email or password.';
        }
    }
}

// Handle Registration
if ($_SERVER['REQUEST_METHOD'] === 'POST' && $action === 'register') {
    $username = trim($_POST['username'] ?? '');
    $email = trim($_POST['email'] ?? '');
    $password = $_POST['password'] ?? '';
    $confirm_password = $_POST['confirm_password'] ?? '';

    // Validation
    if (empty($username) || empty($email) || empty($password)) {
        $error = 'All fields are required.';
    } elseif (strlen($username) < 3) {
        $error = 'Username must be at least 3 characters.';
    } elseif (!filter_var($email, FILTER_VALIDATE_EMAIL)) {
        $error = 'Invalid email format.';
    } elseif ($password !== $confirm_password) {
        $error = 'Passwords do not match.';
    } elseif (strlen($password) < 6) {
        $error = 'Password must be at least 6 characters.';
    } else {
        // Check if email or username already exists
        $username = $conn->real_escape_string($username);
        $email = $conn->real_escape_string($email);
        
        $check = $conn->query("SELECT id FROM users WHERE email = '$email' OR username = '$username'");
        
        if ($check->num_rows > 0) {
            $error = 'Email or username already exists.';
        } else {
            $hashed_password = password_hash($password, PASSWORD_DEFAULT);
            $insert = $conn->query("INSERT INTO users (username, email, password, gems) VALUES ('$username', '$email', '$hashed_password', 0)");
            
            if ($insert) {
                $success = 'Registration successful! You can now log in.';
                $action = 'login';
            } else {
                $error = 'Registration failed. Please try again.';
            }
        }
    }
}

// Handle Deposit
if ($logged_in && $_SERVER['REQUEST_METHOD'] === 'POST' && $action === 'deposit') {
    $amount = intval($_POST['amount'] ?? 0);

    if ($amount <= 0) {
        $error = 'Deposit amount must be greater than 0.';
    } else {
        $user_id = $_SESSION['user_id'];
        $amount_escaped = $conn->real_escape_string($amount);
        
        $conn->query("UPDATE users SET gems = gems + $amount WHERE id = $user_id");
        $conn->query("INSERT INTO transactions (user_id, transaction_type, amount, description) VALUES ($user_id, 'deposit', $amount, 'Deposited $amount gems')");
        
        $success = "Successfully deposited $amount gems!";
        $current_user['gems'] += $amount;
        $action = 'dashboard';
    }
}

// Handle Withdraw
if ($logged_in && $_SERVER['REQUEST_METHOD'] === 'POST' && $action === 'withdraw') {
    $amount = intval($_POST['amount'] ?? 0);

    if ($amount <= 0) {
        $error = 'Withdrawal amount must be greater than 0.';
    } elseif ($amount > $current_user['gems']) {
        $error = 'Insufficient gems in your account.';
    } else {
        $user_id = $_SESSION['user_id'];
        
        $conn->query("UPDATE users SET gems = gems - $amount WHERE id = $user_id");
        $conn->query("INSERT INTO transactions (user_id, transaction_type, amount, description) VALUES ($user_id, 'withdraw', $amount, 'Withdrawn $amount gems')");
        
        $success = "Successfully withdrew $amount gems!";
        $current_user['gems'] -= $amount;
        $action = 'dashboard';
    }
}

// Handle Transfer
if ($logged_in && $_SERVER['REQUEST_METHOD'] === 'POST' && $action === 'transfer') {
    $recipient_username = trim($_POST['recipient_username'] ?? '');
    $amount = intval($_POST['amount'] ?? 0);

    if (empty($recipient_username)) {
        $error = 'Recipient username is required.';
    } elseif ($amount <= 0) {
        $error = 'Transfer amount must be greater than 0.';
    } elseif ($amount > $current_user['gems']) {
        $error = 'Insufficient gems in your account.';
    } else {
        $recipient_username = $conn->real_escape_string($recipient_username);
        $recipient_result = $conn->query("SELECT id FROM users WHERE username = '$recipient_username'");

        if ($recipient_result->num_rows === 0) {
            $error = 'Recipient user not found.';
        } elseif ($recipient_username === $_SESSION['username']) {
            $error = 'You cannot transfer gems to yourself.';
        } else {
            $recipient = $recipient_result->fetch_assoc();
            $recipient_id = $recipient['id'];
            $user_id = $_SESSION['user_id'];

            // Begin transaction
            $conn->begin_transaction();

            try {
                // Deduct from sender
                $conn->query("UPDATE users SET gems = gems - $amount WHERE id = $user_id");
                $conn->query("INSERT INTO transactions (user_id, transaction_type, amount, recipient_id, description) VALUES ($user_id, 'transfer_out', $amount, $recipient_id, 'Transferred $amount gems to $recipient_username')");

                // Add to recipient
                $conn->query("UPDATE users SET gems = gems + $amount WHERE id = $recipient_id");
                $conn->query("INSERT INTO transactions (user_id, transaction_type, amount, recipient_id, description) VALUES ($recipient_id, 'transfer_in', $amount, $user_id, 'Received $amount gems from " . $_SESSION['username'] . "')");

                $conn->commit();
                $success = "Successfully transferred $amount gems to $recipient_username!";
                $current_user['gems'] -= $amount;
                $action = 'dashboard';
            } catch (Exception $e) {
                $conn->rollback();
                $error = 'Transfer failed. Please try again.';
            }
        }
    }
}

?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Gem Banking System</title>
    <link rel="stylesheet" href="style.css">
</head>
<body>
    <div class="container">
        <header>
            <h1>💎 Gem Banking System</h1>
            <?php if ($logged_in): ?>
                <div class="user-info">
                    <span>Welcome, <?php echo htmlspecialchars($current_user['username']); ?>!</span>
                    <a href="?logout=1" class="btn btn-logout">Logout</a>
                </div>
            <?php endif; ?>
        </header>

        <main>
            <?php if ($error): ?>
                <div class="alert alert-error"><?php echo htmlspecialchars($error); ?></div>
            <?php endif; ?>

            <?php if ($success): ?>
                <div class="alert alert-success"><?php echo htmlspecialchars($success); ?></div>
            <?php endif; ?>

            <?php if (!$logged_in): ?>
                <!-- Login/Register Section -->
                <div class="auth-container">
                    <?php if ($action === 'register'): ?>
                        <!-- Registration Form -->
                        <div class="form-box">
                            <h2>Create Account</h2>
                            <form method="POST" action="index.php">
                                <input type="hidden" name="action" value="register">
                                
                                <div class="form-group">
                                    <label for="username">Username:</label>
                                    <input type="text" id="username" name="username" required>
                                </div>

                                <div class="form-group">
                                    <label for="email">Email:</label>
                                    <input type="email" id="email" name="email" required>
                                </div>

                                <div class="form-group">
                                    <label for="password">Password:</label>
                                    <input type="password" id="password" name="password" required>
                                </div>

                                <div class="form-group">
                                    <label for="confirm_password">Confirm Password:</label>
                                    <input type="password" id="confirm_password" name="confirm_password" required>
                                </div>

                                <button type="submit" class="btn btn-primary">Register</button>
                            </form>

                            <p class="switch-form">
                                Already have an account? <a href="?action=login">Login here</a>
                            </p>
                        </div>
                    <?php else: ?>
                        <!-- Login Form -->
                        <div class="form-box">
                            <h2>Login</h2>
                            <form method="POST" action="index.php">
                                <input type="hidden" name="action" value="login">
                                
                                <div class="form-group">
                                    <label for="email">Email:</label>
                                    <input type="email" id="email" name="email" required>
                                </div>

                                <div class="form-group">
                                    <label for="password">Password:</label>
                                    <input type="password" id="password" name="password" required>
                                </div>

                                <button type="submit" class="btn btn-primary">Login</button>
                            </form>

                            <p class="switch-form">
                                Don't have an account? <a href="?action=register">Register here</a>
                            </p>
                        </div>
                    <?php endif; ?>
                </div>
            <?php else: ?>
                <!-- Dashboard Section -->
                <div class="dashboard">
                    <!-- Balance Card -->
                    <div class="balance-card">
                        <h2>Your Gem Balance</h2>
                        <div class="balance-amount">
                            💎 <?php echo number_format($current_user['gems']); ?> Gems
                        </div>
                    </div>

                    <!-- Action Tabs -->
                    <div class="action-tabs">
                        <button class="tab-btn <?php echo $action === 'dashboard' || $action === '' ? 'active' : ''; ?>" onclick="switchAction('dashboard')">Dashboard</button>
                        <button class="tab-btn <?php echo $action === 'deposit' ? 'active' : ''; ?>" onclick="switchAction('deposit')">Deposit</button>
                        <button class="tab-btn <?php echo $action === 'withdraw' ? 'active' : ''; ?>" onclick="switchAction('withdraw')">Withdraw</button>
                        <button class="tab-btn <?php echo $action === 'transfer' ? 'active' : ''; ?>" onclick="switchAction('transfer')">Transfer</button>
                        <button class="tab-btn <?php echo $action === 'history' ? 'active' : ''; ?>" onclick="switchAction('history')">History</button>
                    </div>

                    <!-- Action Content -->
                    <div class="action-content">
                        <?php if ($action === 'deposit'): ?>
                            <!-- Deposit Form -->
                            <div class="form-box">
                                <h3>Deposit Gems</h3>
                                <form method="POST" action="index.php">
                                    <input type="hidden" name="action" value="deposit">
                                    
                                    <div class="form-group">
                                        <label for="amount">Amount (gems):</label>
                                        <input type="number" id="amount" name="amount" min="1" required>
                                    </div>

                                    <button type="submit" class="btn btn-primary">Deposit</button>
                                </form>
                            </div>

                        <?php elseif ($action === 'withdraw'): ?>
                            <!-- Withdraw Form -->
                            <div class="form-box">
                                <h3>Withdraw Gems</h3>
                                <form method="POST" action="index.php">
                                    <input type="hidden" name="action" value="withdraw">
                                    
                                    <div class="form-group">
                                        <label for="amount">Amount (gems):</label>
                                        <input type="number" id="amount" name="amount" min="1" max="<?php echo $current_user['gems']; ?>" required>
                                    </div>

                                    <button type="submit" class="btn btn-primary">Withdraw</button>
                                </form>
                            </div>

                        <?php elseif ($action === 'transfer'): ?>
                            <!-- Transfer Form -->
                            <div class="form-box">
                                <h3>Transfer Gems to Another User</h3>
                                <form method="POST" action="index.php">
                                    <input type="hidden" name="action" value="transfer">
                                    
                                    <div class="form-group">
                                        <label for="recipient_username">Recipient Username:</label>
                                        <input type="text" id="recipient_username" name="recipient_username" required>
                                    </div>

                                    <div class="form-group">
                                        <label for="amount">Amount (gems):</label>
                                        <input type="number" id="amount" name="amount" min="1" max="<?php echo $current_user['gems']; ?>" required>
                                    </div>

                                    <button type="submit" class="btn btn-primary">Transfer</button>
                                </form>
                            </div>

                        <?php elseif ($action === 'history'): ?>
                            <!-- Transaction History -->
                            <div class="history-box">
                                <h3>Transaction History</h3>
                                <?php
                                $user_id = $_SESSION['user_id'];
                                $history_result = $conn->query("SELECT id, transaction_type, amount, description, created_at FROM transactions WHERE user_id = $user_id ORDER BY created_at DESC LIMIT 50");

                                if ($history_result && $history_result->num_rows > 0):
                                ?>
                                    <table class="transaction-table">
                                        <thead>
                                            <tr>
                                                <th>Date</th>
                                                <th>Type</th>
                                                <th>Amount</th>
                                                <th>Description</th>
                                            </tr>
                                        </thead>
                                        <tbody>
                                            <?php while ($transaction = $history_result->fetch_assoc()): ?>
                                                <tr>
                                                    <td><?php echo date('Y-m-d H:i:s', strtotime($transaction['created_at'])); ?></td>
                                                    <td><span class="badge badge-<?php echo $transaction['transaction_type']; ?>"><?php echo ucfirst(str_replace('_', ' ', $transaction['transaction_type'])); ?></span></td>
                                                    <td><?php echo $transaction['amount']; ?></td>
                                                    <td><?php echo htmlspecialchars($transaction['description']); ?></td>
                                                </tr>
                                            <?php endwhile; ?>
                                        </tbody>
                                    </table>
                                <?php else: ?>
                                    <p class="no-data">No transactions yet.</p>
                                <?php endif; ?>
                            </div>

                        <?php else: ?>
                            <!-- Dashboard Welcome -->
                            <div class="dashboard-welcome">
                                <h3>Welcome to Gem Banking System!</h3>
                                <p>Manage your digital game currency with ease. Choose an action from the tabs above:</p>
                                <ul>
                                    <li><strong>Deposit:</strong> Add gems to your account</li>
                                    <li><strong>Withdraw:</strong> Remove gems from your account</li>
                                    <li><strong>Transfer:</strong> Send gems to another player</li>
                                    <li><strong>History:</strong> View your transaction history</li>
                                </ul>
                            </div>
                        <?php endif; ?>
                    </div>
                </div>
            <?php endif; ?>
        </main>

        <footer>
            <p>&copy; 2026 Gem Banking System. All rights reserved.</p>
        </footer>
    </div>

    <script>
        function switchAction(action) {
            window.location.href = '?action=' + action;
        }
    </script>
</body>
</html>