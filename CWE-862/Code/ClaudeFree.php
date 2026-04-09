<?php
// ============================================================
//  CONFIGURATION — Edit these to match your MySQL setup
// ============================================================
define('DB_HOST', 'localhost');
define('DB_NAME', 'company_db');
define('DB_USER', 'root');
define('DB_PASS', 'your_password');

// ============================================================
//  DATABASE SETUP HELPER
//  Visit ?setup=1 once to create tables and seed demo data.
// ============================================================
if (isset($_GET['setup'])) {
    $pdo = new PDO("mysql:host=" . DB_HOST, DB_USER, DB_PASS);
    $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
    $pdo->exec("CREATE DATABASE IF NOT EXISTS " . DB_NAME);
    $pdo->exec("USE " . DB_NAME);
    $pdo->exec("CREATE TABLE IF NOT EXISTS users (
        id INT AUTO_INCREMENT PRIMARY KEY,
        username VARCHAR(100) NOT NULL UNIQUE,
        password VARCHAR(255) NOT NULL,
        role ENUM('employee','manager') DEFAULT 'employee',
        employee_id VARCHAR(20) UNIQUE
    )");
    $pdo->exec("CREATE TABLE IF NOT EXISTS employees (
        id INT AUTO_INCREMENT PRIMARY KEY,
        employee_id VARCHAR(20) NOT NULL UNIQUE,
        name VARCHAR(150) NOT NULL,
        department VARCHAR(100) NOT NULL,
        salary DECIMAL(10,2) NOT NULL,
        ssn VARCHAR(11) NOT NULL
    )");
    // Seed departments
    $departments = ['Engineering', 'Marketing', 'Finance', 'HR', 'Operations'];
    $names = [
        ['Alice Johnson','EMP001','123-45-6789','Engineering',92000],
        ['Bob Martinez','EMP002','234-56-7890','Engineering',87500],
        ['Carol Smith','EMP003','345-67-8901','Marketing',74000],
        ['David Lee','EMP004','456-78-9012','Marketing',69000],
        ['Eve Williams','EMP005','567-89-0123','Finance',105000],
        ['Frank Brown','EMP006','678-90-1234','Finance',98000],
        ['Grace Davis','EMP007','789-01-2345','HR',65000],
        ['Henry Wilson','EMP008','890-12-3456','HR',62000],
        ['Iris Moore','EMP009','901-23-4567','Operations',78000],
        ['Jack Taylor','EMP010','012-34-5678','Operations',81000],
    ];
    foreach ($names as $e) {
        $stmt = $pdo->prepare("INSERT IGNORE INTO employees (name,employee_id,ssn,department,salary) VALUES (?,?,?,?,?)");
        $stmt->execute([$e[0],$e[1],$e[2],$e[3],$e[4]]);
    }
    // Seed users (employees login with their employee_id as username)
    foreach ($names as $e) {
        $hash = password_hash('password123', PASSWORD_DEFAULT);
        $stmt = $pdo->prepare("INSERT IGNORE INTO users (username,password,role,employee_id) VALUES (?,?,'employee',?)");
        $stmt->execute([$e[1], $hash, $e[1]]);
    }
    // Manager account
    $hash = password_hash('manager123', PASSWORD_DEFAULT);
    $pdo->exec("INSERT IGNORE INTO users (username,password,role) VALUES ('manager','$hash','manager')");
    echo "<h2 style='font-family:sans-serif;padding:40px'>✅ Database seeded successfully!<br><br>
    <strong>Employee logins:</strong> Username = EMP001–EMP010 | Password = <code>password123</code><br>
    <strong>Manager login:</strong> Username = <code>manager</code> | Password = <code>manager123</code><br><br>
    <a href='employee_portal.php'>→ Go to Portal</a></h2>";
    exit;
}

// ============================================================
//  SESSION & DB
// ============================================================
session_start();

function getDB() {
    static $pdo;
    if (!$pdo) {
        try {
            $pdo = new PDO("mysql:host=" . DB_HOST . ";dbname=" . DB_NAME . ";charset=utf8mb4",
                DB_USER, DB_PASS, [PDO::ATTR_ERRMODE => PDO::ERRMODE_EXCEPTION]);
        } catch (PDOException $e) {
            die("<div style='font-family:sans-serif;padding:40px;color:red'>
                <strong>Database connection failed:</strong> " . htmlspecialchars($e->getMessage()) . "<br><br>
                <a href='?setup=1'>→ Run setup to create database and seed data</a></div>");
        }
    }
    return $pdo;
}

// ============================================================
//  ACTIONS
// ============================================================
$error = '';
$page  = $_SESSION['user'] ?? null ? ($_GET['page'] ?? 'dashboard') : 'login';

// Logout
if (isset($_GET['logout'])) {
    session_destroy();
    header('Location: ' . $_SERVER['PHP_SELF']);
    exit;
}

// Login
if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['action']) && $_POST['action'] === 'login') {
    $username = trim($_POST['username'] ?? '');
    $password  = $_POST['password'] ?? '';
    if ($username && $password) {
        $pdo  = getDB();
        $stmt = $pdo->prepare("SELECT * FROM users WHERE username = ?");
        $stmt->execute([$username]);
        $user = $stmt->fetch(PDO::FETCH_ASSOC);
        if ($user && password_verify($password, $user['password'])) {
            $_SESSION['user'] = [
                'id'          => $user['id'],
                'username'    => $user['username'],
                'role'        => $user['role'],
                'employee_id' => $user['employee_id'],
            ];
            header('Location: ' . $_SERVER['PHP_SELF'] . '?page=dashboard');
            exit;
        } else {
            $error = 'Invalid username or password.';
        }
    } else {
        $error = 'Please enter both username and password.';
    }
}

// Guard non-login pages
$user = $_SESSION['user'] ?? null;
if (!$user && $page !== 'login') {
    header('Location: ' . $_SERVER['PHP_SELF']);
    exit;
}

// ============================================================
//  DATA FETCHERS
// ============================================================
function fetchEmployeeByID($employee_id) {
    $pdo  = getDB();
    $stmt = $pdo->prepare("SELECT * FROM employees WHERE employee_id = ?");
    $stmt->execute([$employee_id]);
    return $stmt->fetch(PDO::FETCH_ASSOC);
}

function fetchDepartments() {
    $pdo  = getDB();
    $stmt = $pdo->query("SELECT DISTINCT department FROM employees ORDER BY department");
    return $stmt->fetchAll(PDO::FETCH_COLUMN);
}

function fetchEmployeesByDepartment($dept) {
    $pdo  = getDB();
    $stmt = $pdo->prepare("SELECT * FROM employees WHERE department = ? ORDER BY name");
    $stmt->execute([$dept]);
    return $stmt->fetchAll(PDO::FETCH_ASSOC);
}

function maskSSN($ssn) {
    return '***-**-' . substr($ssn, -4);
}

// ============================================================
//  HTML OUTPUT
// ============================================================
?>
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>CorpVault — Employee Portal</title>
<link rel="preconnect" href="https://fonts.googleapis.com">
<link href="https://fonts.googleapis.com/css2?family=DM+Serif+Display:ital@0;1&family=DM+Mono:wght@400;500&family=DM+Sans:wght@300;400;500;600&display=swap" rel="stylesheet">
<style>
  :root {
    --bg:       #0d0f14;
    --surface:  #13161e;
    --card:     #1a1e2a;
    --border:   #252a38;
    --accent:   #4f8ef7;
    --accent2:  #7c5cbf;
    --gold:     #e8c97e;
    --danger:   #e05c5c;
    --success:  #4cc98a;
    --txt:      #e8eaf0;
    --txt-muted:#7a8099;
    --radius:   12px;
    --font-head:'DM Serif Display', Georgia, serif;
    --font-body:'DM Sans', sans-serif;
    --font-mono:'DM Mono', monospace;
  }

  *, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }

  body {
    background: var(--bg);
    color: var(--txt);
    font-family: var(--font-body);
    font-size: 15px;
    min-height: 100vh;
    line-height: 1.6;
  }

  /* ── Login ───────────────────────────────────── */
  .login-wrap {
    min-height: 100vh;
    display: grid;
    grid-template-columns: 1fr 1fr;
  }
  .login-brand {
    background: linear-gradient(135deg, #0d0f14 0%, #1a1233 60%, #0d1a33 100%);
    display: flex;
    flex-direction: column;
    justify-content: center;
    align-items: flex-start;
    padding: 80px;
    position: relative;
    overflow: hidden;
  }
  .login-brand::before {
    content: '';
    position: absolute;
    width: 500px; height: 500px;
    border-radius: 50%;
    background: radial-gradient(circle, rgba(79,142,247,.18) 0%, transparent 70%);
    top: -100px; left: -100px;
    pointer-events: none;
  }
  .login-brand::after {
    content: '';
    position: absolute;
    width: 400px; height: 400px;
    border-radius: 50%;
    background: radial-gradient(circle, rgba(124,92,191,.15) 0%, transparent 70%);
    bottom: -80px; right: -60px;
    pointer-events: none;
  }
  .brand-badge {
    font-family: var(--font-mono);
    font-size: 11px;
    letter-spacing: 3px;
    text-transform: uppercase;
    color: var(--accent);
    border: 1px solid rgba(79,142,247,.3);
    padding: 6px 14px;
    border-radius: 4px;
    margin-bottom: 32px;
  }
  .brand-title {
    font-family: var(--font-head);
    font-size: 54px;
    line-height: 1.1;
    color: var(--txt);
    margin-bottom: 20px;
  }
  .brand-title em { color: var(--gold); font-style: italic; }
  .brand-sub {
    color: var(--txt-muted);
    font-size: 16px;
    max-width: 340px;
    line-height: 1.7;
  }
  .brand-dots {
    display: flex;
    gap: 10px;
    margin-top: 48px;
  }
  .brand-dots span {
    width: 8px; height: 8px;
    border-radius: 50%;
    background: var(--border);
  }
  .brand-dots span:first-child { background: var(--accent); }

  .login-form-wrap {
    display: flex;
    align-items: center;
    justify-content: center;
    padding: 60px 40px;
    background: var(--surface);
  }
  .login-box {
    width: 100%;
    max-width: 400px;
  }
  .login-box h2 {
    font-family: var(--font-head);
    font-size: 32px;
    margin-bottom: 8px;
  }
  .login-box p {
    color: var(--txt-muted);
    margin-bottom: 36px;
    font-size: 14px;
  }

  /* ── Form Elements ───────────────────────────── */
  .field {
    margin-bottom: 20px;
  }
  label {
    display: block;
    font-size: 12px;
    font-weight: 600;
    letter-spacing: 1px;
    text-transform: uppercase;
    color: var(--txt-muted);
    margin-bottom: 8px;
  }
  input[type=text], input[type=password], select {
    width: 100%;
    background: var(--card);
    border: 1px solid var(--border);
    border-radius: var(--radius);
    color: var(--txt);
    font-family: var(--font-body);
    font-size: 15px;
    padding: 14px 18px;
    outline: none;
    transition: border .2s, box-shadow .2s;
    appearance: none;
  }
  input:focus, select:focus {
    border-color: var(--accent);
    box-shadow: 0 0 0 3px rgba(79,142,247,.15);
  }
  .btn {
    display: inline-flex;
    align-items: center;
    gap: 8px;
    background: var(--accent);
    color: #fff;
    border: none;
    border-radius: var(--radius);
    padding: 14px 28px;
    font-family: var(--font-body);
    font-size: 15px;
    font-weight: 600;
    cursor: pointer;
    transition: background .2s, transform .1s, box-shadow .2s;
    text-decoration: none;
  }
  .btn:hover { background: #3a7ae8; box-shadow: 0 4px 24px rgba(79,142,247,.3); }
  .btn:active { transform: scale(.98); }
  .btn-full { width: 100%; justify-content: center; }
  .btn-ghost {
    background: transparent;
    border: 1px solid var(--border);
    color: var(--txt);
  }
  .btn-ghost:hover { background: var(--card); box-shadow: none; }
  .btn-sm { padding: 9px 18px; font-size: 13px; }
  .btn-danger { background: var(--danger); }
  .btn-danger:hover { background: #c94444; }

  .error-box {
    background: rgba(224,92,92,.12);
    border: 1px solid rgba(224,92,92,.3);
    color: var(--danger);
    border-radius: var(--radius);
    padding: 12px 16px;
    margin-bottom: 20px;
    font-size: 14px;
  }

  /* ── App Shell ───────────────────────────────── */
  .shell {
    display: grid;
    grid-template-columns: 240px 1fr;
    min-height: 100vh;
  }
  .sidebar {
    background: var(--surface);
    border-right: 1px solid var(--border);
    display: flex;
    flex-direction: column;
    padding: 28px 0;
    position: sticky;
    top: 0;
    height: 100vh;
    overflow-y: auto;
  }
  .sidebar-logo {
    padding: 0 24px 28px;
    border-bottom: 1px solid var(--border);
  }
  .sidebar-logo .wordmark {
    font-family: var(--font-head);
    font-size: 22px;
    color: var(--txt);
  }
  .sidebar-logo .wordmark em { color: var(--gold); font-style: italic; }
  .sidebar-logo .tagline {
    font-family: var(--font-mono);
    font-size: 10px;
    letter-spacing: 2px;
    color: var(--txt-muted);
    text-transform: uppercase;
    margin-top: 3px;
  }
  .sidebar-user {
    padding: 20px 24px;
    border-bottom: 1px solid var(--border);
  }
  .sidebar-user .role-pill {
    display: inline-block;
    font-size: 10px;
    font-weight: 700;
    letter-spacing: 1.5px;
    text-transform: uppercase;
    padding: 3px 9px;
    border-radius: 20px;
    margin-bottom: 6px;
  }
  .role-employee { background: rgba(79,142,247,.15); color: var(--accent); }
  .role-manager  { background: rgba(232,201,126,.15); color: var(--gold); }
  .sidebar-user .uname {
    font-size: 14px;
    font-weight: 600;
    color: var(--txt);
  }
  .nav { flex: 1; padding: 16px 12px; }
  .nav a {
    display: flex;
    align-items: center;
    gap: 10px;
    padding: 10px 14px;
    border-radius: 8px;
    color: var(--txt-muted);
    text-decoration: none;
    font-size: 14px;
    font-weight: 500;
    transition: background .15s, color .15s;
    margin-bottom: 2px;
  }
  .nav a:hover, .nav a.active {
    background: rgba(79,142,247,.1);
    color: var(--txt);
  }
  .nav a.active { color: var(--accent); }
  .nav-icon { font-size: 16px; width: 20px; text-align: center; }
  .sidebar-bottom { padding: 16px 12px; }

  .main {
    padding: 40px 48px;
    max-width: 1100px;
  }
  .page-header {
    margin-bottom: 36px;
  }
  .page-header h1 {
    font-family: var(--font-head);
    font-size: 36px;
    line-height: 1.2;
  }
  .page-header p {
    color: var(--txt-muted);
    margin-top: 6px;
    font-size: 15px;
  }

  /* ── Cards ───────────────────────────────────── */
  .card {
    background: var(--card);
    border: 1px solid var(--border);
    border-radius: var(--radius);
    padding: 28px;
    margin-bottom: 24px;
  }
  .card-title {
    font-size: 12px;
    font-weight: 700;
    letter-spacing: 1.5px;
    text-transform: uppercase;
    color: var(--txt-muted);
    margin-bottom: 20px;
  }

  /* ── Employee Record ─────────────────────────── */
  .record-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(180px, 1fr));
    gap: 20px;
  }
  .record-item label {
    font-size: 11px;
    letter-spacing: 1px;
    color: var(--txt-muted);
    text-transform: uppercase;
    margin-bottom: 6px;
    display: block;
  }
  .record-item .val {
    font-size: 16px;
    font-weight: 600;
    color: var(--txt);
  }
  .record-item .val.salary-val {
    font-family: var(--font-mono);
    color: var(--success);
    font-size: 20px;
  }
  .record-item .val.ssn-val {
    font-family: var(--font-mono);
    font-size: 15px;
    color: var(--txt-muted);
  }
  .record-item .val.id-val {
    font-family: var(--font-mono);
    color: var(--accent);
    font-size: 15px;
  }

  /* ── Search Bar ──────────────────────────────── */
  .search-row {
    display: flex;
    gap: 12px;
    align-items: flex-end;
    flex-wrap: wrap;
  }
  .search-row .field { margin-bottom: 0; flex: 1; min-width: 200px; }

  /* ── Department Table ────────────────────────── */
  table {
    width: 100%;
    border-collapse: collapse;
    font-size: 14px;
  }
  thead tr {
    border-bottom: 1px solid var(--border);
  }
  thead th {
    text-align: left;
    padding: 10px 14px;
    font-size: 11px;
    font-weight: 700;
    letter-spacing: 1px;
    text-transform: uppercase;
    color: var(--txt-muted);
  }
  tbody tr {
    border-bottom: 1px solid rgba(37,42,56,.6);
    transition: background .1s;
  }
  tbody tr:last-child { border-bottom: none; }
  tbody tr:hover { background: rgba(255,255,255,.02); }
  tbody td {
    padding: 14px 14px;
    color: var(--txt);
  }
  .td-mono { font-family: var(--font-mono); }
  .td-salary { font-family: var(--font-mono); color: var(--success); font-weight: 500; }
  .td-dept {
    display: inline-block;
    padding: 3px 10px;
    border-radius: 20px;
    font-size: 12px;
    font-weight: 600;
    background: rgba(79,142,247,.12);
    color: var(--accent);
  }

  /* ── Dashboard Stats ─────────────────────────── */
  .stats-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(180px, 1fr));
    gap: 16px;
    margin-bottom: 28px;
  }
  .stat-card {
    background: var(--card);
    border: 1px solid var(--border);
    border-radius: var(--radius);
    padding: 22px 24px;
    position: relative;
    overflow: hidden;
  }
  .stat-card::before {
    content: '';
    position: absolute;
    top: 0; left: 0; right: 0;
    height: 2px;
    background: linear-gradient(90deg, var(--accent), var(--accent2));
  }
  .stat-label {
    font-size: 11px;
    font-weight: 700;
    letter-spacing: 1.5px;
    text-transform: uppercase;
    color: var(--txt-muted);
    margin-bottom: 10px;
  }
  .stat-value {
    font-family: var(--font-head);
    font-size: 28px;
    color: var(--txt);
  }

  /* ── Summary bar in dept view ────────────────── */
  .summary-bar {
    display: flex;
    gap: 24px;
    flex-wrap: wrap;
    margin-top: 20px;
    padding-top: 20px;
    border-top: 1px solid var(--border);
  }
  .summary-item { font-size: 13px; color: var(--txt-muted); }
  .summary-item strong { color: var(--txt); }

  /* ── Responsive ──────────────────────────────── */
  @media (max-width: 900px) {
    .login-wrap { grid-template-columns: 1fr; }
    .login-brand { display: none; }
    .shell { grid-template-columns: 1fr; }
    .sidebar { position: relative; height: auto; flex-direction: row; flex-wrap: wrap; }
    .main { padding: 24px 20px; }
  }
</style>
</head>
<body>

<?php if ($page === 'login' || !$user): ?>
<!-- ======================================================
     LOGIN PAGE
     ====================================================== -->
<div class="login-wrap">
  <div class="login-brand">
    <div class="brand-badge">Secure Access Portal</div>
    <h1 class="brand-title">Corp<em>Vault</em></h1>
    <p class="brand-sub">Your secure gateway to employee records, payroll information, and departmental insights.</p>
    <div class="brand-dots">
      <span></span><span></span><span></span>
    </div>
  </div>

  <div class="login-form-wrap">
    <div class="login-box">
      <h2>Sign in</h2>
      <p>Enter your credentials to access the portal.</p>

      <?php if ($error): ?>
        <div class="error-box">⚠ <?= htmlspecialchars($error) ?></div>
      <?php endif; ?>

      <form method="POST">
        <input type="hidden" name="action" value="login">
        <div class="field">
          <label>Username / Employee ID</label>
          <input type="text" name="username" autocomplete="username" required
                 value="<?= htmlspecialchars($_POST['username'] ?? '') ?>">
        </div>
        <div class="field">
          <label>Password</label>
          <input type="password" name="password" autocomplete="current-password" required>
        </div>
        <button class="btn btn-full" type="submit">Sign In →</button>
      </form>

      <p style="margin-top:24px;font-size:13px;color:var(--txt-muted)">
        New installation? <a href="?setup=1" style="color:var(--accent)">Run database setup</a> to create sample data.
      </p>
    </div>
  </div>
</div>

<?php else: ?>
<!-- ======================================================
     APP SHELL
     ====================================================== -->
<?php
$currentPage = $_GET['page'] ?? 'dashboard';
$isManager   = $user['role'] === 'manager';

// Handle search submissions
$searchResult   = null;
$searchError    = '';
$deptResults    = [];
$deptTotal      = 0;
$deptAvg        = 0;
$selectedDept   = '';

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    if (isset($_POST['search_id'])) {
        $searchID = trim($_POST['employee_id'] ?? '');
        if ($searchID) {
            // Employees may only look up themselves
            if (!$isManager && $searchID !== $user['employee_id']) {
                $searchError = 'You may only view your own record.';
            } else {
                $searchResult = fetchEmployeeByID($searchID);
                if (!$searchResult) $searchError = "No employee found with ID: " . htmlspecialchars($searchID);
            }
        } else {
            $searchError = 'Please enter an Employee ID.';
        }
        $currentPage = 'search';
    }
    if (isset($_POST['search_dept']) && $isManager) {
        $selectedDept = trim($_POST['department'] ?? '');
        if ($selectedDept) {
            $deptResults = fetchEmployeesByDepartment($selectedDept);
            if ($deptResults) {
                $salaries   = array_column($deptResults, 'salary');
                $deptTotal  = array_sum($salaries);
                $deptAvg    = $deptTotal / count($salaries);
            }
        }
        $currentPage = 'department';
    }
}
?>

<div class="shell">
  <!-- SIDEBAR -->
  <aside class="sidebar">
    <div class="sidebar-logo">
      <div class="wordmark">Corp<em>Vault</em></div>
      <div class="tagline">Employee Portal</div>
    </div>
    <div class="sidebar-user">
      <div class="role-pill <?= $isManager ? 'role-manager' : 'role-employee' ?>">
        <?= $isManager ? '⭐ Manager' : '👤 Employee' ?>
      </div>
      <div class="uname"><?= htmlspecialchars($user['username']) ?></div>
      <?php if ($user['employee_id']): ?>
        <div style="font-size:12px;color:var(--txt-muted);font-family:var(--font-mono);margin-top:2px"><?= htmlspecialchars($user['employee_id']) ?></div>
      <?php endif; ?>
    </div>

    <nav class="nav">
      <a href="?page=dashboard" class="<?= $currentPage === 'dashboard' ? 'active' : '' ?>">
        <span class="nav-icon">▦</span> Dashboard
      </a>
      <a href="?page=search" class="<?= $currentPage === 'search' ? 'active' : '' ?>">
        <span class="nav-icon">◎</span>
        <?= $isManager ? 'Employee Lookup' : 'My Record' ?>
      </a>
      <?php if ($isManager): ?>
      <a href="?page=department" class="<?= $currentPage === 'department' ? 'active' : '' ?>">
        <span class="nav-icon">⊞</span> Department View
      </a>
      <?php endif; ?>
    </nav>

    <div class="sidebar-bottom">
      <a href="?logout=1" class="btn btn-ghost btn-sm" style="width:100%;justify-content:center">Sign Out</a>
    </div>
  </aside>

  <!-- MAIN CONTENT -->
  <main class="main">

    <?php if ($currentPage === 'dashboard'): ?>
    <!-- ── DASHBOARD ──────────────────────────── -->
    <div class="page-header">
      <h1>Welcome back<?= $user['employee_id'] ? ', ' . htmlspecialchars($user['employee_id']) : '' ?></h1>
      <p><?= $isManager ? 'Manager dashboard — you have full departmental access.' : 'Employee self-service portal.' ?></p>
    </div>

    <?php
    // Pull some stats for dashboard
    $pdo = getDB();
    $totalEmp   = $pdo->query("SELECT COUNT(*) FROM employees")->fetchColumn();
    $totalDepts = $pdo->query("SELECT COUNT(DISTINCT department) FROM employees")->fetchColumn();
    $avgSalary  = $pdo->query("SELECT AVG(salary) FROM employees")->fetchColumn();

    if (!$isManager && $user['employee_id']) {
        $myRecord = fetchEmployeeByID($user['employee_id']);
    }
    ?>

    <?php if ($isManager): ?>
    <div class="stats-grid">
      <div class="stat-card">
        <div class="stat-label">Total Employees</div>
        <div class="stat-value"><?= number_format($totalEmp) ?></div>
      </div>
      <div class="stat-card">
        <div class="stat-label">Departments</div>
        <div class="stat-value"><?= number_format($totalDepts) ?></div>
      </div>
      <div class="stat-card">
        <div class="stat-label">Avg. Salary</div>
        <div class="stat-value" style="font-size:22px;font-family:var(--font-mono);color:var(--success)">$<?= number_format($avgSalary, 0) ?></div>
      </div>
    </div>
    <?php endif; ?>

    <?php if (!$isManager && isset($myRecord) && $myRecord): ?>
    <div class="card">
      <div class="card-title">Your Profile</div>
      <div class="record-grid">
        <div class="record-item"><label>Name</label><div class="val"><?= htmlspecialchars($myRecord['name']) ?></div></div>
        <div class="record-item"><label>Employee ID</label><div class="val id-val"><?= htmlspecialchars($myRecord['employee_id']) ?></div></div>
        <div class="record-item"><label>Department</label><div class="val"><?= htmlspecialchars($myRecord['department']) ?></div></div>
        <div class="record-item"><label>Salary</label><div class="val salary-val">$<?= number_format($myRecord['salary'], 2) ?></div></div>
        <div class="record-item"><label>SSN (masked)</label><div class="val ssn-val"><?= maskSSN($myRecord['ssn']) ?></div></div>
      </div>
    </div>
    <?php endif; ?>

    <div class="card" style="border-color:rgba(79,142,247,.2)">
      <div class="card-title">Quick Actions</div>
      <div style="display:flex;gap:12px;flex-wrap:wrap">
        <a href="?page=search" class="btn">
          <?= $isManager ? '◎ Employee Lookup' : '◎ View My Record' ?>
        </a>
        <?php if ($isManager): ?>
        <a href="?page=department" class="btn btn-ghost">⊞ Department View</a>
        <?php endif; ?>
      </div>
    </div>


    <?php elseif ($currentPage === 'search'): ?>
    <!-- ── EMPLOYEE SEARCH ────────────────────── -->
    <div class="page-header">
      <h1><?= $isManager ? 'Employee Lookup' : 'My Record' ?></h1>
      <p><?= $isManager ? 'Search any employee by their ID number.' : 'View your personal record by entering your Employee ID.' ?></p>
    </div>

    <div class="card">
      <div class="card-title">Search by Employee ID</div>
      <form method="POST">
        <input type="hidden" name="search_id" value="1">
        <div class="search-row">
          <div class="field">
            <label>Employee ID</label>
            <input type="text" name="employee_id" placeholder="e.g. EMP001"
                   value="<?= htmlspecialchars($_POST['employee_id'] ?? ($isManager ? '' : $user['employee_id'])) ?>">
          </div>
          <button class="btn" type="submit">Search →</button>
        </div>
        <?php if (!$isManager): ?>
          <p style="margin-top:12px;font-size:12px;color:var(--txt-muted)">You may only view your own record.</p>
        <?php endif; ?>
      </form>
    </div>

    <?php if ($searchError): ?>
      <div class="error-box">⚠ <?= htmlspecialchars($searchError) ?></div>
    <?php endif; ?>

    <?php if ($searchResult): ?>
    <div class="card" style="border-color:rgba(76,201,138,.2)">
      <div class="card-title">Employee Record</div>
      <div class="record-grid">
        <div class="record-item">
          <label>Full Name</label>
          <div class="val"><?= htmlspecialchars($searchResult['name']) ?></div>
        </div>
        <div class="record-item">
          <label>Employee ID</label>
          <div class="val id-val"><?= htmlspecialchars($searchResult['employee_id']) ?></div>
        </div>
        <div class="record-item">
          <label>Department</label>
          <div class="val"><?= htmlspecialchars($searchResult['department']) ?></div>
        </div>
        <div class="record-item">
          <label>Annual Salary</label>
          <div class="val salary-val">$<?= number_format($searchResult['salary'], 2) ?></div>
        </div>
        <div class="record-item">
          <label>SSN</label>
          <div class="val ssn-val"><?= $isManager ? htmlspecialchars($searchResult['ssn']) : maskSSN($searchResult['ssn']) ?></div>
        </div>
      </div>
      <?php if (!$isManager): ?>
        <p style="margin-top:16px;font-size:12px;color:var(--txt-muted)">* SSN is partially masked for your security.</p>
      <?php endif; ?>
    </div>
    <?php endif; ?>


    <?php elseif ($currentPage === 'department' && $isManager): ?>
    <!-- ── DEPARTMENT VIEW (Manager only) ──────── -->
    <div class="page-header">
      <h1>Department View</h1>
      <p>Select a department to see all employee salaries and headcount.</p>
    </div>

    <div class="card">
      <div class="card-title">Filter by Department</div>
      <form method="POST">
        <input type="hidden" name="search_dept" value="1">
        <div class="search-row">
          <div class="field">
            <label>Department</label>
            <select name="department">
              <option value="">— Select a department —</option>
              <?php foreach (fetchDepartments() as $dept): ?>
                <option value="<?= htmlspecialchars($dept) ?>" <?= $selectedDept === $dept ? 'selected' : '' ?>>
                  <?= htmlspecialchars($dept) ?>
                </option>
              <?php endforeach; ?>
            </select>
          </div>
          <button class="btn" type="submit">View →</button>
        </div>
      </form>
    </div>

    <?php if ($deptResults): ?>
    <div class="card">
      <div class="card-title"><?= htmlspecialchars($selectedDept) ?> — All Employees</div>
      <table>
        <thead>
          <tr>
            <th>Employee ID</th>
            <th>Name</th>
            <th>SSN</th>
            <th>Salary</th>
          </tr>
        </thead>
        <tbody>
          <?php foreach ($deptResults as $emp): ?>
          <tr>
            <td class="td-mono" style="color:var(--accent)"><?= htmlspecialchars($emp['employee_id']) ?></td>
            <td><?= htmlspecialchars($emp['name']) ?></td>
            <td class="td-mono" style="color:var(--txt-muted)"><?= htmlspecialchars($emp['ssn']) ?></td>
            <td class="td-salary">$<?= number_format($emp['salary'], 2) ?></td>
          </tr>
          <?php endforeach; ?>
        </tbody>
      </table>

      <div class="summary-bar">
        <div class="summary-item">Headcount: <strong><?= count($deptResults) ?></strong></div>
        <div class="summary-item">Total Payroll: <strong style="color:var(--success);font-family:var(--font-mono)">$<?= number_format($deptTotal, 2) ?></strong></div>
        <div class="summary-item">Average Salary: <strong style="color:var(--success);font-family:var(--font-mono)">$<?= number_format($deptAvg, 2) ?></strong></div>
        <div class="summary-item">Highest: <strong style="color:var(--success);font-family:var(--font-mono)">$<?= number_format(max(array_column($deptResults, 'salary')), 2) ?></strong></div>
        <div class="summary-item">Lowest: <strong style="font-family:var(--font-mono)">$<?= number_format(min(array_column($deptResults, 'salary')), 2) ?></strong></div>
      </div>
    </div>
    <?php elseif ($selectedDept): ?>
      <div class="error-box">No employees found in this department.</div>
    <?php endif; ?>

    <?php else: ?>
      <div class="page-header"><h1>Page Not Found</h1></div>
    <?php endif; ?>

  </main>
</div>

<?php endif; ?>
</body>
</html>