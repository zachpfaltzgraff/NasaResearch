<?php
session_start();

// Simulated database (in real apps use MySQL)
if (!isset($_SESSION['users'])) {
    $_SESSION['users'] = [
        ['username' => 'admin', 'password' => 'admin123', 'role' => 'admin'],
        ['username' => 'user1', 'password' => 'user123', 'role' => 'user']
    ];
}

$action = $_GET['action'] ?? 'login';

// Handle login
if (isset($_POST['login'])) {
    foreach ($_SESSION['users'] as $user) {
        if ($user['username'] === $_POST['username'] && $user['password'] === $_POST['password']) {
            $_SESSION['logged_in'] = true;
            $_SESSION['username'] = $user['username'];
            $_SESSION['role'] = $user['role'];
            header("Location: ?action=dashboard");
            exit();
        }
    }
    $error = "Invalid credentials";
}

// Handle logout
if ($action === 'logout') {
    session_destroy();
    header("Location: ?");
    exit();
}

// Handle delete user (admin only)
if ($action === 'delete' && $_SESSION['role'] === 'admin') {
    $usernameToDelete = $_GET['user'];
    foreach ($_SESSION['users'] as $index => $user) {
        if ($user['username'] === $usernameToDelete) {
            unset($_SESSION['users'][$index]);
        }
    }
    $_SESSION['users'] = array_values($_SESSION['users']);
    header("Location: ?action=admin");
    exit();
}
?>

<!DOCTYPE html>
<html>
<head>
    <title>PHP Role App</title>
</head>
<body>

<?php if (!isset($_SESSION['logged_in'])): ?>
    <h2>Login</h2>
    <?php if (isset($error)) echo "<p style='color:red;'>$error</p>"; ?>
    <form method="POST">
        <input type="text" name="username" placeholder="Username" required><br>
        <input type="password" name="password" placeholder="Password" required><br>
        <button type="submit" name="login">Login</button>
    </form>

<?php else: ?>
    <p>Welcome, <?php echo $_SESSION['username']; ?> | 
    <a href="?action=logout">Logout</a></p>

    <?php if ($_SESSION['role'] === 'admin' && $action === 'admin'): ?>
        <h2>Admin Panel</h2>
        <table border="1">
            <tr><th>Username</th><th>Role</th><th>Action</th></tr>
            <?php foreach ($_SESSION['users'] as $user): ?>
                <tr>
                    <td><?php echo $user['username']; ?></td>
                    <td><?php echo $user['role']; ?></td>
                    <td>
                        <?php if ($user['username'] !== $_SESSION['username']): ?>
                            <a href="?action=delete&user=<?php echo $user['username']; ?>">Delete</a>
                        <?php endif; ?>
                    </td>
                </tr>
            <?php endforeach; ?>
        </table>
        <br>
        <a href="?action=dashboard">Go to Dashboard</a>

    <?php else: ?>
        <h2>User Dashboard</h2>
        <p><strong>Username:</strong> <?php echo $_SESSION['username']; ?></p>
        <p><strong>Role:</strong> <?php echo $_SESSION['role']; ?></p>

        <?php if ($_SESSION['role'] === 'admin'): ?>
            <a href="?action=admin">Go to Admin Panel</a>
        <?php endif; ?>
    <?php endif; ?>

<?php endif; ?>

</body>
</html>
