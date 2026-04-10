<?php
session_start();

/* ===== DATABASE CONNECTION ===== */
$host = "localhost";
$dbname = "employee_db";
$user = "root";
$pass = "";

try {
    $pdo = new PDO("mysql:host=$host;dbname=$dbname", $user, $pass);
    $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
} catch(PDOException $e) {
    die("DB Connection failed: " . $e->getMessage());
}

/* ===== ROUTING ===== */
$page = $_GET['page'] ?? 'login';

/* ===== LOGIN HANDLER ===== */
if (isset($_POST['login'])) {
    $username = $_POST['username'];
    $password = md5($_POST['password']);

    $stmt = $pdo->prepare("SELECT * FROM users WHERE username=? AND password=?");
    $stmt->execute([$username, $password]);
    $userData = $stmt->fetch();

    if ($userData) {
        $_SESSION['user'] = $userData;
        header("Location: ?page=dashboard");
        exit;
    } else {
        $error = "Invalid credentials";
    }
}

/* ===== LOGOUT ===== */
if ($page == 'logout') {
    session_destroy();
    header("Location: ?");
    exit;
}

/* ===== ROLE UPDATE (Manager only) ===== */
if (isset($_POST['update_role']) && $_SESSION['user']['role'] == 'manager') {
    $userId = $_POST['user_id'];
    $newRole = $_POST['role'];

    $stmt = $pdo->prepare("UPDATE users SET role=? WHERE id=?");
    $stmt->execute([$newRole, $userId]);
}

/* ===== HTML START ===== */
?>
<!DOCTYPE html>
<html>
<head>
    <title>Employee Management System</title>
    <style>
        body { font-family: Arial; margin: 40px; }
        nav a { margin-right: 15px; }
        table { border-collapse: collapse; width: 70%; }
        table, th, td { border: 1px solid #ccc; padding: 8px; }
    </style>
</head>
<body>

<h2>Employee Management System</h2>

<?php if (isset($_SESSION['user'])): ?>
    <nav>
        <a href="?page=dashboard">Dashboard</a>
        <?php if ($_SESSION['user']['role'] == 'manager'): ?>
            <a href="?page=manage">Manage Employees</a>
        <?php endif; ?>
        <a href="?page=logout">Logout</a>
    </nav>
    <hr>
<?php endif; ?>

<?php
/* ===== LOGIN PAGE ===== */
if ($page == 'login' && !isset($_SESSION['user'])):
?>

    <h3>Login</h3>
    <?php if (isset($error)) echo "<p style='color:red;'>$error</p>"; ?>
    <form method="POST">
        Username: <input type="text" name="username" required><br><br>
        Password: <input type="password" name="password" required><br><br>
        <button type="submit" name="login">Login</button>
    </form>

<?php
/* ===== DASHBOARD ===== */
elseif ($page == 'dashboard' && isset($_SESSION['user'])):
    $user = $_SESSION['user'];
?>

    <h3>Welcome, <?= htmlspecialchars($user['name']) ?></h3>
    <p><strong>Role:</strong> <?= $user['role'] ?></p>

    <h4>Your Profile</h4>
    <ul>
        <li><strong>ID:</strong> <?= $user['id'] ?></li>
        <li><strong>Name:</strong> <?= htmlspecialchars($user['name']) ?></li>
        <li><strong>Position:</strong> <?= htmlspecialchars($user['position']) ?></li>
    </ul>

<?php
/* ===== MANAGER PAGE ===== */
elseif ($page == 'manage' && isset($_SESSION['user']) && $_SESSION['user']['role'] == 'manager'):

    $stmt = $pdo->query("SELECT * FROM users");
    $users = $stmt->fetchAll();
?>

    <h3>Manage Employees</h3>

    <table>
        <tr>
            <th>ID</th><th>Name</th><th>Username</th><th>Position</th><th>Role</th><th>Update</th>
        </tr>

        <?php foreach ($users as $u): ?>
        <tr>
            <td><?= $u['id'] ?></td>
            <td><?= htmlspecialchars($u['name']) ?></td>
            <td><?= htmlspecialchars($u['username']) ?></td>
            <td><?= htmlspecialchars($u['position']) ?></td>
            <td><?= $u['role'] ?></td>
            <td>
                <form method="POST" style="display:inline;">
                    <input type="hidden" name="user_id" value="<?= $u['id'] ?>">
                    <select name="role">
                        <option value="employee" <?= $u['role']=='employee'?'selected':'' ?>>Employee</option>
                        <option value="manager" <?= $u['role']=='manager'?'selected':'' ?>>Manager</option>
                    </select>
                    <button name="update_role">Update</button>
                </form>
            </td>
        </tr>
        <?php endforeach; ?>
    </table>

<?php
/* ===== ACCESS DENIED ===== */
else:
    echo "<p>Access denied. Please <a href='?'>login</a>.</p>";
endif;
?>

</body>
</html>