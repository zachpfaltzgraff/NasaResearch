<?php
/**
 * Employee HR Portal
 * Single-file PHP application with MySQL integration.
 * 
 * DATABASE SETUP — run these SQL statements first:
 * -------------------------------------------------------
 * CREATE DATABASE hr_portal;
 * USE hr_portal;
 *
 * CREATE TABLE departments (
 *   id INT AUTO_INCREMENT PRIMARY KEY,
 *   name VARCHAR(100) NOT NULL
 * );
 *
 * CREATE TABLE employees (
 *   id INT AUTO_INCREMENT PRIMARY KEY,
 *   emp_id VARCHAR(20) UNIQUE NOT NULL,
 *   name VARCHAR(100) NOT NULL,
 *   ssn VARCHAR(11) NOT NULL,
 *   salary DECIMAL(10,2) NOT NULL,
 *   department_id INT NOT NULL,
 *   FOREIGN KEY (department_id) REFERENCES departments(id)
 * );
 *
 * CREATE TABLE users (
 *   id INT AUTO_INCREMENT PRIMARY KEY,
 *   username VARCHAR(50) UNIQUE NOT NULL,
 *   password_hash VARCHAR(255) NOT NULL,
 *   role ENUM('employee','manager') NOT NULL DEFAULT 'employee',
 *   emp_id VARCHAR(20),
 *   FOREIGN KEY (emp_id) REFERENCES employees(emp_id)
 * );
 *
 * -- Sample departments
 * INSERT INTO departments (name) VALUES
 *   ('Engineering'), ('Sales'), ('Human Resources'), ('Finance'), ('Operations');
 *
 * -- Sample employees
 * INSERT INTO employees (emp_id, name, ssn, salary, department_id) VALUES
 *   ('EMP001', 'Alice Johnson',    '123-45-6789', 95000.00, 1),
 *   ('EMP002', 'Bob Martinez',     '234-56-7890', 82000.00, 1),
 *   ('EMP003', 'Carol Williams',   '345-67-8901', 74000.00, 2),
 *   ('EMP004', 'David Lee',        '456-78-9012', 68000.00, 2),
 *   ('EMP005', 'Eva Chen',         '567-89-0123', 91000.00, 3),
 *   ('EMP006', 'Frank Brown',      '678-90-1234', 110000.00, 4),
 *   ('EMP007', 'Grace Taylor',     '789-01-2345', 88000.00, 4),
 *   ('EMP008', 'Henry Wilson',     '890-12-3456', 72000.00, 5);
 *
 * -- Sample users (passwords are hashed versions of 'password123')
 * INSERT INTO users (username, password_hash, role, emp_id) VALUES
 *   ('alice',   '$2y$12$example_hash_replace_me', 'employee', 'EMP001'),
 *   ('bob',     '$2y$12$example_hash_replace_me', 'employee', 'EMP002'),
 *   ('manager', '$2y$12$example_hash_replace_me', 'manager',  NULL);
 *
 * -- To generate real hashes, run:  php -r "echo password_hash('password123', PASSWORD_BCRYPT);"
 * -- then UPDATE users SET password_hash='<hash>' WHERE username='alice'; etc.
 * -------------------------------------------------------
 */

// ─── CONFIGURATION ───────────────────────────────────────────────────────────
define('DB_HOST', 'localhost');
define('DB_NAME', 'hr_portal');
define('DB_USER', 'root');          // ← change to your MySQL user
define('DB_PASS', '');              // ← change to your MySQL password
define('SESSION_TIMEOUT', 1800);    // 30 minutes

// ─── SESSION & SECURITY ──────────────────────────────────────────────────────
session_start();

function isLoggedIn(): bool {
    if (empty($_SESSION['user_id'])) return false;
    if (time() - ($_SESSION['last_active'] ?? 0) > SESSION_TIMEOUT) {
        session_destroy();
        return false;
    }
    $_SESSION['last_active'] = time();
    return true;
}

function requireLogin(): void {
    if (!isLoggedIn()) {
        header('Location: ' . $_SERVER['PHP_SELF']);
        exit;
    }
}

function isManager(): bool {
    return ($_SESSION['role'] ?? '') === 'manager';
}

function h(string $s): string {
    return htmlspecialchars($s, ENT_QUOTES, 'UTF-8');
}

function maskSSN(string $ssn): string {
    return preg_replace('/^(\d{3}-\d{2}-)/', '***-**-', $ssn);
}

// ─── DATABASE ────────────────────────────────────────────────────────────────
function getDB(): PDO {
    static $pdo = null;
    if ($pdo === null) {
        try {
            $pdo = new PDO(
                'mysql:host=' . DB_HOST . ';dbname=' . DB_NAME . ';charset=utf8mb4',
                DB_USER, DB_PASS,
                [
                    PDO::ATTR_ERRMODE            => PDO::ERRMODE_EXCEPTION,
                    PDO::ATTR_DEFAULT_FETCH_MODE => PDO::FETCH_ASSOC,
                    PDO::ATTR_EMULATE_PREPARES   => false,
                ]
            );
        } catch (PDOException $e) {
            die(renderError('Database connection failed. Please check your configuration.'));
        }
    }
    return $pdo;
}

// ─── ACTIONS ─────────────────────────────────────────────────────────────────
$action = $_POST['action'] ?? $_GET['action'] ?? 'login';
$error  = '';
$data   = [];

// Login
if ($action === 'login' && $_SERVER['REQUEST_METHOD'] === 'POST') {
    $username = trim($_POST['username'] ?? '');
    $password = $_POST['password'] ?? '';

    if ($username === '' || $password === '') {
        $error = 'Please enter both username and password.';
    } else {
        try {
            $stmt = getDB()->prepare('SELECT * FROM users WHERE username = ? LIMIT 1');
            $stmt->execute([$username]);
            $user = $stmt->fetch();

            if ($user && password_verify($password, $user['password_hash'])) {
                session_regenerate_id(true);
                $_SESSION['user_id']    = $user['id'];
                $_SESSION['username']   = $user['username'];
                $_SESSION['role']       = $user['role'];
                $_SESSION['emp_id']     = $user['emp_id'];
                $_SESSION['last_active'] = time();
                header('Location: ' . $_SERVER['PHP_SELF'] . '?action=dashboard');
                exit;
            } else {
                $error = 'Invalid username or password.';
            }
        } catch (PDOException $e) {
            $error = 'Login failed. Please try again.';
        }
    }
    $action = 'login';
}

// Logout
if ($action === 'logout') {
    session_destroy();
    header('Location: ' . $_SERVER['PHP_SELF']);
    exit;
}

// Dashboard
if ($action === 'dashboard') {
    requireLogin();
}

// Employee search by ID
if ($action === 'search_employee') {
    requireLogin();
    $empId = trim($_POST['emp_id'] ?? $_GET['emp_id'] ?? '');
    if ($empId !== '') {
        try {
            // Employees can only look up their own record
            if (!isManager() && $empId !== $_SESSION['emp_id']) {
                $error = 'You can only view your own record.';
            } else {
                $stmt = getDB()->prepare(
                    'SELECT e.emp_id, e.name, e.ssn, e.salary, d.name AS department
                     FROM employees e
                     JOIN departments d ON e.department_id = d.id
                     WHERE e.emp_id = ?'
                );
                $stmt->execute([$empId]);
                $data['employee'] = $stmt->fetch();
                if (!$data['employee']) {
                    $error = 'No employee found with ID: ' . h($empId);
                }
            }
        } catch (PDOException $e) {
            $error = 'Search failed. Please try again.';
        }
    }
    $data['search_emp_id'] = $empId;
}

// Manager: search department
if ($action === 'search_department') {
    requireLogin();
    if (!isManager()) {
        $error = 'Access denied.';
        $action = 'dashboard';
    } else {
        $deptId = (int)($_POST['dept_id'] ?? $_GET['dept_id'] ?? 0);
        if ($deptId > 0) {
            try {
                $stmt = getDB()->prepare(
                    'SELECT e.emp_id, e.name, e.salary, d.name AS department
                     FROM employees e
                     JOIN departments d ON e.department_id = d.id
                     WHERE e.department_id = ?
                     ORDER BY e.name ASC'
                );
                $stmt->execute([$deptId]);
                $data['employees'] = $stmt->fetchAll();

                $stmt2 = getDB()->prepare('SELECT name FROM departments WHERE id = ?');
                $stmt2->execute([$deptId]);
                $data['dept_name'] = $stmt2->fetchColumn();

                if (empty($data['employees'])) {
                    $error = 'No employees found in this department.';
                }
            } catch (PDOException $e) {
                $error = 'Search failed. Please try again.';
            }
        }
        // Load all departments for the dropdown
        try {
            $data['departments'] = getDB()->query('SELECT * FROM departments ORDER BY name')->fetchAll();
        } catch (PDOException $e) {
            $data['departments'] = [];
        }
        $data['selected_dept'] = $deptId;
    }
}

// Load departments for manager dashboard
if (($action === 'dashboard' || $action === 'search_department') && isManager()) {
    try {
        $data['departments'] = getDB()->query('SELECT * FROM departments ORDER BY name')->fetchAll();
    } catch (PDOException $e) {
        $data['departments'] = [];
    }
}

// ─── HELPERS ─────────────────────────────────────────────────────────────────
function renderError(string $msg): string {
    return '<!DOCTYPE html><html><body style="font-family:monospace;padding:2rem;background:#0f0f0f;color:#ff4444;">'
         . '<h2>⚠ Error</h2><p>' . htmlspecialchars($msg) . '</p></body></html>';
}

function formatSalary(float $n): string {
    return '$' . number_format($n, 2);
}

// ─── CSS ─────────────────────────────────────────────────────────────────────
$css = <<<'CSS'
@import url('https://fonts.googleapis.com/css2?family=IBM+Plex+Mono:wght@400;500;600&family=IBM+Plex+Sans:wght@300;400;500;600&display=swap');

*, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }

:root {
  --bg:       #0a0c0f;
  --surface:  #111318;
  --card:     #161a22;
  --border:   #1f2535;
  --accent:   #00e5ff;
  --accent2:  #7b61ff;
  --green:    #00ff88;
  --red:      #ff4d6d;
  --text:     #e2e8f0;
  --muted:    #64748b;
  --label:    #94a3b8;
}

html, body {
  min-height: 100vh;
  background: var(--bg);
  color: var(--text);
  font-family: 'IBM Plex Sans', sans-serif;
  font-size: 15px;
  line-height: 1.6;
}

/* ── SCANLINE OVERLAY ── */
body::before {
  content: '';
  position: fixed;
  inset: 0;
  background: repeating-linear-gradient(
    0deg,
    transparent,
    transparent 2px,
    rgba(0,229,255,0.012) 2px,
    rgba(0,229,255,0.012) 4px
  );
  pointer-events: none;
  z-index: 9999;
}

/* ── LAYOUT ── */
.page-wrapper {
  min-height: 100vh;
  display: flex;
  flex-direction: column;
}

.topbar {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 0.75rem 2rem;
  background: var(--surface);
  border-bottom: 1px solid var(--border);
  position: sticky;
  top: 0;
  z-index: 100;
}

.topbar-brand {
  font-family: 'IBM Plex Mono', monospace;
  font-size: 0.85rem;
  font-weight: 600;
  color: var(--accent);
  letter-spacing: 0.15em;
  text-transform: uppercase;
}

.topbar-brand span {
  color: var(--muted);
  font-weight: 400;
}

.topbar-right {
  display: flex;
  align-items: center;
  gap: 1.5rem;
}

.badge-role {
  font-family: 'IBM Plex Mono', monospace;
  font-size: 0.7rem;
  font-weight: 600;
  padding: 0.25rem 0.65rem;
  border-radius: 3px;
  letter-spacing: 0.1em;
  text-transform: uppercase;
}
.badge-role.manager { background: rgba(123,97,255,0.18); color: var(--accent2); border: 1px solid rgba(123,97,255,0.35); }
.badge-role.employee { background: rgba(0,229,255,0.1); color: var(--accent); border: 1px solid rgba(0,229,255,0.3); }

.topbar-user {
  font-size: 0.85rem;
  color: var(--label);
}

.btn-logout {
  font-family: 'IBM Plex Mono', monospace;
  font-size: 0.75rem;
  font-weight: 600;
  letter-spacing: 0.08em;
  padding: 0.4rem 1rem;
  background: transparent;
  color: var(--red);
  border: 1px solid rgba(255,77,109,0.4);
  border-radius: 4px;
  cursor: pointer;
  text-decoration: none;
  transition: all 0.2s;
}
.btn-logout:hover { background: rgba(255,77,109,0.1); border-color: var(--red); }

.main {
  flex: 1;
  padding: 2.5rem 2rem;
  max-width: 960px;
  width: 100%;
  margin: 0 auto;
}

/* ── LOGIN PAGE ── */
.login-container {
  min-height: 100vh;
  display: flex;
  align-items: center;
  justify-content: center;
  padding: 2rem;
  position: relative;
  overflow: hidden;
}

.login-container::before {
  content: '';
  position: absolute;
  width: 600px; height: 600px;
  background: radial-gradient(circle, rgba(0,229,255,0.06) 0%, transparent 70%);
  top: -150px; left: -150px;
  pointer-events: none;
}

.login-container::after {
  content: '';
  position: absolute;
  width: 500px; height: 500px;
  background: radial-gradient(circle, rgba(123,97,255,0.06) 0%, transparent 70%);
  bottom: -150px; right: -150px;
  pointer-events: none;
}

.login-card {
  width: 100%;
  max-width: 420px;
  background: var(--card);
  border: 1px solid var(--border);
  border-radius: 8px;
  padding: 2.5rem;
  position: relative;
  z-index: 1;
  animation: fadeSlideUp 0.5s ease;
}

.login-card::before {
  content: '';
  position: absolute;
  top: 0; left: 0; right: 0;
  height: 2px;
  background: linear-gradient(90deg, var(--accent2), var(--accent));
  border-radius: 8px 8px 0 0;
}

.login-logo {
  font-family: 'IBM Plex Mono', monospace;
  font-size: 0.7rem;
  letter-spacing: 0.3em;
  color: var(--accent);
  text-transform: uppercase;
  margin-bottom: 0.5rem;
}

.login-title {
  font-family: 'IBM Plex Sans', sans-serif;
  font-size: 1.6rem;
  font-weight: 600;
  color: var(--text);
  margin-bottom: 0.4rem;
}

.login-subtitle {
  font-size: 0.85rem;
  color: var(--muted);
  margin-bottom: 2rem;
}

/* ── FORMS ── */
.form-group {
  margin-bottom: 1.25rem;
}

label {
  display: block;
  font-family: 'IBM Plex Mono', monospace;
  font-size: 0.7rem;
  font-weight: 600;
  letter-spacing: 0.12em;
  color: var(--label);
  text-transform: uppercase;
  margin-bottom: 0.45rem;
}

input[type=text],
input[type=password],
select {
  width: 100%;
  padding: 0.65rem 0.9rem;
  background: var(--surface);
  border: 1px solid var(--border);
  border-radius: 5px;
  color: var(--text);
  font-family: 'IBM Plex Mono', monospace;
  font-size: 0.9rem;
  outline: none;
  transition: border-color 0.2s, box-shadow 0.2s;
  -webkit-appearance: none;
}

input:focus, select:focus {
  border-color: var(--accent);
  box-shadow: 0 0 0 3px rgba(0,229,255,0.08);
}

select option {
  background: var(--surface);
}

.btn-primary {
  width: 100%;
  padding: 0.75rem;
  background: linear-gradient(135deg, var(--accent2), var(--accent));
  color: #fff;
  font-family: 'IBM Plex Mono', monospace;
  font-size: 0.85rem;
  font-weight: 600;
  letter-spacing: 0.1em;
  text-transform: uppercase;
  border: none;
  border-radius: 5px;
  cursor: pointer;
  transition: opacity 0.2s, transform 0.1s;
  margin-top: 0.5rem;
}
.btn-primary:hover { opacity: 0.9; }
.btn-primary:active { transform: scale(0.99); }

.btn-search {
  padding: 0.65rem 1.5rem;
  background: rgba(0,229,255,0.1);
  color: var(--accent);
  font-family: 'IBM Plex Mono', monospace;
  font-size: 0.8rem;
  font-weight: 600;
  letter-spacing: 0.1em;
  text-transform: uppercase;
  border: 1px solid rgba(0,229,255,0.35);
  border-radius: 5px;
  cursor: pointer;
  transition: all 0.2s;
  white-space: nowrap;
}
.btn-search:hover { background: rgba(0,229,255,0.18); border-color: var(--accent); }

.input-row {
  display: flex;
  gap: 0.75rem;
  align-items: flex-end;
}
.input-row .form-group { flex: 1; margin-bottom: 0; }

/* ── ALERTS ── */
.alert {
  padding: 0.8rem 1rem;
  border-radius: 5px;
  font-size: 0.85rem;
  margin-bottom: 1.5rem;
  display: flex;
  align-items: center;
  gap: 0.5rem;
}
.alert-error {
  background: rgba(255,77,109,0.1);
  border: 1px solid rgba(255,77,109,0.3);
  color: #ff8fa3;
}
.alert-icon { font-size: 1rem; }

/* ── SECTION HEADER ── */
.section-header {
  margin-bottom: 1.75rem;
}
.section-eyebrow {
  font-family: 'IBM Plex Mono', monospace;
  font-size: 0.68rem;
  letter-spacing: 0.22em;
  color: var(--accent);
  text-transform: uppercase;
  margin-bottom: 0.35rem;
}
.section-title {
  font-size: 1.5rem;
  font-weight: 600;
  color: var(--text);
}
.section-desc {
  font-size: 0.85rem;
  color: var(--muted);
  margin-top: 0.3rem;
}

/* ── SEARCH CARD ── */
.search-card {
  background: var(--card);
  border: 1px solid var(--border);
  border-radius: 8px;
  padding: 1.75rem;
  margin-bottom: 2rem;
}

.search-card h3 {
  font-family: 'IBM Plex Mono', monospace;
  font-size: 0.78rem;
  font-weight: 600;
  letter-spacing: 0.15em;
  color: var(--accent2);
  text-transform: uppercase;
  margin-bottom: 1.25rem;
}

/* ── EMPLOYEE RESULT CARD ── */
.result-card {
  background: var(--card);
  border: 1px solid var(--border);
  border-radius: 8px;
  overflow: hidden;
  animation: fadeSlideUp 0.35s ease;
}

.result-header {
  padding: 1.2rem 1.75rem;
  background: linear-gradient(135deg, rgba(0,229,255,0.06), rgba(123,97,255,0.06));
  border-bottom: 1px solid var(--border);
  display: flex;
  align-items: center;
  gap: 1rem;
}

.result-avatar {
  width: 46px; height: 46px;
  background: linear-gradient(135deg, var(--accent2), var(--accent));
  border-radius: 8px;
  display: flex;
  align-items: center;
  justify-content: center;
  font-weight: 700;
  font-size: 1.1rem;
  color: #fff;
  flex-shrink: 0;
}

.result-name {
  font-size: 1.1rem;
  font-weight: 600;
  color: var(--text);
}

.result-id {
  font-family: 'IBM Plex Mono', monospace;
  font-size: 0.75rem;
  color: var(--muted);
}

.result-grid {
  display: grid;
  grid-template-columns: repeat(auto-fill, minmax(180px, 1fr));
  gap: 0;
}

.result-field {
  padding: 1.2rem 1.75rem;
  border-bottom: 1px solid var(--border);
  border-right: 1px solid var(--border);
}

.result-field:nth-child(even) { border-right: none; }

.result-field-label {
  font-family: 'IBM Plex Mono', monospace;
  font-size: 0.65rem;
  font-weight: 600;
  letter-spacing: 0.15em;
  color: var(--muted);
  text-transform: uppercase;
  margin-bottom: 0.3rem;
}

.result-field-value {
  font-size: 1rem;
  font-weight: 500;
  color: var(--text);
}

.result-field-value.salary {
  font-family: 'IBM Plex Mono', monospace;
  color: var(--green);
  font-size: 1.05rem;
}

.result-field-value.ssn {
  font-family: 'IBM Plex Mono', monospace;
  font-size: 0.9rem;
  color: var(--label);
}

/* ── DEPARTMENT TABLE ── */
.dept-table-wrapper {
  background: var(--card);
  border: 1px solid var(--border);
  border-radius: 8px;
  overflow: hidden;
  animation: fadeSlideUp 0.35s ease;
}

.dept-table-header {
  padding: 1.2rem 1.75rem;
  background: linear-gradient(135deg, rgba(123,97,255,0.08), rgba(0,229,255,0.04));
  border-bottom: 1px solid var(--border);
  display: flex;
  align-items: center;
  justify-content: space-between;
}

.dept-table-title {
  font-family: 'IBM Plex Mono', monospace;
  font-size: 0.8rem;
  font-weight: 600;
  letter-spacing: 0.12em;
  color: var(--accent2);
  text-transform: uppercase;
}

.dept-summary {
  font-size: 0.8rem;
  color: var(--muted);
}

table {
  width: 100%;
  border-collapse: collapse;
}

thead th {
  font-family: 'IBM Plex Mono', monospace;
  font-size: 0.68rem;
  font-weight: 600;
  letter-spacing: 0.14em;
  color: var(--muted);
  text-transform: uppercase;
  padding: 0.8rem 1.5rem;
  text-align: left;
  background: rgba(255,255,255,0.015);
  border-bottom: 1px solid var(--border);
}

tbody tr {
  border-bottom: 1px solid rgba(31,37,53,0.7);
  transition: background 0.15s;
}
tbody tr:last-child { border-bottom: none; }
tbody tr:hover { background: rgba(0,229,255,0.03); }

tbody td {
  padding: 1rem 1.5rem;
  font-size: 0.9rem;
  color: var(--text);
}

.td-mono {
  font-family: 'IBM Plex Mono', monospace;
  font-size: 0.82rem;
  color: var(--label);
}

.td-salary {
  font-family: 'IBM Plex Mono', monospace;
  font-size: 0.9rem;
  color: var(--green);
  font-weight: 500;
}

.td-total {
  font-family: 'IBM Plex Mono', monospace;
  font-size: 0.88rem;
  color: var(--accent);
  font-weight: 600;
}

tfoot td {
  padding: 0.9rem 1.5rem;
  background: rgba(0,229,255,0.03);
  border-top: 1px solid var(--border);
}

/* ── NAV TABS ── */
.nav-tabs {
  display: flex;
  gap: 0.5rem;
  margin-bottom: 2rem;
  border-bottom: 1px solid var(--border);
  padding-bottom: 0;
}

.nav-tab {
  font-family: 'IBM Plex Mono', monospace;
  font-size: 0.75rem;
  font-weight: 600;
  letter-spacing: 0.1em;
  text-transform: uppercase;
  padding: 0.6rem 1.2rem;
  color: var(--muted);
  text-decoration: none;
  border-bottom: 2px solid transparent;
  margin-bottom: -1px;
  transition: color 0.2s, border-color 0.2s;
}

.nav-tab:hover { color: var(--label); }
.nav-tab.active { color: var(--accent); border-bottom-color: var(--accent); }

/* ── DIVIDER ── */
.divider { height: 1px; background: var(--border); margin: 2rem 0; }

/* ── ANIMATIONS ── */
@keyframes fadeSlideUp {
  from { opacity: 0; transform: translateY(14px); }
  to   { opacity: 1; transform: translateY(0); }
}

/* ── RESPONSIVE ── */
@media (max-width: 600px) {
  .main { padding: 1.5rem 1rem; }
  .topbar { padding: 0.75rem 1rem; }
  .topbar-user { display: none; }
  .result-grid { grid-template-columns: 1fr; }
  .result-field { border-right: none !important; }
  .input-row { flex-direction: column; }
  .btn-search { width: 100%; }
  thead th, tbody td, tfoot td { padding: 0.75rem 1rem; }
}
CSS;

?>
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>HR Portal<?= isLoggedIn() ? ' — ' . h($_SESSION['username']) : '' ?></title>
<style><?= $css ?></style>
</head>
<body>
<div class="page-wrapper">

<?php if (!isLoggedIn()): ?>
<!-- ════════════════════════════════ LOGIN PAGE ════════════════════════════════ -->
<div class="login-container">
  <div class="login-card">
    <div class="login-logo">&#9632; HR Portal v2</div>
    <h1 class="login-title">Employee Access</h1>
    <p class="login-subtitle">Sign in with your credentials to continue.</p>

    <?php if ($error): ?>
    <div class="alert alert-error"><span class="alert-icon">⚠</span><?= h($error) ?></div>
    <?php endif; ?>

    <form method="POST" action="<?= h($_SERVER['PHP_SELF']) ?>">
      <input type="hidden" name="action" value="login">
      <div class="form-group">
        <label for="username">Username</label>
        <input type="text" id="username" name="username" autocomplete="username" autofocus
               value="<?= h($_POST['username'] ?? '') ?>" placeholder="Enter your username">
      </div>
      <div class="form-group">
        <label for="password">Password</label>
        <input type="password" id="password" name="password" autocomplete="current-password" placeholder="••••••••">
      </div>
      <button type="submit" class="btn-primary">Sign In &rarr;</button>
    </form>
  </div>
</div>

<?php else: ?>
<!-- ════════════════════════════════ APP SHELL ════════════════════════════════ -->
<header class="topbar">
  <div class="topbar-brand">&#9632; HR<span>&nbsp;Portal</span></div>
  <div class="topbar-right">
    <span class="topbar-user"><?= h($_SESSION['username']) ?></span>
    <span class="badge-role <?= h($_SESSION['role']) ?>"><?= h($_SESSION['role']) ?></span>
    <a class="btn-logout" href="<?= h($_SERVER['PHP_SELF']) ?>?action=logout">Logout</a>
  </div>
</header>

<main class="main">

  <?php
  // ── NAV TABS ──
  $tab = (in_array($action, ['search_employee']) || ($action === 'dashboard' && !isManager()))
       ? 'employee'
       : (in_array($action, ['search_department']) ? 'department' : 'employee');
  ?>
  <nav class="nav-tabs">
    <a class="nav-tab <?= $tab === 'employee' ? 'active' : '' ?>"
       href="<?= h($_SERVER['PHP_SELF']) ?>?action=search_employee">
      Employee Lookup
    </a>
    <?php if (isManager()): ?>
    <a class="nav-tab <?= $tab === 'department' ? 'active' : '' ?>"
       href="<?= h($_SERVER['PHP_SELF']) ?>?action=search_department">
      Department Salaries
    </a>
    <?php endif; ?>
  </nav>

  <?php if ($error && $action !== 'login'): ?>
  <div class="alert alert-error"><span class="alert-icon">⚠</span><?= h($error) ?></div>
  <?php endif; ?>

  <!-- ──────────────────── EMPLOYEE LOOKUP ──────────────────── -->
  <?php if ($action === 'dashboard' || $action === 'search_employee'): ?>
  <div class="section-header">
    <div class="section-eyebrow">Secure Lookup</div>
    <h2 class="section-title">Employee Record</h2>
    <p class="section-desc">
      <?= isManager()
          ? 'Search any employee by their ID number to view their full record.'
          : 'Search using your employee ID to view your personal record.' ?>
    </p>
  </div>

  <div class="search-card">
    <h3>&#128269; Search by Employee ID</h3>
    <form method="POST" action="<?= h($_SERVER['PHP_SELF']) ?>">
      <input type="hidden" name="action" value="search_employee">
      <div class="input-row">
        <div class="form-group">
          <label for="emp_id">Employee ID</label>
          <input type="text" id="emp_id" name="emp_id"
                 placeholder="e.g. EMP001"
                 value="<?= h($data['search_emp_id'] ?? '') ?>">
        </div>
        <button type="submit" class="btn-search">Search</button>
      </div>
    </form>
    <?php if (!isManager()): ?>
      <p style="font-size:0.78rem;color:var(--muted);margin-top:0.85rem;">
        Your Employee ID: <code style="color:var(--accent);font-family:'IBM Plex Mono',monospace;"><?= h($_SESSION['emp_id'] ?? 'N/A') ?></code>
      </p>
    <?php endif; ?>
  </div>

  <?php if (!empty($data['employee'])): $emp = $data['employee']; ?>
  <div class="result-card">
    <div class="result-header">
      <div class="result-avatar"><?= strtoupper(substr($emp['name'], 0, 1)) ?></div>
      <div>
        <div class="result-name"><?= h($emp['name']) ?></div>
        <div class="result-id">ID: <?= h($emp['emp_id']) ?></div>
      </div>
    </div>
    <div class="result-grid">
      <div class="result-field">
        <div class="result-field-label">Full Name</div>
        <div class="result-field-value"><?= h($emp['name']) ?></div>
      </div>
      <div class="result-field">
        <div class="result-field-label">Employee ID</div>
        <div class="result-field-value" style="font-family:'IBM Plex Mono',monospace;"><?= h($emp['emp_id']) ?></div>
      </div>
      <div class="result-field">
        <div class="result-field-label">Department</div>
        <div class="result-field-value"><?= h($emp['department']) ?></div>
      </div>
      <div class="result-field">
        <div class="result-field-label">Annual Salary</div>
        <div class="result-field-value salary"><?= formatSalary((float)$emp['salary']) ?></div>
      </div>
      <div class="result-field" style="border-bottom:none;">
        <div class="result-field-label">SSN</div>
        <div class="result-field-value ssn"><?= h(maskSSN($emp['ssn'])) ?>
          <?php if (isManager()): ?>
          <span style="color:var(--muted);font-size:0.75rem;">&nbsp;(full: <?= h($emp['ssn']) ?>)</span>
          <?php endif; ?>
        </div>
      </div>
    </div>
  </div>
  <?php endif; ?>

  <?php endif; // end employee lookup ?>

  <!-- ──────────────────── DEPARTMENT VIEW (MANAGER) ──────────────────── -->
  <?php if ($action === 'search_department' && isManager()): ?>
  <div class="section-header">
    <div class="section-eyebrow">Manager View</div>
    <h2 class="section-title">Department Salaries</h2>
    <p class="section-desc">Select a department to view all employee salaries.</p>
  </div>

  <div class="search-card">
    <h3>&#127970; Search by Department</h3>
    <form method="POST" action="<?= h($_SERVER['PHP_SELF']) ?>">
      <input type="hidden" name="action" value="search_department">
      <div class="input-row">
        <div class="form-group">
          <label for="dept_id">Department</label>
          <select id="dept_id" name="dept_id">
            <option value="">— Select a department —</option>
            <?php foreach ($data['departments'] ?? [] as $dept): ?>
            <option value="<?= h($dept['id']) ?>" <?= ($data['selected_dept'] ?? 0) == $dept['id'] ? 'selected' : '' ?>>
              <?= h($dept['name']) ?>
            </option>
            <?php endforeach; ?>
          </select>
        </div>
        <button type="submit" class="btn-search">View</button>
      </div>
    </form>
  </div>

  <?php if (!empty($data['employees'])): ?>
  <?php
    $total = array_sum(array_column($data['employees'], 'salary'));
    $avg   = $total / count($data['employees']);
    $max   = max(array_column($data['employees'], 'salary'));
    $min   = min(array_column($data['employees'], 'salary'));
  ?>
  <div class="dept-table-wrapper">
    <div class="dept-table-header">
      <div class="dept-table-title">&#128202; <?= h($data['dept_name']) ?></div>
      <div class="dept-summary"><?= count($data['employees']) ?> employee<?= count($data['employees']) !== 1 ? 's' : '' ?></div>
    </div>
    <table>
      <thead>
        <tr>
          <th>#</th>
          <th>Employee ID</th>
          <th>Name</th>
          <th>Annual Salary</th>
        </tr>
      </thead>
      <tbody>
        <?php foreach ($data['employees'] as $i => $emp): ?>
        <tr>
          <td class="td-mono"><?= $i + 1 ?></td>
          <td class="td-mono"><?= h($emp['emp_id']) ?></td>
          <td><?= h($emp['name']) ?></td>
          <td class="td-salary"><?= formatSalary((float)$emp['salary']) ?></td>
        </tr>
        <?php endforeach; ?>
      </tbody>
      <tfoot>
        <tr>
          <td colspan="3" style="font-family:'IBM Plex Mono',monospace;font-size:0.72rem;color:var(--muted);letter-spacing:0.1em;">
            TOTAL &nbsp;|&nbsp; AVG: <span style="color:var(--label);"><?= formatSalary($avg) ?></span>
            &nbsp;|&nbsp; MAX: <span style="color:var(--label);"><?= formatSalary($max) ?></span>
            &nbsp;|&nbsp; MIN: <span style="color:var(--label);"><?= formatSalary($min) ?></span>
          </td>
          <td class="td-total"><?= formatSalary($total) ?></td>
        </tr>
      </tfoot>
    </table>
  </div>
  <?php endif; ?>
  <?php endif; // end department view ?>

</main>
<?php endif; // end app shell ?>

</div><!-- .page-wrapper -->
</body>
</html>