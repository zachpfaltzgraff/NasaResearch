<?php
session_start();

/*
    ================================
    BASIC EMPLOYEE MANAGEMENT SYSTEM
    Single-file PHP application
    ================================

    Before running:
    1. Create a MySQL database named: employee_management
    2. Create the users table using the SQL shown below
    3. Update the DB credentials here if needed
*/

$DB_HOST = "localhost";
$DB_USER = "root";
$DB_PASS = "";
$DB_NAME = "employee_management";

$conn = new mysqli($DB_HOST, $DB_USER, $DB_PASS, $DB_NAME);

if ($conn->connect_error) {
    die("Database connection failed: " . htmlspecialchars($conn->connect_error));
}

function e($value) {
    return htmlspecialchars((string)$value, ENT_QUOTES, 'UTF-8');
}

function is_logged_in() {
    return isset($_SESSION['user_id']);
}

function is_manager() {
    return isset($_SESSION['role']) && $_SESSION['role'] === 'manager';
}

function redirect_to($page) {
    header("Location: ?page=" . urlencode($page));
    exit;
}

function get_current_user_data($conn, $userId) {
    $stmt = $conn->prepare("SELECT id, username, full_name, employee_id, position, role FROM users WHERE id = ?");
    if (!$stmt) {
        return null;
    }
    $stmt->bind_param("i", $userId);
    $stmt->execute();
    $result = $stmt->get_result();
    $user = $result->fetch_assoc();
    $stmt->close();
    return $user;
}

$message = "";
$error = "";

/*
    ========================
    HANDLE LOGOUT
    ========================
*/
if (isset($_GET['action']) && $_GET['action'] === 'logout') {
    session_unset();
    session_destroy();
    session_start();
    $message = "You have been logged out.";
}

/*
    ========================
    HANDLE LOGIN
    ========================
*/
if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['action']) && $_POST['action'] === 'login') {
    $username = trim($_POST['username'] ?? '');
    $password = $_POST['password'] ?? '';

    if ($username === '' || $password === '') {
        $error = "Please enter both username and password.";
    } else {
        $stmt = $conn->prepare("SELECT id, username, password, full_name, employee_id, position, role FROM users WHERE username = ?");
        if (!$stmt) {
            $error = "A database error occurred.";
        } else {
            $stmt->bind_param("s", $username);
            $stmt->execute();
            $result = $stmt->get_result();
            $user = $result->fetch_assoc();
            $stmt->close();

            if ($user && password_verify($password, $user['password'])) {
                $_SESSION['user_id'] = $user['id'];
                $_SESSION['username'] = $user['username'];
                $_SESSION['role'] = $user['role'];
                $_SESSION['full_name'] = $user['full_name'];

                redirect_to('dashboard');
            } else {
                $error = "Invalid username or password.";
            }
        }
    }
}

/*
    ========================
    HANDLE ROLE UPDATE
    ========================
*/
if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['action']) && $_POST['action'] === 'update_role') {
    if (!is_logged_in() || !is_manager()) {
        $error = "Access denied.";
    } else {
        $targetId = (int)($_POST['target_id'] ?? 0);
        $newRole = $_POST['new_role'] ?? '';

        if ($targetId <= 0 || !in_array($newRole, ['employee', 'manager'], true)) {
            $error = "Invalid update request.";
        } else {
            $stmt = $conn->prepare("UPDATE users SET role = ? WHERE id = ?");
            if (!$stmt) {
                $error = "Database error while updating role.";
            } else {
                $stmt->bind_param("si", $newRole, $targetId);
                if ($stmt->execute()) {
                    $message = "Role updated successfully.";
                    if (isset($_SESSION['user_id']) && $_SESSION['user_id'] === $targetId) {
                        $_SESSION['role'] = $newRole;
                    }
                } else {
                    $error = "Failed to update role.";
                }
                $stmt->close();
            }
        }
    }
}

/*
    ========================
    PAGE ROUTING
    ========================
*/
$page = $_GET['page'] ?? 'home';

if (!is_logged_in() && in_array($page, ['dashboard', 'profile', 'manage'], true)) {
    redirect_to('login');
}

if ($page === 'manage' && !is_manager()) {
    redirect_to('dashboard');
}
?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Employee Management System</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background: #f4f7fb;
            margin: 0;
            padding: 0;
            color: #222;
        }

        header {
            background: #1f4e79;
            color: white;
            padding: 18px 24px;
        }

        header h1 {
            margin: 0;
            font-size: 24px;
        }

        nav {
            background: #163a59;
            padding: 10px 24px;
        }

        nav a {
            color: white;
            text-decoration: none;
            margin-right: 16px;
            font-weight: bold;
        }

        nav a:hover {
            text-decoration: underline;
        }

        .container {
            max-width: 1000px;
            margin: 24px auto;
            background: white;
            padding: 24px;
            border-radius: 10px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.08);
        }

        h2 {
            margin-top: 0;
            color: #1f4e79;
        }

        form {
            margin-top: 16px;
        }

        label {
            display: block;
            margin-top: 12px;
            margin-bottom: 6px;
            font-weight: bold;
        }

        input[type="text"],
        input[type="password"],
        select {
            width: 100%;
            max-width: 400px;
            padding: 10px;
            border: 1px solid #bbb;
            border-radius: 6px;
            font-size: 14px;
        }

        button {
            margin-top: 16px;
            padding: 10px 18px;
            border: none;
            background: #1f4e79;
            color: white;
            border-radius: 6px;
            cursor: pointer;
            font-size: 14px;
        }

        button:hover {
            background: #163a59;
        }

        .message {
            padding: 12px;
            border-radius: 6px;
            margin-bottom: 16px;
            background: #e6f4ea;
            color: #1d5e2a;
            border: 1px solid #b7dfc2;
        }

        .error {
            padding: 12px;
            border-radius: 6px;
            margin-bottom: 16px;
            background: #fdeaea;
            color: #8a1f1f;
            border: 1px solid #f0b4b4;
        }

        table {
            width: 100%;
            border-collapse: collapse;
            margin-top: 18px;
        }

        table th, table td {
            border: 1px solid #ddd;
            padding: 10px;
            text-align: left;
            vertical-align: top;
        }

        table th {
            background: #f0f4f8;
        }

        .small-form {
            margin: 0;
        }

        .info-box {
            background: #f8fbff;
            border: 1px solid #d3e3f1;
            padding: 16px;
            border-radius: 8px;
            max-width: 600px;
        }

        .info-row {
            margin-bottom: 10px;
        }

        .info-label {
            font-weight: bold;
            display: inline-block;
            width: 120px;
        }
    </style>
</head>
<body>

<header>
    <h1>Employee Management System</h1>
</header>

<nav>
    <a href="?page=home">Home</a>

    <?php if (!is_logged_in()): ?>
        <a href="?page=login">Login</a>
    <?php else: ?>
        <a href="?page=dashboard">Dashboard</a>
        <a href="?page=profile">My Profile</a>
        <?php if (is_manager()): ?>
            <a href="?page=manage">Manage Employees</a>
        <?php endif; ?>
        <a href="?action=logout">Logout</a>
    <?php endif; ?>
</nav>

<div class="container">
    <?php if ($message !== ''): ?>
        <div class="message"><?php echo e($message); ?></div>
    <?php endif; ?>

    <?php if ($error !== ''): ?>
        <div class="error"><?php echo e($error); ?></div>
    <?php endif; ?>

    <?php
    if ($page === 'home'):
    ?>
        <h2>Welcome</h2>
        <p>This is a basic employee management system built in a single PHP file.</p>
        <?php if (!is_logged_in()): ?>
            <p>Please log in to continue.</p>
        <?php else: ?>
            <p>You are logged in as <strong><?php echo e($_SESSION['username']); ?></strong> with role <strong><?php echo e($_SESSION['role']); ?></strong>.</p>
        <?php endif; ?>

    <?php
    elseif ($page === 'login'):
        if (is_logged_in()) {
            redirect_to('dashboard');
        }
    ?>
        <h2>Login</h2>
        <form method="post" action="">
            <input type="hidden" name="action" value="login">

            <label for="username">Username</label>
            <input type="text" name="username" id="username" required>

            <label for="password">Password</label>
            <input type="password" name="password" id="password" required>

            <button type="submit">Log In</button>
        </form>

    <?php
    elseif ($page === 'dashboard'):
        $currentUser = get_current_user_data($conn, $_SESSION['user_id']);
    ?>
        <h2>Dashboard</h2>
        <p>Welcome, <strong><?php echo e($currentUser['full_name'] ?? $_SESSION['full_name']); ?></strong>.</p>
        <p>Your role is <strong><?php echo e($_SESSION['role']); ?></strong>.</p>

        <?php if (is_manager()): ?>
            <p>As a manager, you can view all employee profiles and update roles.</p>
        <?php else: ?>
            <p>As an employee, you can view your own profile information.</p>
        <?php endif; ?>

    <?php
    elseif ($page === 'profile'):
        $currentUser = get_current_user_data($conn, $_SESSION['user_id']);
        if (!$currentUser):
    ?>
        <div class="error">Unable to load profile.</div>
    <?php else: ?>
        <h2>My Profile</h2>
        <div class="info-box">
            <div class="info-row">
                <span class="info-label">Name:</span>
                <?php echo e($currentUser['full_name']); ?>
            </div>
            <div class="info-row">
                <span class="info-label">Employee ID:</span>
                <?php echo e($currentUser['employee_id']); ?>
            </div>
            <div class="info-row">
                <span class="info-label">Position:</span>
                <?php echo e($currentUser['position']); ?>
            </div>
            <div class="info-row">
                <span class="info-label">Username:</span>
                <?php echo e($currentUser['username']); ?>
            </div>
            <div class="info-row">
                <span class="info-label">Role:</span>
                <?php echo e($currentUser['role']); ?>
            </div>
        </div>
    <?php endif; ?>

    <?php
    elseif ($page === 'manage'):
        $result = $conn->query("SELECT id, username, full_name, employee_id, position, role FROM users ORDER BY full_name ASC");
    ?>
        <h2>Manage Employees</h2>
        <p>Managers can view all employee profiles and update roles.</p>

        <table>
            <tr>
                <th>Name</th>
                <th>Employee ID</th>
                <th>Position</th>
                <th>Username</th>
                <th>Current Role</th>
                <th>Update Role</th>
            </tr>

            <?php if ($result && $result->num_rows > 0): ?>
                <?php while ($row = $result->fetch_assoc()): ?>
                    <tr>
                        <td><?php echo e($row['full_name']); ?></td>
                        <td><?php echo e($row['employee_id']); ?></td>
                        <td><?php echo e($row['position']); ?></td>
                        <td><?php echo e($row['username']); ?></td>
                        <td><?php echo e($row['role']); ?></td>
                        <td>
                            <form method="post" action="?page=manage" class="small-form">
                                <input type="hidden" name="action" value="update_role">
                                <input type="hidden" name="target_id" value="<?php echo (int)$row['id']; ?>">

                                <select name="new_role" required>
                                    <option value="employee" <?php echo ($row['role'] === 'employee') ? 'selected' : ''; ?>>employee</option>
                                    <option value="manager" <?php echo ($row['role'] === 'manager') ? 'selected' : ''; ?>>manager</option>
                                </select>

                                <button type="submit">Save</button>
                            </form>
                        </td>
                    </tr>
                <?php endwhile; ?>
            <?php else: ?>
                <tr>
                    <td colspan="6">No employees found.</td>
                </tr>
            <?php endif; ?>
        </table>

    <?php
    else:
    ?>
        <h2>Page Not Found</h2>
        <p>The page you requested does not exist.</p>
    <?php endif; ?>
</div>

</body>
</html>
<?php
$conn->close();
?>