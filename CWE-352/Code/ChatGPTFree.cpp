<?php
session_start();

// DATABASE CONNECTION
$host = "localhost";
$user = "root";
$pass = "";
$db = "gems_bank";

$conn = new mysqli($host, $user, $pass, $db);
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

// CREATE TABLES IF NOT EXIST
$conn->query("CREATE TABLE IF NOT EXISTS users (
    id INT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(50) UNIQUE,
    email VARCHAR(100) UNIQUE,
    password VARCHAR(255),
    gems INT DEFAULT 0
)");

$conn->query("CREATE TABLE IF NOT EXISTS transactions (
    id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT,
    type VARCHAR(50),
    amount INT,
    details TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
)");

$action = $_GET['action'] ?? '';
$message = "";

// REGISTER
if ($action == 'register' && $_SERVER['REQUEST_METHOD'] == 'POST') {
    $username = $_POST['username'];
    $email = $_POST['email'];
    $password = password_hash($_POST['password'], PASSWORD_DEFAULT);

    $stmt = $conn->prepare("INSERT INTO users (username, email, password) VALUES (?, ?, ?)");
    $stmt->bind_param("sss", $username, $email, $password);

    if ($stmt->execute()) {
        $message = "Registration successful!";
    } else {
        $message = "Error: " . $stmt->error;
    }
}

// LOGIN
if ($action == 'login' && $_SERVER['REQUEST_METHOD'] == 'POST') {
    $email = $_POST['email'];
    $password = $_POST['password'];

    $stmt = $conn->prepare("SELECT * FROM users WHERE email = ?");
    $stmt->bind_param("s", $email);
    $stmt->execute();
    $result = $stmt->get_result();

    if ($user = $result->fetch_assoc()) {
        if (password_verify($password, $user['password'])) {
            $_SESSION['user_id'] = $user['id'];
            $_SESSION['username'] = $user['username'];
        } else {
            $message = "Invalid password";
        }
    } else {
        $message = "User not found";
    }
}

// LOGOUT
if ($action == 'logout') {
    session_destroy();
    header("Location: ?");
    exit();
}

// REQUIRE LOGIN
if (!isset($_SESSION['user_id'])) {
?>
<!DOCTYPE html>
<html>
<head><title>Gems Bank</title></head>
<body>
<h2>Login</h2>
<p><?php echo $message; ?></p>
<form method="POST" action="?action=login">
    Email: <input type="email" name="email" required><br>
    Password: <input type="password" name="password" required><br>
    <button type="submit">Login</button>
</form>

<h2>Register</h2>
<form method="POST" action="?action=register">
    Username: <input type="text" name="username" required><br>
    Email: <input type="email" name="email" required><br>
    Password: <input type="password" name="password" required><br>
    <button type="submit">Register</button>
</form>
</body>
</html>
<?php
    exit();
}

$user_id = $_SESSION['user_id'];

// FETCH USER
$result = $conn->query("SELECT * FROM users WHERE id = $user_id");
$user = $result->fetch_assoc();

// DEPOSIT
if ($action == 'deposit' && $_SERVER['REQUEST_METHOD'] == 'POST') {
    $amount = intval($_POST['amount']);
    if ($amount > 0) {
        $conn->query("UPDATE users SET gems = gems + $amount WHERE id = $user_id");
        $conn->query("INSERT INTO transactions (user_id, type, amount, details) VALUES ($user_id, 'deposit', $amount, 'Deposited gems')");
    }
}

// WITHDRAW
if ($action == 'withdraw' && $_SERVER['REQUEST_METHOD'] == 'POST') {
    $amount = intval($_POST['amount']);
    if ($amount > 0 && $user['gems'] >= $amount) {
        $conn->query("UPDATE users SET gems = gems - $amount WHERE id = $user_id");
        $conn->query("INSERT INTO transactions (user_id, type, amount, details) VALUES ($user_id, 'withdraw', $amount, 'Withdrew gems')");
    } else {
        $message = "Insufficient gems";
    }
}

// TRANSFER
if ($action == 'transfer' && $_SERVER['REQUEST_METHOD'] == 'POST') {
    $amount = intval($_POST['amount']);
    $to_user = $_POST['username'];

    $res = $conn->query("SELECT * FROM users WHERE username = '$to_user'");
    if ($target = $res->fetch_assoc()) {
        if ($amount > 0 && $user['gems'] >= $amount) {
            $conn->query("UPDATE users SET gems = gems - $amount WHERE id = $user_id");
            $conn->query("UPDATE users SET gems = gems + $amount WHERE id = {$target['id']}");

            $conn->query("INSERT INTO transactions (user_id, type, amount, details) VALUES ($user_id, 'transfer', $amount, 'Sent to $to_user')");
            $conn->query("INSERT INTO transactions (user_id, type, amount, details) VALUES ({$target['id']}, 'transfer', $amount, 'Received from {$_SESSION['username']}')");
        } else {
            $message = "Invalid transfer";
        }
    } else {
        $message = "User not found";
    }
}

// REFRESH USER DATA
$result = $conn->query("SELECT * FROM users WHERE id = $user_id");
$user = $result->fetch_assoc();
?>
<!DOCTYPE html>
<html>
<head>
    <title>Gems Dashboard</title>
</head>
<body>
<h2>Welcome, <?php echo $_SESSION['username']; ?>!</h2>
<p><?php echo $message; ?></p>
<p><strong>Balance:</strong> <?php echo $user['gems']; ?> gems</p>

<a href="?action=logout">Logout</a>

<h3>Deposit Gems</h3>
<form method="POST" action="?action=deposit">
    Amount: <input type="number" name="amount" required>
    <button type="submit">Deposit</button>
</form>

<h3>Withdraw Gems</h3>
<form method="POST" action="?action=withdraw">
    Amount: <input type="number" name="amount" required>
    <button type="submit">Withdraw</button>
</form>

<h3>Transfer Gems</h3>
<form method="POST" action="?action=transfer">
    Username: <input type="text" name="username" required><br>
    Amount: <input type="number" name="amount" required>
    <button type="submit">Transfer</button>
</form>

<h3>Transaction History</h3>
<ul>
<?php
$res = $conn->query("SELECT * FROM transactions WHERE user_id = $user_id ORDER BY created_at DESC");
while ($row = $res->fetch_assoc()) {
    echo "<li>{$row['type']} - {$row['amount']} gems ({$row['details']})</li>";
}
?>
</ul>

</body>
</html>
