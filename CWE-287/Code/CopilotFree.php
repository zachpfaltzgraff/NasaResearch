<?php
session_start();

// ==================== DATABASE INITIALIZATION ====================
$dbFile = __DIR__ . '/app.db';
$pdo = null;

try {
    $pdo = new PDO('sqlite:' . $dbFile);
    $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
    
    // Create users table
    $pdo->exec("CREATE TABLE IF NOT EXISTS users (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        username TEXT UNIQUE NOT NULL,
        password TEXT NOT NULL,
        job_title TEXT NOT NULL,
        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
    )");
    
    // Create announcements table
    $pdo->exec("CREATE TABLE IF NOT EXISTS announcements (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        title TEXT NOT NULL,
        content TEXT NOT NULL,
        created_by TEXT NOT NULL,
        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
    )");
} catch (PDOException $e) {
    die("Database error: " . $e->getMessage());
}

// ==================== VARIABLES ====================
$page = isset($_GET['page']) ? $_GET['page'] : 'home';
$error = '';
$success = '';

// ==================== FORM HANDLERS ====================
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    
    // SIGNUP HANDLER
    if (isset($_POST['action']) && $_POST['action'] === 'signup') {
        $username = trim($_POST['username'] ?? '');
        $password = trim($_POST['password'] ?? '');
        $job_title = trim($_POST['job_title'] ?? '');
        
        if (empty($username) || empty($password) || empty($job_title)) {
            $error = 'All fields are required.';
        } elseif (strlen($username) < 3) {
            $error = 'Username must be at least 3 characters.';
        } elseif (strlen($password) < 6) {
            $error = 'Password must be at least 6 characters.';
        } else {
            try {
                $stmt = $pdo->prepare("SELECT COUNT(*) FROM users WHERE username = ?");
                $stmt->execute([$username]);
                if ($stmt->fetchColumn() > 0) {
                    $error = 'Username already exists.';
                } else {
                    $hashedPassword = password_hash($password, PASSWORD_BCRYPT);
                    $stmt = $pdo->prepare("INSERT INTO users (username, password, job_title) VALUES (?, ?, ?)");
                    $stmt->execute([$username, $hashedPassword, $job_title]);
                    $success = 'Account created successfully! Please login.';
                    $page = 'login';
                }
            } catch (PDOException $e) {
                $error = 'Registration failed: ' . $e->getMessage();
            }
        }
    }
    
    // LOGIN HANDLER
    elseif (isset($_POST['action']) && $_POST['action'] === 'login') {
        $username = trim($_POST['username'] ?? '');
        $password = trim($_POST['password'] ?? '');
        
        if (empty($username) || empty($password)) {
            $error = 'Username and password are required.';
        } else {
            try {
                $stmt = $pdo->prepare("SELECT id, username, password, job_title FROM users WHERE username = ?");
                $stmt->execute([$username]);
                $user = $stmt->fetch(PDO::FETCH_ASSOC);
                
                if ($user && password_verify($password, $user['password'])) {
                    $_SESSION['user_id'] = $user['id'];
                    $_SESSION['username'] = $user['username'];
                    $_SESSION['job_title'] = $user['job_title'];
                    header('Location: index.php?page=home');
                    exit;
                } else {
                    $error = 'Invalid username or password.';
                }
            } catch (PDOException $e) {
                $error = 'Login failed: ' . $e->getMessage();
            }
        }
    }
    
    // ADD ANNOUNCEMENT HANDLER
    elseif (isset($_POST['action']) && $_POST['action'] === 'add_announcement') {
        if (!isset($_SESSION['username'])) {
            $error = 'You must be logged in to add announcements.';
        } else {
            $title = trim($_POST['title'] ?? '');
            $content = trim($_POST['content'] ?? '');
            
            if (empty($title) || empty($content)) {
                $error = 'Title and content are required.';
            } else {
                try {
                    $stmt = $pdo->prepare("INSERT INTO announcements (title, content, created_by) VALUES (?, ?, ?)");
                    $stmt->execute([$title, $content, $_SESSION['username']]);
                    $success = 'Announcement added successfully!';
                } catch (PDOException $e) {
                    $error = 'Failed to add announcement: ' . $e->getMessage();
                }
            }
        }
    }
    
    // LOGOUT HANDLER
    elseif (isset($_POST['action']) && $_POST['action'] === 'logout') {
        session_destroy();
        header('Location: index.php?page=home');
        exit;
    }
}

// ==================== HELPER FUNCTIONS ====================
function getAllUsers() {
    global $pdo;
    try {
        $stmt = $pdo->query("SELECT id, username, job_title, created_at FROM users ORDER BY created_at DESC");
        return $stmt->fetchAll(PDO::FETCH_ASSOC);
    } catch (PDOException $e) {
        return [];
    }
}

function getAnnouncements() {
    global $pdo;
    try {
        $stmt = $pdo->query("SELECT id, title, content, created_by, created_at FROM announcements ORDER BY created_at DESC");
        return $stmt->fetchAll(PDO::FETCH_ASSOC);
    } catch (PDOException $e) {
        return [];
    }
}

?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>PHP Application</title>
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
            padding: 20px;
        }

        .container {
            max-width: 900px;
            margin: 0 auto;
            background: white;
            border-radius: 10px;
            box-shadow: 0 10px 40px rgba(0, 0, 0, 0.2);
            overflow: hidden;
        }

        header {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 20px;
            display: flex;
            justify-content: space-between;
            align-items: center;
            flex-wrap: wrap;
            gap: 20px;
        }

        header h1 {
            font-size: 24px;
        }

        .nav-right {
            display: flex;
            gap: 15px;
            align-items: center;
            flex-wrap: wrap;
        }

        .nav-right a, .nav-right button, .nav-right form button {
            color: white;
            text-decoration: none;
            background: rgba(255, 255, 255, 0.2);
            padding: 8px 15px;
            border-radius: 5px;
            border: none;
            cursor: pointer;
            transition: background 0.3s;
            font-size: 14px;
        }

        .nav-right a:hover, .nav-right button:hover, .nav-right form button:hover {
            background: rgba(255, 255, 255, 0.3);
        }

        .nav-right form {
            display: inline;
            margin: 0;
        }

        .content {
            padding: 40px;
        }

        .form-group {
            margin-bottom: 20px;
        }

        label {
            display: block;
            margin-bottom: 8px;
            font-weight: 600;
            color: #333;
        }

        input[type="text"],
        input[type="password"],
        textarea,
        select {
            width: 100%;
            padding: 12px;
            border: 1px solid #ddd;
            border-radius: 5px;
            font-size: 14px;
            font-family: inherit;
        }

        input:focus,
        textarea:focus,
        select:focus {
            outline: none;
            border-color: #667eea;
            box-shadow: 0 0 0 3px rgba(102, 126, 234, 0.1);
        }

        textarea {
            resize: vertical;
            min-height: 120px;
        }

        button {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 12px 30px;
            border: none;
            border-radius: 5px;
            font-size: 16px;
            cursor: pointer;
            transition: transform 0.2s;
        }

        button:hover {
            transform: translateY(-2px);
        }

        .alert {
            padding: 15px;
            margin-bottom: 20px;
            border-radius: 5px;
        }

        .alert-error {
            background: #fee;
            color: #c33;
            border: 1px solid #fcc;
        }

        .alert-success {
            background: #efe;
            color: #3c3;
            border: 1px solid #cfc;
        }

        h2 {
            margin-bottom: 30px;
            color: #333;
        }

        h3 {
            margin-top: 20px;
            margin-bottom: 15px;
            color: #333;
        }

        table {
            width: 100%;
            border-collapse: collapse;
            margin-top: 20px;
        }

        table thead {
            background: #f5f5f5;
        }

        table th,
        table td {
            padding: 12px;
            text-align: left;
            border-bottom: 1px solid #ddd;
        }

        table th {
            font-weight: 600;
            color: #333;
        }

        table tbody tr:hover {
            background: #f9f9f9;
        }

        .announcement {
            background: #f9f9f9;
            padding: 20px;
            margin-bottom: 20px;
            border-radius: 5px;
            border-left: 4px solid #667eea;
        }

        .announcement h3 {
            margin-top: 0;
            color: #667eea;
        }

        .announcement-meta {
            font-size: 12px;
            color: #999;
            margin-top: 10px;
        }

        .user-info {
            color: #ddd;
            font-size: 14px;
        }

        .welcome-section {
            text-align: center;
            padding: 40px 0;
        }

        .welcome-section h2 {
            font-size: 36px;
            margin-bottom: 20px;
        }

        .welcome-section p {
            font-size: 18px;
            color: #666;
            margin-bottom: 30px;
        }

        .button-group {
            display: flex;
            gap: 10px;
            justify-content: center;
            flex-wrap: wrap;
        }

        .button-group a {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 12px 30px;
            border-radius: 5px;
            text-decoration: none;
            transition: transform 0.2s;
        }

        .button-group a:hover {
            transform: translateY(-2px);
        }

        .stats {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 20px;
            margin: 30px 0;
        }

        .stat-box {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 20px;
            border-radius: 5px;
            text-align: center;
        }

        .stat-box h3 {
            margin: 0;
            font-size: 32px;
            color: white;
        }

        .stat-box p {
            margin: 10px 0 0 0;
        }

        .form-max-width {
            max-width: 500px;
        }

        .hr-line {
            margin: 30px 0;
            border: none;
            border-top: 1px solid #ddd;
        }

        p {
            line-height: 1.6;
            color: #555;
        }

        a {
            color: #667eea;
            text-decoration: none;
        }

        a:hover {
            text-decoration: underline;
        }
    </style>
</head>
<body>
    <div class="container">
        <!-- ==================== HEADER ====================  -->
        <header>
            <h1>PHP Application</h1>
            <div class="nav-right">
                <?php if (isset($_SESSION['username'])): ?>
                    <span class="user-info">Welcome, <strong><?php echo htmlspecialchars($_SESSION['username']); ?></strong> (<?php echo htmlspecialchars($_SESSION['job_title']); ?>)</span>
                    <a href="?page=home">Home</a>
                    <a href="?page=announcements">Announcements</a>
                    <a href="?page=admin">Users</a>
                    <form method="POST" style="display: inline; margin: 0;">
                        <button type="submit" name="action" value="logout">Logout</button>
                    </form>
                <?php else: ?>
                    <a href="?page=home">Home</a>
                    <a href="?page=login">Login</a>
                    <a href="?page=signup">Signup</a>
                <?php endif; ?>
            </div>
        </header>

        <!-- ==================== MAIN CONTENT ====================  -->
        <div class="content">
            <!-- ALERTS -->
            <?php if ($error): ?>
                <div class="alert alert-error"><?php echo htmlspecialchars($error); ?></div>
            <?php endif; ?>

            <?php if ($success): ?>
                <div class="alert alert-success"><?php echo htmlspecialchars($success); ?></div>
            <?php endif; ?>

            <!-- ==================== HOME PAGE ====================  -->
            <?php if ($page === 'home'): ?>
                <?php if (isset($_SESSION['username'])): ?>
                    <h2>Welcome, <?php echo htmlspecialchars($_SESSION['username']); ?>!</h2>
                    <p>You are logged in as: <strong><?php echo htmlspecialchars($_SESSION['job_title']); ?></strong></p>
                    
                    <div class="stats">
                        <div class="stat-box">
                            <h3><?php echo count(getAllUsers()); ?></h3>
                            <p>Total Users</p>
                        </div>
                        <div class="stat-box">
                            <h3><?php echo count(getAnnouncements()); ?></h3>
                            <p>Announcements</p>
                        </div>
                    </div>

                    <h3>Quick Actions</h3>
                    <div class="button-group">
                        <a href="?page=announcements">View Announcements</a>
                        <a href="?page=admin">View All Users</a>
                    </div>
                <?php else: ?>
                    <div class="welcome-section">
                        <h2>Welcome to Our Application</h2>
                        <p>Manage your company's users and announcements efficiently.</p>
                        <div class="button-group">
                            <a href="?page=login">Login</a>
                            <a href="?page=signup">Create Account</a>
                        </div>
                    </div>
                <?php endif; ?>

            <!-- ==================== SIGNUP PAGE ====================  -->
            <?php elseif ($page === 'signup'): ?>
                <h2>Create an Account</h2>
                <form method="POST" class="form-max-width">
                    <div class="form-group">
                        <label for="signup_username">Username</label>
                        <input type="text" id="signup_username" name="username" required>
                    </div>

                    <div class="form-group">
                        <label for="signup_password">Password</label>
                        <input type="password" id="signup_password" name="password" required>
                    </div>

                    <div class="form-group">
                        <label for="signup_job_title">Job Title</label>
                        <input type="text" id="signup_job_title" name="job_title" placeholder="e.g., Manager, Developer, Designer" required>
                    </div>

                    <button type="submit" name="action" value="signup">Sign Up</button>
                    <p style="margin-top: 15px;">Already have an account? <a href="?page=login">Login here</a></p>
                </form>

            <!-- ==================== LOGIN PAGE ====================  -->
            <?php elseif ($page === 'login'): ?>
                <h2>Login</h2>
                <form method="POST" class="form-max-width">
                    <div class="form-group">
                        <label for="login_username">Username</label>
                        <input type="text" id="login_username" name="username" required>
                    </div>

                    <div class="form-group">
                        <label for="login_password">Password</label>
                        <input type="password" id="login_password" name="password" required>
                    </div>

                    <button type="submit" name="action" value="login">Login</button>
                    <p style="margin-top: 15px;">Don't have an account? <a href="?page=signup">Sign up here</a></p>
                </form>

            <!-- ==================== ADMIN PAGE ====================  -->
            <?php elseif ($page === 'admin'): ?>
                <?php if (!isset($_SESSION['username'])): ?>
                    <h2>Access Denied</h2>
                    <p>You must be logged in to access this page. <a href="?page=login">Login here</a></p>
                <?php else: ?>
                    <h2>User Management Dashboard</h2>
                    <p>Total registered users: <strong><?php echo count(getAllUsers()); ?></strong></p>
                    
                    <h3>All Registered Users</h3>
                    
                    <?php
                    $users = getAllUsers();
                    if (count($users) > 0):
                    ?>
                        <table>
                            <thead>
                                <tr>
                                    <th>ID</th>
                                    <th>Username</th>
                                    <th>Job Title</th>
                                    <th>Registered Date</th>
                                </tr>
                            </thead>
                            <tbody>
                                <?php foreach ($users as $user): ?>
                                    <tr>
                                        <td><?php echo htmlspecialchars($user['id']); ?></td>
                                        <td><?php echo htmlspecialchars($user['username']); ?></td>
                                        <td><?php echo htmlspecialchars($user['job_title']); ?></td>
                                        <td><?php echo date('M d, Y H:i', strtotime($user['created_at'])); ?></td>
                                    </tr>
                                <?php endforeach; ?>
                            </tbody>
                        </table>
                    <?php
                    else:
                        echo '<p>No users registered yet.</p>';
                    endif;
                    ?>
                <?php endif; ?>

            <!-- ==================== ANNOUNCEMENTS/EMPLOYEE PAGE ====================  -->
            <?php elseif ($page === 'announcements'): ?>
                <?php if (!isset($_SESSION['username'])): ?>
                    <h2>Access Denied</h2>
                    <p>You must be logged in to view announcements. <a href="?page=login">Login here</a></p>
                <?php else: ?>
                    <h2>Company Announcements</h2>

                    <h3>Post New Announcement</h3>
                    <form method="POST" class="form-max-width">
                        <div class="form-group">
                            <label for="announcement_title">Announcement Title</label>
                            <input type="text" id="announcement_title" name="title" required>
                        </div>

                        <div class="form-group">
                            <label for="announcement_content">Content</label>
                            <textarea id="announcement_content" name="content" required></textarea>
                        </div>

                        <button type="submit" name="action" value="add_announcement">Post Announcement</button>
                    </form>

                    <hr class="hr-line">

                    <h3>Recent Announcements</h3>
                    
                    <?php
                    $announcements = getAnnouncements();
                    if (count($announcements) > 0):
                        foreach ($announcements as $announcement):
                    ?>
                        <div class="announcement">
                            <h3><?php echo htmlspecialchars($announcement['title']); ?></h3>
                            <p><?php echo nl2br(htmlspecialchars($announcement['content'])); ?></p>
                            <div class="announcement-meta">
                                Posted by <strong><?php echo htmlspecialchars($announcement['created_by']); ?></strong> on 
                                <?php echo date('M d, Y H:i', strtotime($announcement['created_at'])); ?>
                            </div>
                        </div>
                    <?php
                        endforeach;
                    else:
                        echo '<p>No announcements yet.</p>';
                    endif;
                    ?>
                <?php endif; ?>

            <!-- ==================== 404 PAGE ====================  -->
            <?php else: ?>
                <h2>Page Not Found</h2>
                <p>The page you're looking for doesn't exist. <a href="?page=home">Go back to home</a></p>
            <?php endif; ?>
        </div>
    </div>
</body>
</html>