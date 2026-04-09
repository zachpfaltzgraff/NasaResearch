<?php
session_start();

/*
    SINGLE-FILE PHP EMPLOYEE PORTAL
    --------------------------------
    Features:
    - User login
    - Employee search by employee ID
    - Manager department salary search
    - Role-based access (employee / manager)
    - Dynamic page switching in one file
    - MySQL via PDO
    - Prepared statements for safety
*/

/* =========================
   DATABASE CONFIG
   ========================= */
$dbHost = "localhost";
$dbName = "company_portal";
$dbUser = "root";
$dbPass = "";

/* =========================
   CONNECT TO DATABASE
   ========================= */
try {
    $pdo = new PDO(
        "mysql:host={$dbHost};dbname={$dbName};charset=utf8mb4",
        $dbUser,
        $dbPass,
        [
            PDO::ATTR_ERRMODE => PDO::ERRMODE_EXCEPTION,
            PDO::ATTR_DEFAULT_FETCH_MODE => PDO::FETCH_ASSOC
        ]
    );
} catch (PDOException $e) {
    die("Database connection failed: " . htmlspecialchars($e->getMessage()));
}

/* =========================
   HELPER FUNCTIONS
   ========================= */
function h($value) {
    return htmlspecialchars((string)$value, ENT_QUOTES, 'UTF-8');
}

function isLoggedIn() {
    return isset($_SESSION['user']);
}

function isManager() {
    return isLoggedIn() && isset($_SESSION['user']['role']) && $_SESSION['user']['role'] === 'manager';
}

function maskSSN($ssn) {
    $digits = preg_replace('/\D/', '', $ssn);
    if (strlen($digits) === 9) {
        return "***-**-" . substr($digits, -4);
    }
    return "Hidden";
}

function showMessage($msg, $type = "success") {
    $bg = ($type === "error") ? "#f8d7da" : "#d1e7dd";
    $color = ($type === "error") ? "#842029" : "#0f5132";
    echo "<div style='padding:10px;margin:10px 0;border-radius:6px;background:$bg;color:$color;'>"
        . h($msg) .
        "</div>";
}

/* =========================
   OPTIONAL TABLE CREATION
   Run once if you want quick setup
   ========================= */
if (isset($_GET['setup']) && $_GET['setup'] === '1') {
    try {
        $pdo->exec("
            CREATE TABLE IF NOT EXISTS users (
                id INT AUTO_INCREMENT PRIMARY KEY,
                username VARCHAR(50) NOT NULL UNIQUE,
                password VARCHAR(255) NOT NULL,
                role ENUM('employee', 'manager') NOT NULL DEFAULT 'employee'
            )
        ");

        $pdo->exec("
            CREATE TABLE IF NOT EXISTS employees (
                id INT AUTO_INCREMENT PRIMARY KEY,
                employee_id VARCHAR(20) NOT NULL UNIQUE,
                name VARCHAR(100) NOT NULL,
                salary DECIMAL(10,2) NOT NULL,
                department VARCHAR(100) NOT NULL,
                ssn VARCHAR(20) NOT NULL
            )
        ");

        // Insert default users if not present
        $checkStmt = $pdo->prepare("SELECT COUNT(*) AS total FROM users");
        $checkStmt->execute();
        $userCount = $checkStmt->fetch()['total'];

        if ($userCount == 0) {
            $insertUser = $pdo->prepare("INSERT INTO users (username, password, role) VALUES (?, ?, ?)");

            $insertUser->execute([
                "employee1",
                password_hash("employee123", PASSWORD_DEFAULT),
                "employee"
            ]);

            $insertUser->execute([
                "manager1",
                password_hash("manager123", PASSWORD_DEFAULT),
                "manager"
            ]);
        }

        // Insert sample employees if not present
        $checkEmpStmt = $pdo->prepare("SELECT COUNT(*) AS total FROM employees");
        $checkEmpStmt->execute();
        $empCount = $checkEmpStmt->fetch()['total'];

        if ($empCount == 0) {
            $insertEmp = $pdo->prepare("
                INSERT INTO employees (employee_id, name, salary, department, ssn)
                VALUES (?, ?, ?, ?, ?)
            ");

            $insertEmp->execute(["1001", "John Smith", 55000.00, "IT", "123-45-6789"]);
            $insertEmp->execute(["1002", "Sarah Johnson", 62000.00, "HR", "234-56-7890"]);
            $insertEmp->execute(["1003", "Michael Brown", 71000.00, "IT", "345-67-8901"]);
            $insertEmp->execute(["1004", "Emily Davis", 48000.00, "Sales", "456-78-9012"]);
            $insertEmp->execute(["1005", "David Wilson", 80000.00, "Finance", "567-89-0123"]);
        }

        echo "<h2>Setup Complete</h2>";
        echo "<p>Default accounts created:</p>";
        echo "<ul>
                <li>Employee login: employee1 / employee123</li>
                <li>Manager login: manager1 / manager123</li>
              </ul>";
        echo "<p><a href='index.php'>Go to Login</a></p>";
        exit;
    } catch (PDOException $e) {
        die("Setup failed: " . h($e->getMessage()));
    }
}

/* =========================
   HANDLE LOGOUT
   ========================= */
if (isset($_GET['action']) && $_GET['action'] === 'logout') {
    session_destroy();
    header("Location: index.php");
    exit;
}

/* =========================
   HANDLE LOGIN
   ========================= */
$loginError = "";

if (isset($_POST['action']) && $_POST['action'] === 'login') {
    $username = trim($_POST['username'] ?? '');
    $password = $_POST['password'] ?? '';

    if ($username === '' || $password === '') {
        $loginError = "Please enter both username and password.";
    } else {
        $stmt = $pdo->prepare("SELECT id, username, password, role FROM users WHERE username = ?");
        $stmt->execute([$username]);
        $user = $stmt->fetch();

        if ($user && password_verify($password, $user['password'])) {
            $_SESSION['user'] = [
                'id' => $user['id'],
                'username' => $user['username'],
                'role' => $user['role']
            ];
            header("Location: index.php?page=dashboard");
            exit;
        } else {
            $loginError = "Invalid username or password.";
        }
    }
}

/* =========================
   PAGE ROUTING
   ========================= */
$page = $_GET['page'] ?? 'login';
if (!isLoggedIn()) {
    $page = 'login';
}
?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Employee Portal</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background: #f4f6f9;
            margin: 0;
            padding: 0;
        }
        .container {
            width: 900px;
            max-width: 95%;
            margin: 30px auto;
            background: white;
            padding: 25px;
            border-radius: 10px;
            box-shadow: 0 0 10px rgba(0,0,0,0.08);
        }
        h1, h2, h3 {
            margin-top: 0;
        }
        nav {
            background: #1f4e79;
            padding: 12px 20px;
            color: white;
        }
        nav a {
            color: white;
            text-decoration: none;
            margin-right: 15px;
            font-weight: bold;
        }
        input, select, button {
            padding: 10px;
            margin: 6px 0;
            width: 100%;
            box-sizing: border-box;
        }
        button {
            background: #1f4e79;
            color: white;
            border: none;
            cursor: pointer;
            border-radius: 5px;
        }
        button:hover {
            background: #163a59;
        }
        table {
            width: 100%;
            border-collapse: collapse;
            margin-top: 15px;
        }
        table th, table td {
            border: 1px solid #ccc;
            padding: 10px;
            text-align: left;
        }
        table th {
            background: #e9eef5;
        }
        .card {
            background: #fafafa;
            border: 1px solid #ddd;
            padding: 15px;
            border-radius: 8px;
            margin-top: 15px;
        }
        .small {
            color: #666;
            font-size: 14px;
        }
    </style>
</head>
<body>

<?php if (isLoggedIn()): ?>
<nav>
    <a href="index.php?page=dashboard">Dashboard</a>
    <a href="index.php?page=employee_search">Employee Search</a>
    <?php if (isManager()): ?>
        <a href="index.php?page=department_search">Department Search</a>
    <?php endif; ?>
    <span style="float:right;">
        Logged in as <?= h($_SESSION['user']['username']) ?> (<?= h($_SESSION['user']['role']) ?>)
        | <a href="index.php?action=logout">Logout</a>
    </span>
</nav>
<?php endif; ?>

<div class="container">

<?php
/* =========================
   LOGIN PAGE
   ========================= */
if ($page === 'login'):
?>
    <h1>Employee Portal Login</h1>
    <p class="small">Use <code>?setup=1</code> once to create sample tables and demo accounts.</p>

    <?php
    if ($loginError !== "") {
        showMessage($loginError, "error");
    }
    ?>

    <form method="post" action="index.php">
        <input type="hidden" name="action" value="login">

        <label>Username</label>
        <input type="text" name="username" required>

        <label>Password</label>
        <input type="password" name="password" required>

        <button type="submit">Login</button>
    </form>

<?php
/* =========================
   DASHBOARD
   ========================= */
elseif ($page === 'dashboard'):
?>
    <h1>Dashboard</h1>
    <div class="card">
        <h3>Welcome, <?= h($_SESSION['user']['username']) ?></h3>
        <p>Your role is: <strong><?= h($_SESSION['user']['role']) ?></strong></p>
        <p>You can:</p>
        <ul>
            <li>Search employee information by ID number</li>
            <?php if (isManager()): ?>
                <li>Search a department and view all employee salaries in that department</li>
            <?php endif; ?>
        </ul>
    </div>

<?php
/* =========================
   EMPLOYEE SEARCH PAGE
   ========================= */
elseif ($page === 'employee_search'):
    $employeeResult = null;
    $employeeError = "";

    if (isset($_POST['action']) && $_POST['action'] === 'employee_search') {
        $employeeId = trim($_POST['employee_id'] ?? '');

        if ($employeeId === '') {
            $employeeError = "Please enter an employee ID.";
        } else {
            $stmt = $pdo->prepare("
                SELECT employee_id, name, salary, department, ssn
                FROM employees
                WHERE employee_id = ?
            ");
            $stmt->execute([$employeeId]);
            $employeeResult = $stmt->fetch();

            if (!$employeeResult) {
                $employeeError = "No employee found with that ID.";
            }
        }
    }
?>
    <h1>Search Employee by ID</h1>

    <form method="post" action="index.php?page=employee_search">
        <input type="hidden" name="action" value="employee_search">

        <label>Employee ID Number</label>
        <input type="text" name="employee_id" placeholder="Enter employee ID" required>

        <button type="submit">Search</button>
    </form>

    <?php
    if ($employeeError !== "") {
        showMessage($employeeError, "error");
    }

    if ($employeeResult):
    ?>
        <div class="card">
            <h3>Employee Information</h3>
            <p><strong>Name:</strong> <?= h($employeeResult['name']) ?></p>
            <p><strong>ID Number:</strong> <?= h($employeeResult['employee_id']) ?></p>
            <p><strong>Department:</strong> <?= h($employeeResult['department']) ?></p>
            <p><strong>Salary:</strong> $<?= number_format((float)$employeeResult['salary'], 2) ?></p>
            <p><strong>SSN:</strong> <?= h(maskSSN($employeeResult['ssn'])) ?></p>
        </div>
    <?php endif; ?>

<?php
/* =========================
   DEPARTMENT SEARCH PAGE
   ========================= */
elseif ($page === 'department_search' && isManager()):
    $departmentResults = [];
    $departmentError = "";
    $departmentName = "";

    if (isset($_POST['action']) && $_POST['action'] === 'department_search') {
        $departmentName = trim($_POST['department'] ?? '');

        if ($departmentName === '') {
            $departmentError = "Please enter a department.";
        } else {
            $stmt = $pdo->prepare("
                SELECT employee_id, name, salary, department
                FROM employees
                WHERE department = ?
                ORDER BY name ASC
            ");
            $stmt->execute([$departmentName]);
            $departmentResults = $stmt->fetchAll();

            if (!$departmentResults) {
                $departmentError = "No employees found in that department.";
            }
        }
    }
?>
    <h1>Manager Department Search</h1>

    <form method="post" action="index.php?page=department_search">
        <input type="hidden" name="action" value="department_search">

        <label>Department Name</label>
        <input type="text" name="department" placeholder="Enter department name" required>

        <button type="submit">Search Department</button>
    </form>

    <?php
    if ($departmentError !== "") {
        showMessage($departmentError, "error");
    }

    if (!empty($departmentResults)):
    ?>
        <h3>Department Results: <?= h($departmentName) ?></h3>
        <table>
            <tr>
                <th>Employee ID</th>
                <th>Name</th>
                <th>Department</th>
                <th>Salary</th>
            </tr>
            <?php foreach ($departmentResults as $row): ?>
                <tr>
                    <td><?= h($row['employee_id']) ?></td>
                    <td><?= h($row['name']) ?></td>
                    <td><?= h($row['department']) ?></td>
                    <td>$<?= number_format((float)$row['salary'], 2) ?></td>
                </tr>
            <?php endforeach; ?>
        </table>
    <?php endif; ?>

<?php
/* =========================
   FALLBACK
   ========================= */
else:
?>
    <h1>Access Denied</h1>
    <p>You do not have permission to view this page.</p>
<?php endif; ?>

</div>
</body>
</html>