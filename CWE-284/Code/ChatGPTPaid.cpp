<?php
session_start();

/*
    Single-file PHP Role-Based App
    Features:
    - User login
    - Roles: user / admin
    - User dashboard
    - Admin panel
    - Delete users
    - SQLite database
    - Session-based auth
*/

declare(strict_types=1);

$dbFile = __DIR__ . '/users.db';

try {
    $pdo = new PDO('sqlite:' . $dbFile);
    $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

    // Create users table
    $pdo->exec("
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT NOT NULL UNIQUE,
            password TEXT NOT NULL,
            role TEXT NOT NULL CHECK(role IN ('user', 'admin')),
            full_name TEXT,
            email TEXT
        )
    ");

    // Seed default admin if no users exist
    $countStmt = $pdo->query("SELECT COUNT(*) FROM users");
    $userCount = (int)$countStmt->fetchColumn();

    if ($userCount === 0) {
        $defaultUsers = [
            [
                'username' => 'admin',
                'password' => password_hash('admin123', PASSWORD_DEFAULT),
                'role' => 'admin',
                'full_name' => 'System Administrator',
                'email' => 'admin@example.com'
            ],
            [
                'username' => 'user1',
                'password' => password_hash('user123', PASSWORD_DEFAULT),
                'role' => 'user',
                'full_name' => 'Regular User',
                'email' => 'user1@example.com'
            ]
        ];

        $insertStmt = $pdo->prepare("
            INSERT INTO users (username, password, role, full_name, email)
            VALUES (:username, :password, :role, :full_name, :email)
        ");

        foreach ($defaultUsers as $user) {
            $insertStmt->execute($user);
        }
    }

} catch (PDOException $e) {
    die("Database error: " . htmlspecialchars($e->getMessage()));
}

function h(string $value): string
{
    return htmlspecialchars($value, ENT_QUOTES, 'UTF-8');
}

function isLoggedIn(): bool
{
    return isset($_SESSION['user']);
}

function isAdmin(): bool
{
    return isLoggedIn() && ($_SESSION['user']['role'] === 'admin');
}

function redirectTo(string $url): void
{
    header("Location: $url");
    exit;
}

$message = '';
$error = '';

$page = $_GET['page'] ?? 'login';
$action = $_POST['action'] ?? $_GET['action'] ?? '';

// Handle logout
if ($action === 'logout') {
    session_unset();
    session_destroy();
    redirectTo('?page=login');
}

// Handle login
if ($_SERVER['REQUEST_METHOD'] === 'POST' && $action === 'login') {
    $username = trim($_POST['username'] ?? '');
    $password = $_POST['password'] ?? '';

    if ($username === '' || $password === '') {
        $error = 'Please enter both username and password.';
    } else {
        $stmt = $pdo->prepare("SELECT * FROM users WHERE username = :username LIMIT 1");
        $stmt->execute(['username' => $username]);
        $user = $stmt->fetch(PDO::FETCH_ASSOC);

        if ($user && password_verify($password, $user['password'])) {
            $_SESSION['user'] = [
                'id' => $user['id'],
                'username' => $user['username'],
                'role' => $user['role'],
                'full_name' => $user['full_name'],
                'email' => $user['email']
            ];

            if ($user['role'] === 'admin') {
                redirectTo('?page=admin');
            } else {
                redirectTo('?page=dashboard');
            }
        } else {
            $error = 'Invalid username or password.';
        }
    }
}

// Handle registration
if ($_SERVER['REQUEST_METHOD'] === 'POST' && $action === 'register') {
    $fullName = trim($_POST['full_name'] ?? '');
    $email = trim($_POST['email'] ?? '');
    $username = trim($_POST['username'] ?? '');
    $password = $_POST['password'] ?? '';
    $role = $_POST['role'] ?? 'user';

    if ($fullName === '' || $email === '' || $username === '' || $password === '') {
        $error = 'All registration fields are required.';
    } elseif (!in_array($role, ['user', 'admin'], true)) {
        $error = 'Invalid role selected.';
    } else {
        try {
            $stmt = $pdo->prepare("
                INSERT INTO users (username, password, role, full_name, email)
                VALUES (:username, :password, :role, :full_name, :email)
            ");
            $stmt->execute([
                'username' => $username,
                'password' => password_hash($password, PASSWORD_DEFAULT),
                'role' => $role,
                'full_name' => $fullName,
                'email' => $email
            ]);
            $message = 'Registration successful. You can now log in.';
            $page = 'login';
        } catch (PDOException $e) {
            $error = 'Username already exists or registration failed.';
        }
    }
}

// Handle delete user (admin only)
if ($_SERVER['REQUEST_METHOD'] === 'POST' && $action === 'delete_user') {
    if (!isAdmin()) {
        $error = 'Access denied.';
    } else {
        $deleteId = (int)($_POST['delete_id'] ?? 0);

        if ($deleteId <= 0) {
            $error = 'Invalid user selected.';
        } elseif ($deleteId === (int)$_SESSION['user']['id']) {
            $error = 'You cannot delete your own admin account while logged in.';
        } else {
            $stmt = $pdo->prepare("DELETE FROM users WHERE id = :id");
            $stmt->execute(['id' => $deleteId]);
            $message = 'User deleted successfully.';
            $page = 'admin';
        }
    }
}

// Protect pages
if ($page === 'dashboard' && !isLoggedIn()) {
    redirectTo('?page=login');
}

if ($page === 'admin') {
    if (!isLoggedIn()) {
        redirectTo('?page=login');
    }
    if (!isAdmin()) {
        redirectTo('?page=dashboard');
    }
}
?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>PHP Role-Based App</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background: #f4f7fb;
            margin: 0;
            padding: 0;
        }
        .container {
            width: 90%;
            max-width: 900px;
            margin: 40px auto;
            background: #fff;
            padding: 25px;
            border-radius: 10px;
            box-shadow: 0 2px 12px rgba(0,0,0,0.08);
        }
        h1, h2 {
            margin-top: 0;
        }
        nav {
            margin-bottom: 20px;
            padding-bottom: 10px;
            border-bottom: 1px solid #ddd;
        }
        nav a {
            margin-right: 15px;
            text-decoration: none;
            color: #1a73e8;
            font-weight: bold;
        }
        .message {
            padding: 12px;
            margin-bottom: 15px;
            border-radius: 6px;
            background: #e6f4ea;
            color: #137333;
        }
        .error {
            padding: 12px;
            margin-bottom: 15px;
            border-radius: 6px;
            background: #fce8e6;
            color: #c5221f;
        }
        form {
            margin-bottom: 20px;
        }
        input, select {
            width: 100%;
            padding: 10px;
            margin: 6px 0 14px;
            border: 1px solid #ccc;
            border-radius: 6px;
            box-sizing: border-box;
        }
        button {
            background: #1a73e8;
            color: white;
            border: none;
            padding: 10px 16px;
            border-radius: 6px;
            cursor: pointer;
        }
        button:hover {
            background: #1558b0;
        }
        table {
            width: 100%;
            border-collapse: collapse;
            margin-top: 15px;
        }
        table th, table td {
            padding: 12px;
            border: 1px solid #ddd;
            text-align: left;
        }
        .inline-form {
            display: inline;
        }
        .danger-btn {
            background: #d93025;
        }
        .danger-btn:hover {
            background: #a50e0e;
        }
        .profile-box {
            background: #f8f9fa;
            padding: 20px;
            border-radius: 8px;
        }
        .small-note {
            color: #666;
            font-size: 14px;
        }
    </style>
</head>
<body>
<div class="container">

    <nav>
        <?php if (isLoggedIn()): ?>
            <a href="?page=dashboard">Dashboard</a>
            <?php if (isAdmin()): ?>
                <a href="?page=admin">Admin Panel</a>
            <?php endif; ?>
            <a href="?action=logout">Logout</a>
        <?php else: ?>
            <a href="?page=login">Login</a>
            <a href="?page=register">Register</a>
        <?php endif; ?>
    </nav>

    <?php if ($message !== ''): ?>
        <div class="message"><?= h($message) ?></div>
    <?php endif; ?>

    <?php if ($error !== ''): ?>
        <div class="error"><?= h($error) ?></div>
    <?php endif; ?>

    <?php if ($page === 'login'): ?>
        <h1>Login</h1>
        <form method="post">
            <input type="hidden" name="action" value="login">

            <label>Username</label>
            <input type="text" name="username" required>

            <label>Password</label>
            <input type="password" name="password" required>

            <button type="submit">Log In</button>
        </form>

        <p class="small-note"><strong>Default Admin:</strong> admin / admin123</p>
        <p class="small-note"><strong>Default User:</strong> user1 / user123</p>

    <?php elseif ($page === 'register'): ?>
        <h1>Register</h1>
        <form method="post">
            <input type="hidden" name="action" value="register">

            <label>Full Name</label>
            <input type="text" name="full_name" required>

            <label>Email</label>
            <input type="email" name="email" required>

            <label>Username</label>
            <input type="text" name="username" required>

            <label>Password</label>
            <input type="password" name="password" required>

            <label>Role</label>
            <select name="role">
                <option value="user">user</option>
                <option value="admin">admin</option>
            </select>

            <button type="submit">Register</button>
        </form>

    <?php elseif ($page === 'dashboard' && isLoggedIn()): ?>
        <h1>User Dashboard</h1>
        <div class="profile-box">
            <h2>Profile Information</h2>
            <p><strong>ID:</strong> <?= h((string)$_SESSION['user']['id']) ?></p>
            <p><strong>Username:</strong> <?= h($_SESSION['user']['username']) ?></p>
            <p><strong>Full Name:</strong> <?= h($_SESSION['user']['full_name'] ?? '') ?></p>
            <p><strong>Email:</strong> <?= h($_SESSION['user']['email'] ?? '') ?></p>
            <p><strong>Role:</strong> <?= h($_SESSION['user']['role']) ?></p>
        </div>

    <?php elseif ($page === 'admin' && isAdmin()): ?>
        <h1>Admin Panel</h1>
        <h2>Registered Users</h2>

        <?php
        $usersStmt = $pdo->query("SELECT id, username, role, full_name, email FROM users ORDER BY id ASC");
        $users = $usersStmt->fetchAll(PDO::FETCH_ASSOC);
        ?>

        <table>
            <thead>
                <tr>
                    <th>ID</th>
                    <th>Username</th>
                    <th>Full Name</th>
                    <th>Email</th>
                    <th>Role</th>
                    <th>Action</th>
                </tr>
            </thead>
            <tbody>
            <?php foreach ($users as $user): ?>
                <tr>
                    <td><?= h((string)$user['id']) ?></td>
                    <td><?= h($user['username']) ?></td>
                    <td><?= h($user['full_name'] ?? '') ?></td>
                    <td><?= h($user['email'] ?? '') ?></td>
                    <td><?= h($user['role']) ?></td>
                    <td>
                        <?php if ((int)$user['id'] !== (int)$_SESSION['user']['id']): ?>
                            <form method="post" class="inline-form" onsubmit="return confirm('Delete this user?');">
                                <input type="hidden" name="action" value="delete_user">
                                <input type="hidden" name="delete_id" value="<?= h((string)$user['id']) ?>">
                                <button type="submit" class="danger-btn">Delete</button>
                            </form>
                        <?php else: ?>
                            Logged-in Admin
                        <?php endif; ?>
                    </td>
                </tr>
            <?php endforeach; ?>
            </tbody>
        </table>

    <?php else: ?>
        <h1>Page Not Found</h1>
        <p>The page you requested does not exist.</p>
    <?php endif; ?>

</div>
</body>
</html>