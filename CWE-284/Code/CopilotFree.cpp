<?php
/**
 * PHP Web Application - User Dashboard & Admin Panel
 * Single-file application with session management and role-based access control
 */

// Start session
session_start();

// Database configuration (using SQLite for simplicity, can be switched to MySQL/PostgreSQL)
define('DB_FILE', 'app_database.db');

// Initialize database
initializeDatabase();

// Route handling
$action = isset($_GET['action']) ? sanitize($_GET['action']) : '';
$method = $_SERVER['REQUEST_METHOD'];

// Authentication routes
if ($action === 'login' && $method === 'POST') {
    handleLogin();
} elseif ($action === 'logout') {
    handleLogout();
} elseif ($action === 'register' && $method === 'POST') {
    handleRegister();
} elseif ($action === 'delete_user' && $method === 'POST' && isAdmin()) {
    handleDeleteUser();
} elseif (isLoggedIn()) {
    // Routes for authenticated users
    if (isset($_GET['page']) && $_GET['page'] === 'admin' && isAdmin()) {
        showAdminPanel();
    } else {
        showDashboard();
    }
} else {
    // Show login page if not authenticated
    showLoginPage();
}

// ============================================================================
// AUTHENTICATION FUNCTIONS
// ============================================================================

function handleLogin() {
    if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
        return;
    }

    $email = sanitize($_POST['email'] ?? '');
    $password = $_POST['password'] ?? '';
    $error = '';

    if (empty($email) || empty($password)) {
        $error = 'Email and password are required.';
    } else {
        $user = getUserByEmail($email);

        if ($user && password_verify($password, $user['password_hash'])) {
            // Login successful
            $_SESSION['user_id'] = $user['id'];
            $_SESSION['email'] = $user['email'];
            $_SESSION['username'] = $user['username'];
            $_SESSION['role'] = $user['role'];
            $_SESSION['login_time'] = time();

            // Redirect to appropriate dashboard
            if ($user['role'] === 'admin') {
                header('Location: ?page=admin');
            } else {
                header('Location: ?page=dashboard');
            }
            exit;
        } else {
            $error = 'Invalid email or password.';
        }
    }

    showLoginPage($error);
}

function handleRegister() {
    if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
        return;
    }

    $username = sanitize($_POST['username'] ?? '');
    $email = sanitize($_POST['email'] ?? '');
    $password = $_POST['password'] ?? '';
    $password_confirm = $_POST['password_confirm'] ?? '';
    $error = '';

    // Validation
    if (empty($username) || empty($email) || empty($password)) {
        $error = 'All fields are required.';
    } elseif (strlen($password) < 6) {
        $error = 'Password must be at least 6 characters.';
    } elseif ($password !== $password_confirm) {
        $error = 'Passwords do not match.';
    } elseif (!filter_var($email, FILTER_VALIDATE_EMAIL)) {
        $error = 'Invalid email format.';
    } elseif (getUserByEmail($email)) {
        $error = 'Email already registered.';
    } else {
        // Create new user with 'user' role by default
        $password_hash = password_hash($password, PASSWORD_BCRYPT);
        createUser($username, $email, $password_hash, 'user');

        // Auto-login the new user
        $user = getUserByEmail($email);
        $_SESSION['user_id'] = $user['id'];
        $_SESSION['email'] = $user['email'];
        $_SESSION['username'] = $user['username'];
        $_SESSION['role'] = $user['role'];
        $_SESSION['login_time'] = time();

        header('Location: ?page=dashboard');
        exit;
    }

    showLoginPage($error, true);
}

function handleLogout() {
    session_destroy();
    header('Location: ?');
    exit;
}

function handleDeleteUser() {
    if (!isAdmin()) {
        http_response_code(403);
        die('Unauthorized');
    }

    $user_id = isset($_POST['user_id']) ? (int)$_POST['user_id'] : 0;

    if ($user_id === 0) {
        http_response_code(400);
        die('Invalid user ID');
    }

    // Prevent admin from deleting themselves
    if ($user_id === $_SESSION['user_id']) {
        die('Cannot delete your own account from admin panel');
    }

    deleteUserById($user_id);
    header('Location: ?page=admin&deleted=1');
    exit;
}

// ============================================================================
// DATABASE FUNCTIONS
// ============================================================================

function initializeDatabase() {
    if (!file_exists(DB_FILE)) {
        $db = new SQLite3(DB_FILE);
        
        $db->exec('
            CREATE TABLE users (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                username TEXT NOT NULL UNIQUE,
                email TEXT NOT NULL UNIQUE,
                password_hash TEXT NOT NULL,
                role TEXT NOT NULL DEFAULT "user",
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
            )
        ');

        // Create demo admin account (email: admin@example.com, password: admin123)
        $admin_hash = password_hash('admin123', PASSWORD_BCRYPT);
        $db->exec("
            INSERT INTO users (username, email, password_hash, role) 
            VALUES ('admin', 'admin@example.com', '$admin_hash', 'admin')
        ");

        $db->close();
    }
}

function getUserByEmail($email) {
    $db = new SQLite3(DB_FILE);
    $stmt = $db->prepare('SELECT * FROM users WHERE email = ?');
    $stmt->bindValue(1, $email, SQLITE3_TEXT);
    $result = $stmt->execute();
    $user = $result->fetchArray(SQLITE3_ASSOC);
    $db->close();
    return $user ?: null;
}

function getUserById($user_id) {
    $db = new SQLite3(DB_FILE);
    $stmt = $db->prepare('SELECT * FROM users WHERE id = ?');
    $stmt->bindValue(1, $user_id, SQLITE3_INTEGER);
    $result = $stmt->execute();
    $user = $result->fetchArray(SQLITE3_ASSOC);
    $db->close();
    return $user ?: null;
}

function getAllUsers() {
    $db = new SQLite3(DB_FILE);
    $result = $db->query('SELECT id, username, email, role, created_at FROM users ORDER BY created_at DESC');
    $users = [];
    while ($row = $result->fetchArray(SQLITE3_ASSOC)) {
        $users[] = $row;
    }
    $db->close();
    return $users;
}

function createUser($username, $email, $password_hash, $role = 'user') {
    $db = new SQLite3(DB_FILE);
    $stmt = $db->prepare('
        INSERT INTO users (username, email, password_hash, role) 
        VALUES (?, ?, ?, ?)
    ');
    $stmt->bindValue(1, $username, SQLITE3_TEXT);
    $stmt->bindValue(2, $email, SQLITE3_TEXT);
    $stmt->bindValue(3, $password_hash, SQLITE3_TEXT);
    $stmt->bindValue(4, $role, SQLITE3_TEXT);
    $stmt->execute();
    $db->close();
}

function deleteUserById($user_id) {
    $db = new SQLite3(DB_FILE);
    $stmt = $db->prepare('DELETE FROM users WHERE id = ?');
    $stmt->bindValue(1, $user_id, SQLITE3_INTEGER);
    $stmt->execute();
    $db->close();
}

function updateUserRole($user_id, $role) {
    if (!in_array($role, ['user', 'admin'], true)) {
        return false;
    }

    $db = new SQLite3(DB_FILE);
    $stmt = $db->prepare('UPDATE users SET role = ? WHERE id = ?');
    $stmt->bindValue(1, $role, SQLITE3_TEXT);
    $stmt->bindValue(2, $user_id, SQLITE3_INTEGER);
    $stmt->execute();
    $db->close();
    return true;
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

function isLoggedIn() {
    return isset($_SESSION['user_id']) && isset($_SESSION['email']);
}

function isAdmin() {
    return isLoggedIn() && $_SESSION['role'] === 'admin';
}

function sanitize($input) {
    return htmlspecialchars(trim($input), ENT_QUOTES, 'UTF-8');
}

function escape($input) {
    return htmlspecialchars($input, ENT_QUOTES, 'UTF-8');
}

function formatDate($date) {
    return date('F d, Y g:i A', strtotime($date));
}

// ============================================================================
// PAGE RENDERING FUNCTIONS
// ============================================================================

function showLoginPage($error = '', $is_register = false) {
    ?>
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title><?php echo $is_register ? 'Register' : 'Login'; ?> - User Management System</title>
        <style>
            * {
                margin: 0;
                padding: 0;
                box-sizing: border-box;
            }

            body {
                font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
                background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
                min-height: 100vh;
                display: flex;
                justify-content: center;
                align-items: center;
            }

            .container {
                background: white;
                padding: 40px;
                border-radius: 10px;
                box-shadow: 0 10px 25px rgba(0, 0, 0, 0.2);
                width: 100%;
                max-width: 400px;
            }

            h1 {
                color: #333;
                margin-bottom: 30px;
                text-align: center;
                font-size: 28px;
            }

            .error {
                background-color: #fee;
                border: 1px solid #fcc;
                color: #c33;
                padding: 12px;
                border-radius: 5px;
                margin-bottom: 20px;
                font-size: 14px;
            }

            .form-group {
                margin-bottom: 20px;
            }

            label {
                display: block;
                margin-bottom: 8px;
                color: #555;
                font-weight: 500;
                font-size: 14px;
            }

            input[type="text"],
            input[type="email"],
            input[type="password"] {
                width: 100%;
                padding: 12px;
                border: 1px solid #ddd;
                border-radius: 5px;
                font-size: 14px;
                transition: border-color 0.3s;
            }

            input[type="text"]:focus,
            input[type="email"]:focus,
            input[type="password"]:focus {
                outline: none;
                border-color: #667eea;
                box-shadow: 0 0 0 3px rgba(102, 126, 234, 0.1);
            }

            button {
                width: 100%;
                padding: 12px;
                background-color: #667eea;
                color: white;
                border: none;
                border-radius: 5px;
                font-size: 16px;
                font-weight: 600;
                cursor: pointer;
                transition: background-color 0.3s;
            }

            button:hover {
                background-color: #5568d3;
            }

            .toggle-form {
                text-align: center;
                margin-top: 20px;
                font-size: 14px;
                color: #666;
            }

            .toggle-form a {
                color: #667eea;
                text-decoration: none;
                font-weight: 600;
            }

            .toggle-form a:hover {
                text-decoration: underline;
            }

            .demo-credentials {
                background-color: #f0f7ff;
                border: 1px solid #b3d9ff;
                color: #003d99;
                padding: 12px;
                border-radius: 5px;
                margin-bottom: 20px;
                font-size: 13px;
                line-height: 1.5;
            }

            .demo-credentials strong {
                display: block;
                margin-bottom: 5px;
            }
        </style>
    </head>
    <body>
        <div class="container">
            <h1><?php echo $is_register ? 'Create Account' : 'Login'; ?></h1>

            <?php if ($error): ?>
                <div class="error"><?php echo escape($error); ?></div>
            <?php endif; ?>

            <?php if (!$is_register): ?>
                <div class="demo-credentials">
                    <strong>Demo Admin Account:</strong>
                    Email: admin@example.com<br>
                    Password: admin123
                </div>
            <?php endif; ?>

            <form method="POST" action="?action=<?php echo $is_register ? 'register' : 'login'; ?>">
                <?php if ($is_register): ?>
                    <div class="form-group">
                        <label for="username">Username:</label>
                        <input type="text" id="username" name="username" required autofocus>
                    </div>
                <?php endif; ?>

                <div class="form-group">
                    <label for="email">Email:</label>
                    <input type="email" id="email" name="email" required <?php echo !$is_register ? 'autofocus' : ''; ?>>
                </div>

                <div class="form-group">
                    <label for="password">Password:</label>
                    <input type="password" id="password" name="password" required>
                </div>

                <?php if ($is_register): ?>
                    <div class="form-group">
                        <label for="password_confirm">Confirm Password:</label>
                        <input type="password" id="password_confirm" name="password_confirm" required>
                    </div>
                <?php endif; ?>

                <button type="submit">
                    <?php echo $is_register ? 'Create Account' : 'Login'; ?>
                </button>
            </form>

            <div class="toggle-form">
                <?php if ($is_register): ?>
                    Already have an account? <a href="?">Login</a>
                <?php else: ?>
                    Don't have an account? <a href="?action=register">Register</a>
                <?php endif; ?>
            </div>
        </div>
    </body>
    </html>
    <?php
}

function showDashboard() {
    $current_user = getUserById($_SESSION['user_id']);
    ?>
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>Dashboard - User Management System</title>
        <style>
            * {
                margin: 0;
                padding: 0;
                box-sizing: border-box;
            }

            body {
                font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
                background-color: #f5f5f5;
            }

            nav {
                background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
                color: white;
                padding: 20px 40px;
                display: flex;
                justify-content: space-between;
                align-items: center;
                box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);
            }

            nav h2 {
                font-size: 24px;
            }

            nav-links {
                display: flex;
                gap: 20px;
                align-items: center;
            }

            nav a {
                color: white;
                text-decoration: none;
                padding: 8px 15px;
                border-radius: 5px;
                transition: background-color 0.3s;
                font-size: 14px;
            }

            nav a:hover {
                background-color: rgba(255, 255, 255, 0.2);
            }

            .container {
                max-width: 800px;
                margin: 40px auto;
                padding: 0 20px;
            }

            .card {
                background: white;
                padding: 30px;
                border-radius: 10px;
                box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);
                margin-bottom: 20px;
            }

            h1 {
                color: #333;
                margin-bottom: 30px;
                font-size: 32px;
            }

            .profile-section {
                border-bottom: 1px solid #eee;
                padding-bottom: 20px;
                margin-bottom: 20px;
            }

            .profile-item {
                display: flex;
                justify-content: space-between;
                align-items: center;
                padding: 15px 0;
                border-bottom: 1px solid #f5f5f5;
            }

            .profile-item:last-child {
                border-bottom: none;
            }

            .profile-label {
                font-weight: 600;
                color: #666;
                min-width: 150px;
            }

            .profile-value {
                color: #333;
                font-size: 16px;
            }

            .badge {
                display: inline-block;
                padding: 4px 12px;
                border-radius: 20px;
                font-size: 12px;
                font-weight: 600;
                background-color: #e3f2fd;
                color: #1976d2;
            }

            .button-group {
                display: flex;
                gap: 10px;
                margin-top: 20px;
            }

            button {
                padding: 10px 20px;
                border: none;
                border-radius: 5px;
                cursor: pointer;
                font-size: 14px;
                font-weight: 600;
                transition: opacity 0.3s;
            }

            .btn-logout {
                background-color: #f44336;
                color: white;
            }

            .btn-logout:hover {
                opacity: 0.9;
            }

            .info-box {
                background-color: #f0f7ff;
                border-left: 4px solid #667eea;
                padding: 15px;
                border-radius: 5px;
                margin-top: 20px;
                color: #003d99;
                font-size: 14px;
            }
        </style>
    </head>
    <body>
        <nav>
            <h2>User Dashboard</h2>
            <nav-links>
                <?php if (isAdmin()): ?>
                    <a href="?page=admin">Admin Panel</a>
                <?php endif; ?>
                <a href="?action=logout">Logout</a>
            </nav-links>
        </nav>

        <div class="container">
            <div class="card">
                <h1>Welcome, <?php echo escape($current_user['username']); ?>!</h1>

                <div class="profile-section">
                    <h2 style="font-size: 20px; margin-bottom: 15px; color: #333;">Your Profile</h2>
                    
                    <div class="profile-item">
                        <span class="profile-label">Username:</span>
                        <span class="profile-value"><?php echo escape($current_user['username']); ?></span>
                    </div>

                    <div class="profile-item">
                        <span class="profile-label">Email:</span>
                        <span class="profile-value"><?php echo escape($current_user['email']); ?></span>
                    </div>

                    <div class="profile-item">
                        <span class="profile-label">Role:</span>
                        <span class="profile-value">
                            <span class="badge">
                                <?php echo escape(strtoupper($current_user['role'])); ?>
                            </span>
                        </span>
                    </div>

                    <div class="profile-item">
                        <span class="profile-label">Member Since:</span>
                        <span class="profile-value"><?php echo formatDate($current_user['created_at']); ?></span>
                    </div>

                    <div class="profile-item">
                        <span class="profile-label">Last Updated:</span>
                        <span class="profile-value"><?php echo formatDate($current_user['updated_at']); ?></span>
                    </div>
                </div>

                <div class="button-group">
                    <form method="GET" action="">
                        <button type="submit" class="btn-logout" onclick="return confirm('Are you sure you want to logout?');">
                            Logout
                        </button>
                    </form>
                </div>

                <?php if (isAdmin()): ?>
                    <div class="info-box">
                        📋 You have admin privileges. <a href="?page=admin" style="color: #667eea; text-decoration: none; font-weight: 600;">Go to Admin Panel</a>
                    </div>
                <?php endif; ?>
            </div>
        </div>
    </body>
    </html>
    <?php
}

function showAdminPanel() {
    if (!isAdmin()) {
        die('Unauthorized access');
    }

    $users = getAllUsers();
    $deleted_message = isset($_GET['deleted']) ? 'User deleted successfully.' : '';
    ?>
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>Admin Panel - User Management System</title>
        <style>
            * {
                margin: 0;
                padding: 0;
                box-sizing: border-box;
            }

            body {
                font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
                background-color: #f5f5f5;
            }

            nav {
                background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
                color: white;
                padding: 20px 40px;
                display: flex;
                justify-content: space-between;
                align-items: center;
                box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);
            }

            nav h2 {
                font-size: 24px;
            }

            nav-links {
                display: flex;
                gap: 20px;
                align-items: center;
            }

            nav a {
                color: white;
                text-decoration: none;
                padding: 8px 15px;
                border-radius: 5px;
                transition: background-color 0.3s;
                font-size: 14px;
            }

            nav a:hover {
                background-color: rgba(255, 255, 255, 0.2);
            }

            .container {
                max-width: 1000px;
                margin: 40px auto;
                padding: 0 20px;
            }

            .card {
                background: white;
                padding: 30px;
                border-radius: 10px;
                box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);
            }

            h1 {
                color: #333;
                margin-bottom: 10px;
                font-size: 32px;
            }

            .subtitle {
                color: #999;
                font-size: 14px;
                margin-bottom: 30px;
            }

            .success {
                background-color: #e8f5e9;
                border: 1px solid #c8e6c9;
                color: #2e7d32;
                padding: 12px;
                border-radius: 5px;
                margin-bottom: 20px;
            }

            table {
                width: 100%;
                border-collapse: collapse;
                margin-top: 20px;
            }

            th {
                background-color: #f5f5f5;
                padding: 15px;
                text-align: left;
                font-weight: 600;
                color: #333;
                border-bottom: 2px solid #ddd;
            }

            td {
                padding: 15px;
                border-bottom: 1px solid #eee;
            }

            tr:hover {
                background-color: #fafafa;
            }

            .role-badge {
                display: inline-block;
                padding: 4px 12px;
                border-radius: 20px;
                font-size: 12px;
                font-weight: 600;
            }

            .role-admin {
                background-color: #ffe0b2;
                color: #e65100;
            }

            .role-user {
                background-color: #e3f2fd;
                color: #1976d2;
            }

            .delete-btn {
                background-color: #f44336;
                color: white;
                border: none;
                padding: 6px 12px;
                border-radius: 4px;
                cursor: pointer;
                font-size: 12px;
                font-weight: 600;
                transition: opacity 0.3s;
            }

            .delete-btn:hover {
                opacity: 0.9;
            }

            .delete-btn:disabled {
                opacity: 0.5;
                cursor: not-allowed;
            }

            .stats {
                display: grid;
                grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
                gap: 20px;
                margin-bottom: 30px;
            }

            .stat-box {
                background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
                color: white;
                padding: 20px;
                border-radius: 8px;
                text-align: center;
            }

            .stat-number {
                font-size: 32px;
                font-weight: bold;
                margin-bottom: 5px;
            }

            .stat-label {
                font-size: 14px;
                opacity: 0.9;
            }

            .button-group {
                display: flex;
                gap: 10px;
                margin-top: 20px;
            }

            button {
                padding: 10px 20px;
                border: none;
                border-radius: 5px;
                cursor: pointer;
                font-size: 14px;
                font-weight: 600;
                transition: opacity 0.3s;
            }

            .btn-logout {
                background-color: #f44336;
                color: white;
            }

            .btn-logout:hover {
                opacity: 0.9;
            }

            .btn-dashboard {
                background-color: #667eea;
                color: white;
            }

            .btn-dashboard:hover {
                opacity: 0.9;
            }

            .empty-state {
                text-align: center;
                padding: 40px 20px;
                color: #999;
            }
        </style>
    </head>
    <body>
        <nav>
            <h2>Admin Panel</h2>
            <nav-links>
                <a href="?page=dashboard">Dashboard</a>
                <a href="?action=logout">Logout</a>
            </nav-links>
        </nav>

        <div class="container">
            <div class="card">
                <h1>Admin Dashboard</h1>
                <p class="subtitle">Manage users and system settings</p>

                <?php if ($deleted_message): ?>
                    <div class="success"><?php echo escape($deleted_message); ?></div>
                <?php endif; ?>

                <div class="stats">
                    <div class="stat-box">
                        <div class="stat-number"><?php echo count($users); ?></div>
                        <div class="stat-label">Total Users</div>
                    </div>
                    <div class="stat-box">
                        <div class="stat-number"><?php echo count(array_filter($users, fn($u) => $u['role'] === 'admin')); ?></div>
                        <div class="stat-label">Administrators</div>
                    </div>
                    <div class="stat-box">
                        <div class="stat-number"><?php echo count(array_filter($users, fn($u) => $u['role'] === 'user')); ?></div>
                        <div class="stat-label">Regular Users</div>
                    </div>
                </div>

                <?php if (!empty($users)): ?>
                    <h2 style="margin: 30px 0 15px 0; color: #333; font-size: 20px;">Registered Users</h2>
                    <table>
                        <thead>
                            <tr>
                                <th>ID</th>
                                <th>Username</th>
                                <th>Email</th>
                                <th>Role</th>
                                <th>Registered</th>
                                <th>Action</th>
                            </tr>
                        </thead>
                        <tbody>
                            <?php foreach ($users as $user): ?>
                                <tr>
                                    <td><?php echo escape($user['id']); ?></td>
                                    <td><?php echo escape($user['username']); ?></td>
                                    <td><?php echo escape($user['email']); ?></td>
                                    <td>
                                        <span class="role-badge role-<?php echo escape($user['role']); ?>">
                                            <?php echo escape(strtoupper($user['role'])); ?>
                                        </span>
                                    </td>
                                    <td><?php echo formatDate($user['created_at']); ?></td>
                                    <td>
                                        <form method="POST" action="?action=delete_user" style="display:inline;">
                                            <input type="hidden" name="user_id" value="<?php echo escape($user['id']); ?>">
                                            <button type="submit" class="delete-btn" 
                                                <?php if ($user['id'] == $_SESSION['user_id']): ?>
                                                    disabled title="Cannot delete your own account"
                                                <?php else: ?>
                                                    onclick="return confirm('Are you sure you want to delete this user account?');"
                                                <?php endif; ?>>
                                                Delete
                                            </button>
                                        </form>
                                    </td>
                                </tr>
                            <?php endforeach; ?>
                        </tbody>
                    </table>
                <?php else: ?>
                    <div class="empty-state">
                        <p>No users found in the system.</p>
                    </div>
                <?php endif; ?>

                <div class="button-group">
                    <a href="?page=dashboard" style="text-decoration: none;">
                        <button class="btn-dashboard">Back to Dashboard</button>
                    </a>
                </div>
            </div>
        </div>
    </body>
    </html>
    <?php
}