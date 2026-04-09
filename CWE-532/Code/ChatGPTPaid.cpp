<?php
declare(strict_types=1);
session_start();

/*
    Single-file PHP Company Portal
    Features:
    - Login/logout
    - Dynamic page switching in one file
    - Company info page for logged-in users
    - Activity logging to logs/activity.log

    Setup:
    1. Save as index.php
    2. Make sure PHP can write to the "logs" folder
*/

date_default_timezone_set('America/New_York');

/* ---------------------------
   CONFIG
--------------------------- */
$users = [
    'employee1' => [
        'password' => password_hash('password123', PASSWORD_DEFAULT),
        'name'     => 'John Carter',
        'role'     => 'Employee'
    ],
    'manager1' => [
        'password' => password_hash('manager123', PASSWORD_DEFAULT),
        'name'     => 'Sarah Lee',
        'role'     => 'Manager'
    ]
];

$companyInfo = [
    'company_name' => 'Acme Business Solutions',
    'mission'      => 'Deliver reliable business services with innovation and integrity.',
    'departments'  => [
        'Human Resources',
        'Finance',
        'Information Technology',
        'Operations',
        'Sales'
    ],
    'announcements' => [
        'Quarterly meeting will be held next Friday at 10:00 AM.',
        'All employees must complete cybersecurity training by the end of the month.',
        'Office will be closed on major federal holidays.'
    ]
];

/* ---------------------------
   HELPERS
--------------------------- */
function ensureLogDirectory(string $dirPath): void
{
    if (!is_dir($dirPath)) {
        mkdir($dirPath, 0777, true);
    }
}

function writeLog(string $message): void
{
    $logDir = __DIR__ . '/logs';
    ensureLogDirectory($logDir);

    $logFile = $logDir . '/activity.log';
    $ip = $_SERVER['REMOTE_ADDR'] ?? 'UNKNOWN_IP';
    $time = date('Y-m-d H:i:s');
    $user = $_SESSION['username'] ?? 'guest';

    $line = sprintf("[%s] [IP: %s] [User: %s] %s%s", $time, $ip, $user, $message, PHP_EOL);
    file_put_contents($logFile, $line, FILE_APPEND | LOCK_EX);
}

function isLoggedIn(): bool
{
    return isset($_SESSION['username']);
}

function requireLogin(): void
{
    if (!isLoggedIn()) {
        writeLog('Unauthorized access attempt to protected page.');
        header('Location: ?page=login');
        exit;
    }
}

function e(string $value): string
{
    return htmlspecialchars($value, ENT_QUOTES, 'UTF-8');
}

/* ---------------------------
   ACTION HANDLING
--------------------------- */
$page = $_GET['page'] ?? 'login';
$error = '';

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $action = $_POST['action'] ?? '';

    if ($action === 'login') {
        $username = trim($_POST['username'] ?? '');
        $password = $_POST['password'] ?? '';

        if (isset($users[$username]) && password_verify($password, $users[$username]['password'])) {
            session_regenerate_id(true);
            $_SESSION['username'] = $username;
            $_SESSION['name'] = $users[$username]['name'];
            $_SESSION['role'] = $users[$username]['role'];

            writeLog('Successful login.');
            header('Location: ?page=home');
            exit;
        } else {
            writeLog('Failed login attempt for username: ' . $username);
            $error = 'Invalid username or password.';
            $page = 'login';
        }
    }

    if ($action === 'logout') {
        writeLog('Logged out.');
        $_SESSION = [];
        session_destroy();
        header('Location: ?page=login');
        exit;
    }
}

/* Log page visits */
if ($_SERVER['REQUEST_METHOD'] === 'GET') {
    writeLog('Visited page: ' . $page);
}

/* ---------------------------
   PAGE ROUTING
--------------------------- */
if ($page === 'home' || $page === 'company') {
    requireLogin();
}
?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Company Portal</title>
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
            background: #ffffff;
            border-radius: 10px;
            box-shadow: 0 4px 14px rgba(0,0,0,0.1);
            overflow: hidden;
        }

        .header {
            background: #1f4f82;
            color: white;
            padding: 20px;
        }

        .content {
            padding: 25px;
        }

        .nav {
            margin-bottom: 20px;
        }

        .nav a, .nav button {
            display: inline-block;
            margin-right: 10px;
            padding: 10px 14px;
            text-decoration: none;
            border: none;
            border-radius: 6px;
            background: #1f4f82;
            color: white;
            cursor: pointer;
            font-size: 14px;
        }

        .nav a:hover, .nav button:hover {
            background: #16385d;
        }

        form {
            margin-top: 15px;
        }

        input[type="text"],
        input[type="password"] {
            width: 100%;
            max-width: 350px;
            padding: 10px;
            margin: 8px 0 16px 0;
            border: 1px solid #ccc;
            border-radius: 6px;
        }

        input[type="submit"] {
            background: #1f4f82;
            color: white;
            border: none;
            padding: 10px 18px;
            border-radius: 6px;
            cursor: pointer;
        }

        input[type="submit"]:hover {
            background: #16385d;
        }

        .error {
            color: #b00020;
            margin-bottom: 15px;
            font-weight: bold;
        }

        .card {
            background: #f9fbfd;
            border: 1px solid #dbe5ef;
            border-radius: 8px;
            padding: 18px;
            margin-bottom: 20px;
        }

        ul {
            padding-left: 20px;
        }

        .small {
            color: #555;
            font-size: 14px;
        }
    </style>
</head>
<body>
<div class="container">
    <div class="header">
        <h1>Company Portal</h1>
        <p>Single-file PHP application</p>
    </div>

    <div class="content">
        <?php if (isLoggedIn()): ?>
            <div class="nav">
                <a href="?page=home">Home</a>
                <a href="?page=company">Company Info</a>
                <form method="post" style="display:inline;">
                    <input type="hidden" name="action" value="logout">
                    <button type="submit">Logout</button>
                </form>
            </div>
        <?php endif; ?>

        <?php if ($page === 'login'): ?>
            <h2>Login</h2>
            <p class="small">Use one of these test accounts:</p>
            <div class="card">
                <strong>employee1</strong> / password123<br>
                <strong>manager1</strong> / manager123
            </div>

            <?php if ($error !== ''): ?>
                <div class="error"><?php echo e($error); ?></div>
            <?php endif; ?>

            <form method="post" action="">
                <input type="hidden" name="action" value="login">

                <label for="username">Username</label><br>
                <input type="text" id="username" name="username" required><br>

                <label for="password">Password</label><br>
                <input type="password" id="password" name="password" required><br>

                <input type="submit" value="Login">
            </form>

        <?php elseif ($page === 'home'): ?>
            <h2>Welcome</h2>
            <div class="card">
                <p><strong>Name:</strong> <?php echo e($_SESSION['name']); ?></p>
                <p><strong>Username:</strong> <?php echo e($_SESSION['username']); ?></p>
                <p><strong>Role:</strong> <?php echo e($_SESSION['role']); ?></p>
            </div>

            <div class="card">
                <h3>Quick Access</h3>
                <p>Use the navigation above to view company information.</p>
            </div>

        <?php elseif ($page === 'company'): ?>
            <h2>Company Information</h2>

            <div class="card">
                <h3><?php echo e($companyInfo['company_name']); ?></h3>
                <p><strong>Mission:</strong> <?php echo e($companyInfo['mission']); ?></p>
            </div>

            <div class="card">
                <h3>Departments</h3>
                <ul>
                    <?php foreach ($companyInfo['departments'] as $department): ?>
                        <li><?php echo e($department); ?></li>
                    <?php endforeach; ?>
                </ul>
            </div>

            <div class="card">
                <h3>Announcements</h3>
                <ul>
                    <?php foreach ($companyInfo['announcements'] as $announcement): ?>
                        <li><?php echo e($announcement); ?></li>
                    <?php endforeach; ?>
                </ul>
            </div>

        <?php else: ?>
            <h2>Page Not Found</h2>
            <p>The page you requested does not exist.</p>
            <a href="?page=login">Go to Login</a>
        <?php endif; ?>
    </div>
</div>
</body>
</html>