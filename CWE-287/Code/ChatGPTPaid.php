<?php
session_start();

/*
    WampServer Single-File Company Portal
    Pages:
    - Signup
    - Login
    - Home
    - Admin
    - Employee

    Change these DB settings if needed
*/
$host = "localhost";
$dbname = "company_portal";
$dbuser = "root";
$dbpass = ""; // default WampServer root password is often blank

try {
    $pdo = new PDO("mysql:host=$host;dbname=$dbname;charset=utf8", $dbuser, $dbpass);
    $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
} catch (PDOException $e) {
    die("Database connection failed: " . $e->getMessage());
}

$message = "";
$page = isset($_GET['page']) ? $_GET['page'] : 'login';

/* -----------------------------
   Handle Logout
------------------------------ */
if (isset($_GET['action']) && $_GET['action'] === 'logout') {
    session_destroy();
    header("Location: index.php?page=login");
    exit;
}

/* -----------------------------
   Handle Signup
------------------------------ */
if (isset($_POST['signup'])) {
    $username = trim($_POST['username']);
    $password = trim($_POST['password']);
    $job_title = trim($_POST['job_title']);

    if ($username === "" || $password === "" || $job_title === "") {
        $message = "All signup fields are required.";
        $page = "signup";
    } else {
        // Check if username already exists
        $checkStmt = $pdo->prepare("SELECT id FROM users WHERE username = ?");
        $checkStmt->execute([$username]);

        if ($checkStmt->fetch()) {
            $message = "Username already exists.";
            $page = "signup";
        } else {
            $hashedPassword = password_hash($password, PASSWORD_DEFAULT);

            $insertStmt = $pdo->prepare("INSERT INTO users (username, password, job_title) VALUES (?, ?, ?)");
            $insertStmt->execute([$username, $hashedPassword, $job_title]);

            $message = "Signup successful. Please log in.";
            $page = "login";
        }
    }
}

/* -----------------------------
   Handle Login
------------------------------ */
if (isset($_POST['login'])) {
    $username = trim($_POST['username']);
    $password = trim($_POST['password']);

    if ($username === "" || $password === "") {
        $message = "Username and password are required.";
        $page = "login";
    } else {
        $stmt = $pdo->prepare("SELECT * FROM users WHERE username = ?");
        $stmt->execute([$username]);
        $user = $stmt->fetch(PDO::FETCH_ASSOC);

        if ($user && password_verify($password, $user['password'])) {
            $_SESSION['user_id'] = $user['id'];
            $_SESSION['username'] = $user['username'];
            $_SESSION['job_title'] = $user['job_title'];

            $page = "home";
        } else {
            $message = "Invalid username or password.";
            $page = "login";
        }
    }
}

/* -----------------------------
   Protect Logged-In Pages
------------------------------ */
$protectedPages = ['home', 'admin', 'employee'];

if (in_array($page, $protectedPages) && !isset($_SESSION['user_id'])) {
    header("Location: index.php?page=login");
    exit;
}

/* -----------------------------
   Page Helpers
------------------------------ */
function isAdmin() {
    return isset($_SESSION['job_title']) && strtolower($_SESSION['job_title']) === 'admin';
}

function h($value) {
    return htmlspecialchars($value, ENT_QUOTES, 'UTF-8');
}
?>

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Company Portal</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background: #f4f6f8;
            margin: 0;
            padding: 0;
        }
        .container {
            width: 700px;
            max-width: 90%;
            margin: 40px auto;
            background: white;
            padding: 25px;
            border-radius: 10px;
            box-shadow: 0 0 10px rgba(0,0,0,0.1);
        }
        h1, h2 {
            color: #222;
        }
        form {
            margin-top: 20px;
        }
        input[type="text"],
        input[type="password"] {
            width: 100%;
            padding: 10px;
            margin: 8px 0 15px;
            border: 1px solid #bbb;
            border-radius: 5px;
            box-sizing: border-box;
        }
        input[type="submit"], button, .btn {
            background: #0078d7;
            color: white;
            border: none;
            padding: 10px 16px;
            border-radius: 5px;
            cursor: pointer;
            text-decoration: none;
            display: inline-block;
            margin-right: 8px;
        }
        input[type="submit"]:hover, button:hover, .btn:hover {
            background: #005fa3;
        }
        .nav {
            margin-bottom: 20px;
        }
        .message {
            padding: 10px;
            background: #eef6ff;
            border: 1px solid #b9daff;
            color: #114a7a;
            border-radius: 5px;
            margin-bottom: 15px;
        }
        .error {
            background: #ffecec;
            border-color: #ffbcbc;
            color: #a10000;
        }
        table {
            width: 100%;
            border-collapse: collapse;
            margin-top: 15px;
        }
        table, th, td {
            border: 1px solid #ccc;
        }
        th, td {
            padding: 10px;
            text-align: left;
        }
        .announcement {
            background: #fafafa;
            border-left: 5px solid #0078d7;
            padding: 12px;
            margin-bottom: 15px;
            border-radius: 5px;
        }
        .small {
            color: #666;
            font-size: 13px;
        }
    </style>
</head>
<body>
<div class="container">

    <?php if (!empty($message)): ?>
        <div class="message"><?php echo h($message); ?></div>
    <?php endif; ?>

    <?php if (isset($_SESSION['user_id'])): ?>
        <div class="nav">
            <a class="btn" href="index.php?page=home">Home</a>

            <?php if (isAdmin()): ?>
                <a class="btn" href="index.php?page=admin">Admin Page</a>
            <?php else: ?>
                <a class="btn" href="index.php?page=employee">Employee Page</a>
            <?php endif; ?>

            <a class="btn" href="index.php?action=logout">Logout</a>
        </div>
    <?php endif; ?>

    <?php
    /* -----------------------------
       Signup Page
    ------------------------------ */
    if ($page === 'signup'):
    ?>
        <h1>Signup</h1>
        <form method="post" action="index.php?page=signup">
            <label>Username</label>
            <input type="text" name="username" required>

            <label>Password</label>
            <input type="password" name="password" required>

            <label>Job Title</label>
            <input type="text" name="job_title" placeholder="Example: admin or employee" required>

            <input type="submit" name="signup" value="Create Account">
        </form>

        <p>Already have an account? <a href="index.php?page=login">Login here</a></p>

    <?php
    /* -----------------------------
       Login Page
    ------------------------------ */
    elseif ($page === 'login'):
    ?>
        <h1>Login</h1>
        <form method="post" action="index.php?page=login">
            <label>Username</label>
            <input type="text" name="username" required>

            <label>Password</label>
            <input type="password" name="password" required>

            <input type="submit" name="login" value="Login">
        </form>

        <p>Need an account? <a href="index.php?page=signup">Sign up here</a></p>

    <?php
    /* -----------------------------
       Home Page
    ------------------------------ */
    elseif ($page === 'home'):
    ?>
        <h1>Home Page</h1>
        <p>Welcome, <strong><?php echo h($_SESSION['username']); ?></strong>.</p>
        <p>Your job title is: <strong><?php echo h($_SESSION['job_title']); ?></strong></p>

        <?php if (isAdmin()): ?>
            <p>You are logged in as an admin. Go to the admin page to view all registered users.</p>
            <a class="btn" href="index.php?page=admin">Go to Admin Page</a>
        <?php else: ?>
            <p>You are logged in as an employee. Go to the employee page to view company announcements.</p>
            <a class="btn" href="index.php?page=employee">Go to Employee Page</a>
        <?php endif; ?>

    <?php
    /* -----------------------------
       Admin Page
    ------------------------------ */
    elseif ($page === 'admin'):
        if (!isAdmin()):
    ?>
            <h1>Access Denied</h1>
            <p>Only admins can access this page.</p>
    <?php
        else:
            $usersStmt = $pdo->query("SELECT id, username, job_title, created_at FROM users ORDER BY id ASC");
            $users = $usersStmt->fetchAll(PDO::FETCH_ASSOC);
    ?>
            <h1>Admin Page</h1>
            <p>This page shows all registered users.</p>

            <table>
                <tr>
                    <th>ID</th>
                    <th>Username</th>
                    <th>Job Title</th>
                    <th>Created At</th>
                </tr>
                <?php foreach ($users as $u): ?>
                    <tr>
                        <td><?php echo h($u['id']); ?></td>
                        <td><?php echo h($u['username']); ?></td>
                        <td><?php echo h($u['job_title']); ?></td>
                        <td><?php echo h($u['created_at']); ?></td>
                    </tr>
                <?php endforeach; ?>
            </table>
    <?php
        endif;

    /* -----------------------------
       Employee Page
    ------------------------------ */
    elseif ($page === 'employee'):
        $annStmt = $pdo->query("SELECT title, message, created_at FROM announcements ORDER BY created_at DESC");
        $announcements = $annStmt->fetchAll(PDO::FETCH_ASSOC);
    ?>
        <h1>Employee Page</h1>
        <p>Company announcements are shown below.</p>

        <?php if (count($announcements) > 0): ?>
            <?php foreach ($announcements as $ann): ?>
                <div class="announcement">
                    <h2><?php echo h($ann['title']); ?></h2>
                    <p><?php echo nl2br(h($ann['message'])); ?></p>
                    <p class="small">Posted: <?php echo h($ann['created_at']); ?></p>
                </div>
            <?php endforeach; ?>
        <?php else: ?>
            <p>No announcements available.</p>
        <?php endif; ?>

    <?php
    /* -----------------------------
       Default Fallback
    ------------------------------ */
    else:
    ?>
        <h1>Page Not Found</h1>
        <p>The page you requested does not exist.</p>
        <a class="btn" href="index.php?page=login">Go to Login</a>
    <?php endif; ?>

</div>
</body>
</html>