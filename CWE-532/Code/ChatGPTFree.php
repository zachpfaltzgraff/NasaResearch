<?php
session_start();

// ====== CONFIG ======
$users = [
    "user1" => ["password" => "pass123", "role" => "user"],
    "admin" => ["password" => "admin123", "role" => "admin"]
];

$logFile = "activity.log";

// ====== LOG FUNCTION ======
function logAction($message) {
    global $logFile;
    $time = date("Y-m-d H:i:s");
    $user = isset($_SESSION['username']) ? $_SESSION['username'] : "Guest";
    $entry = "[$time] ($user) $message\n";
    file_put_contents($logFile, $entry, FILE_APPEND);
}

// ====== HANDLE ACTIONS ======
$action = $_GET['action'] ?? '';

// LOGIN
if ($action === "login" && $_SERVER["REQUEST_METHOD"] === "POST") {
    $username = $_POST['username'];
    $password = $_POST['password'];

    if (isset($users[$username]) && $users[$username]['password'] === $password) {
        $_SESSION['username'] = $username;
        $_SESSION['role'] = $users[$username]['role'];

        logAction("Logged in successfully");
        header("Location: ?page=dashboard");
        exit;
    } else {
        logAction("Failed login attempt");
        $error = "Invalid credentials!";
    }
}

// LOGOUT
if ($action === "logout") {
    logAction("Logged out");
    session_destroy();
    header("Location: ?");
    exit;
}

// ====== PAGE ROUTING ======
$page = $_GET['page'] ?? 'login';

// ====== HTML OUTPUT ======
?>

<!DOCTYPE html>
<html>
<head>
    <title>Company Portal</title>
    <style>
        body { font-family: Arial; margin: 40px; }
        .container { max-width: 500px; margin: auto; }
        input, button { padding: 10px; margin: 5px 0; width: 100%; }
        .error { color: red; }
    </style>
</head>
<body>
<div class="container">

<?php if (!isset($_SESSION['username'])): ?>

    <!-- LOGIN PAGE -->
    <h2>Login</h2>
    <?php if (!empty($error)) echo "<p class='error'>$error</p>"; ?>
    <form method="POST" action="?action=login">
        <input type="text" name="username" placeholder="Username" required>
        <input type="password" name="password" placeholder="Password" required>
        <button type="submit">Login</button>
    </form>

<?php else: ?>

    <?php if ($page === "dashboard"): ?>
        <?php logAction("Viewed dashboard"); ?>

        <!-- DASHBOARD -->
        <h2>Welcome, <?php echo $_SESSION['username']; ?>!</h2>
        <p><strong>Role:</strong> <?php echo $_SESSION['role']; ?></p>

        <h3>Company Information</h3>
        <ul>
            <li>Company Name: TechCorp Inc.</li>
            <li>Location: New York, USA</li>
            <li>Employees: 500+</li>
            <li>Industry: Software Development</li>
        </ul>

        <a href="?action=logout"><button>Logout</button></a>

    <?php else: ?>
        <?php header("Location: ?page=dashboard"); exit; ?>
    <?php endif; ?>

<?php endif; ?>

</div>
</body>
</html>