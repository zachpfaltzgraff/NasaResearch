<?php
/**
 * Employee Management System
 * Single-file PHP application with MySQL backend
 * Supports roles: employee, manager
 */

// ─────────────────────────────────────────────
//  CONFIGURATION — edit to match your environment
// ─────────────────────────────────────────────
define('DB_HOST', 'localhost');
define('DB_NAME', 'employee_mgmt');
define('DB_USER', 'root');
define('DB_PASS', '');
define('DB_PORT', '3306');

// ─────────────────────────────────────────────
//  DATABASE BOOTSTRAP
// ─────────────────────────────────────────────
function getDB(): PDO {
    static $pdo = null;
    if ($pdo === null) {
        $dsn = 'mysql:host=' . DB_HOST . ';port=' . DB_PORT . ';dbname=' . DB_NAME . ';charset=utf8mb4';
        $pdo = new PDO($dsn, DB_USER, DB_PASS, [
            PDO::ATTR_ERRMODE            => PDO::ERRMODE_EXCEPTION,
            PDO::ATTR_DEFAULT_FETCH_MODE => PDO::FETCH_ASSOC,
            PDO::ATTR_EMULATE_PREPARES   => false,
        ]);
    }
    return $pdo;
}

function bootstrapDB(): void {
    $db = getDB();

    $db->exec("
        CREATE TABLE IF NOT EXISTS employees (
            id          INT AUTO_INCREMENT PRIMARY KEY,
            name        VARCHAR(120)  NOT NULL,
            username    VARCHAR(60)   NOT NULL UNIQUE,
            password    VARCHAR(255)  NOT NULL,
            position    VARCHAR(120)  NOT NULL,
            department  VARCHAR(120)  NOT NULL DEFAULT 'General',
            role        ENUM('employee','manager') NOT NULL DEFAULT 'employee',
            email       VARCHAR(180)  NOT NULL DEFAULT '',
            created_at  DATETIME      NOT NULL DEFAULT CURRENT_TIMESTAMP
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
    ");

    // Seed default accounts if table is empty
    $count = $db->query("SELECT COUNT(*) FROM employees")->fetchColumn();
    if ((int)$count === 0) {
        $seeds = [
            ['Alice Johnson',  'alice',   'alice123',   'Software Engineer',    'Engineering',  'employee', 'alice@company.com'],
            ['Bob Martinez',   'bob',     'bob123',     'Product Designer',     'Design',       'employee', 'bob@company.com'],
            ['Carol Williams', 'carol',   'carol123',   'Data Analyst',         'Analytics',    'employee', 'carol@company.com'],
            ['David Chen',     'david',   'david123',   'DevOps Engineer',      'Engineering',  'employee', 'david@company.com'],
            ['Eve Thompson',   'eve',     'eve123',     'Engineering Manager',  'Engineering',  'manager',  'eve@company.com'],
            ['Frank Harris',   'frank',   'frank123',   'HR Director',          'HR',           'manager',  'frank@company.com'],
        ];

        $stmt = $db->prepare("
            INSERT INTO employees (name, username, password, position, department, role, email)
            VALUES (?, ?, ?, ?, ?, ?, ?)
        ");
        foreach ($seeds as $s) {
            $s[2] = password_hash($s[2], PASSWORD_BCRYPT);
            $stmt->execute($s);
        }
    }
}

// ─────────────────────────────────────────────
//  SESSION HELPERS
// ─────────────────────────────────────────────
session_start();

function currentUser(): ?array {
    return $_SESSION['user'] ?? null;
}

function requireLogin(): void {
    if (!currentUser()) {
        redirect('login');
    }
}

function requireManager(): void {
    requireLogin();
    if (currentUser()['role'] !== 'manager') {
        redirect('dashboard');
    }
}

function redirect(string $page, string $msg = '', string $type = 'error'): void {
    $url = '?page=' . urlencode($page);
    if ($msg) {
        $_SESSION['flash'] = ['msg' => $msg, 'type' => $type];
    }
    header('Location: ' . $url);
    exit;
}

function flash(): ?array {
    $f = $_SESSION['flash'] ?? null;
    unset($_SESSION['flash']);
    return $f;
}

function csrf(): string {
    if (empty($_SESSION['csrf'])) {
        $_SESSION['csrf'] = bin2hex(random_bytes(32));
    }
    return $_SESSION['csrf'];
}

function verifyCsrf(): void {
    $token = $_POST['_csrf'] ?? '';
    if (!hash_equals($_SESSION['csrf'] ?? '', $token)) {
        die('CSRF validation failed.');
    }
}

// ─────────────────────────────────────────────
//  CONTROLLERS / ACTIONS
// ─────────────────────────────────────────────
function actionLogin(): void {
    verifyCsrf();
    $username = trim($_POST['username'] ?? '');
    $password = $_POST['password'] ?? '';

    if (!$username || !$password) {
        redirect('login', 'Please fill in all fields.');
        return;
    }

    $db   = getDB();
    $stmt = $db->prepare("SELECT * FROM employees WHERE username = ? LIMIT 1");
    $stmt->execute([$username]);
    $user = $stmt->fetch();

    if (!$user || !password_verify($password, $user['password'])) {
        redirect('login', 'Invalid username or password.');
        return;
    }

    unset($user['password']);
    $_SESSION['user'] = $user;
    redirect('dashboard', 'Welcome back, ' . htmlspecialchars($user['name']) . '!', 'success');
}

function actionLogout(): void {
    session_destroy();
    header('Location: ?page=login');
    exit;
}

function actionUpdateRole(): void {
    requireManager();
    verifyCsrf();

    $empId  = (int)($_POST['employee_id'] ?? 0);
    $role   = $_POST['role'] ?? '';
    $me     = currentUser();

    if (!in_array($role, ['employee', 'manager'], true)) {
        redirect('employees', 'Invalid role selected.');
        return;
    }
    if ($empId === (int)$me['id']) {
        redirect('employees', 'You cannot change your own role.');
        return;
    }

    $db   = getDB();
    $stmt = $db->prepare("UPDATE employees SET role = ? WHERE id = ?");
    $stmt->execute([$role, $empId]);

    redirect('employees', 'Role updated successfully.', 'success');
}

function actionUpdateProfile(): void {
    requireLogin();
    verifyCsrf();

    $me     = currentUser();
    $name   = trim($_POST['name'] ?? '');
    $email  = trim($_POST['email'] ?? '');

    if (!$name) {
        redirect('profile', 'Name cannot be empty.');
        return;
    }

    $db = getDB();
    $db->prepare("UPDATE employees SET name = ?, email = ? WHERE id = ?")
       ->execute([$name, $email, $me['id']]);

    // Refresh session
    $updated = $db->prepare("SELECT * FROM employees WHERE id = ?");
    $updated->execute([$me['id']]);
    $row = $updated->fetch();
    unset($row['password']);
    $_SESSION['user'] = $row;

    redirect('profile', 'Profile updated successfully.', 'success');
}

// ─────────────────────────────────────────────
//  PAGE RENDERERS
// ─────────────────────────────────────────────
function pageDashboard(): void {
    requireLogin();
    $user = currentUser();
    $db   = getDB();

    $totalEmp  = $db->query("SELECT COUNT(*) FROM employees WHERE role='employee'")->fetchColumn();
    $totalMgr  = $db->query("SELECT COUNT(*) FROM employees WHERE role='manager'")->fetchColumn();
    $deptCount = $db->query("SELECT COUNT(DISTINCT department) FROM employees")->fetchColumn();

    layout('Dashboard', function() use ($user, $totalEmp, $totalMgr, $deptCount) { ?>
        <div class="page-header">
            <div>
                <h1 class="page-title">Dashboard</h1>
                <p class="page-sub">Good to see you, <strong><?= h($user['name']) ?></strong></p>
            </div>
            <span class="badge badge-<?= $user['role'] ?>"><?= ucfirst($user['role']) ?></span>
        </div>

        <div class="stats-grid">
            <div class="stat-card">
                <div class="stat-icon">👥</div>
                <div class="stat-body">
                    <div class="stat-value"><?= $totalEmp ?></div>
                    <div class="stat-label">Employees</div>
                </div>
            </div>
            <div class="stat-card">
                <div class="stat-icon">🏅</div>
                <div class="stat-body">
                    <div class="stat-value"><?= $totalMgr ?></div>
                    <div class="stat-label">Managers</div>
                </div>
            </div>
            <div class="stat-card">
                <div class="stat-icon">🏢</div>
                <div class="stat-body">
                    <div class="stat-value"><?= $deptCount ?></div>
                    <div class="stat-label">Departments</div>
                </div>
            </div>
            <div class="stat-card">
                <div class="stat-icon">🔐</div>
                <div class="stat-body">
                    <div class="stat-value"><?= ucfirst($user['role']) ?></div>
                    <div class="stat-label">Your Access Level</div>
                </div>
            </div>
        </div>

        <div class="info-cards">
            <div class="info-card">
                <h3>Your Quick Info</h3>
                <table class="detail-table">
                    <tr><th>ID</th><td>#<?= h($user['id']) ?></td></tr>
                    <tr><th>Position</th><td><?= h($user['position']) ?></td></tr>
                    <tr><th>Department</th><td><?= h($user['department']) ?></td></tr>
                    <tr><th>Role</th><td><span class="badge badge-<?= $user['role'] ?>"><?= ucfirst($user['role']) ?></span></td></tr>
                </table>
                <a href="?page=profile" class="btn btn-secondary mt-1">View Full Profile →</a>
            </div>

            <?php if ($user['role'] === 'manager'): ?>
            <div class="info-card highlight">
                <h3>Manager Actions</h3>
                <p>As a manager you can view all employees, update their roles, and monitor the team.</p>
                <a href="?page=employees" class="btn btn-primary mt-1">Manage Employees →</a>
            </div>
            <?php else: ?>
            <div class="info-card">
                <h3>Your Access</h3>
                <p>As an employee you can view your own profile and keep your contact info up to date.</p>
                <a href="?page=profile" class="btn btn-primary mt-1">Edit Profile →</a>
            </div>
            <?php endif; ?>
        </div>
    <?php });
}

function pageProfile(): void {
    requireLogin();
    $user = currentUser();
    layout('My Profile', function() use ($user) { ?>
        <div class="page-header">
            <div>
                <h1 class="page-title">My Profile</h1>
                <p class="page-sub">Your personal information</p>
            </div>
            <span class="badge badge-<?= $user['role'] ?>"><?= ucfirst($user['role']) ?></span>
        </div>

        <div class="profile-layout">
            <div class="profile-card">
                <div class="avatar-lg"><?= strtoupper(substr($user['name'], 0, 2)) ?></div>
                <div class="profile-name"><?= h($user['name']) ?></div>
                <div class="profile-position"><?= h($user['position']) ?></div>
                <span class="badge badge-<?= $user['role'] ?> mt-1"><?= ucfirst($user['role']) ?></span>

                <table class="detail-table mt-2">
                    <tr><th>Employee ID</th><td>#<?= h($user['id']) ?></td></tr>
                    <tr><th>Department</th><td><?= h($user['department']) ?></td></tr>
                    <tr><th>Username</th><td><?= h($user['username']) ?></td></tr>
                    <tr><th>Email</th><td><?= h($user['email'] ?: '—') ?></td></tr>
                    <tr><th>Member Since</th><td><?= date('M j, Y', strtotime($user['created_at'])) ?></td></tr>
                </table>
            </div>

            <div class="form-card">
                <h3>Edit Information</h3>
                <p class="form-note">Update your display name and contact email.</p>
                <form method="POST" action="?action=update_profile">
                    <input type="hidden" name="_csrf" value="<?= csrf() ?>">
                    <div class="form-group">
                        <label>Full Name</label>
                        <input type="text" name="name" value="<?= h($user['name']) ?>" required>
                    </div>
                    <div class="form-group">
                        <label>Email Address</label>
                        <input type="email" name="email" value="<?= h($user['email']) ?>" placeholder="you@company.com">
                    </div>
                    <button type="submit" class="btn btn-primary">Save Changes</button>
                </form>
            </div>
        </div>
    <?php });
}

function pageEmployees(): void {
    requireManager();
    $db = getDB();
    $employees = $db->query("SELECT * FROM employees ORDER BY name ASC")->fetchAll();
    $me = currentUser();

    layout('Employees', function() use ($employees, $me) { ?>
        <div class="page-header">
            <div>
                <h1 class="page-title">All Employees</h1>
                <p class="page-sub"><?= count($employees) ?> people in the system</p>
            </div>
            <span class="badge badge-manager">Manager View</span>
        </div>

        <div class="table-wrapper">
            <table class="emp-table">
                <thead>
                    <tr>
                        <th>Employee</th>
                        <th>ID</th>
                        <th>Position</th>
                        <th>Department</th>
                        <th>Email</th>
                        <th>Role</th>
                        <th>Action</th>
                    </tr>
                </thead>
                <tbody>
                <?php foreach ($employees as $emp): ?>
                    <tr <?= $emp['id'] == $me['id'] ? 'class="row-self"' : '' ?>>
                        <td>
                            <div class="emp-name-cell">
                                <span class="avatar-sm"><?= strtoupper(substr($emp['name'], 0, 2)) ?></span>
                                <span><?= h($emp['name']) ?>
                                    <?php if ($emp['id'] == $me['id']): ?>
                                        <span class="you-tag">You</span>
                                    <?php endif; ?>
                                </span>
                            </div>
                        </td>
                        <td class="muted">#<?= h($emp['id']) ?></td>
                        <td><?= h($emp['position']) ?></td>
                        <td><?= h($emp['department']) ?></td>
                        <td class="muted"><?= h($emp['email'] ?: '—') ?></td>
                        <td><span class="badge badge-<?= $emp['role'] ?>"><?= ucfirst($emp['role']) ?></span></td>
                        <td>
                            <?php if ($emp['id'] != $me['id']): ?>
                            <form method="POST" action="?action=update_role" class="role-form">
                                <input type="hidden" name="_csrf" value="<?= csrf() ?>">
                                <input type="hidden" name="employee_id" value="<?= $emp['id'] ?>">
                                <select name="role" class="role-select" data-current="<?= $emp['role'] ?>">
                                    <option value="employee" <?= $emp['role']==='employee'?'selected':'' ?>>Employee</option>
                                    <option value="manager"  <?= $emp['role']==='manager' ?'selected':'' ?>>Manager</option>
                                </select>
                                <button type="submit" class="btn btn-sm btn-primary">Update</button>
                            </form>
                            <?php else: ?>
                                <span class="muted">—</span>
                            <?php endif; ?>
                        </td>
                    </tr>
                <?php endforeach; ?>
                </tbody>
            </table>
        </div>
    <?php });
}

function pageLogin(): void {
    if (currentUser()) redirect('dashboard');
    $flash = flash(); ?>
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Sign In — EMS</title>
<?= commonStyles() ?>
<style>
body { background: var(--bg-deep); display: flex; align-items: center; justify-content: center; min-height: 100vh; }
.login-wrap { width: 100%; max-width: 420px; padding: 1rem; }
.login-card { background: var(--bg-card); border: 1px solid var(--border); border-radius: 16px; padding: 2.5rem; box-shadow: 0 24px 64px rgba(0,0,0,.35); }
.login-logo { text-align: center; margin-bottom: 2rem; }
.login-logo .logo-mark { width: 56px; height: 56px; background: var(--accent); border-radius: 14px; display: inline-flex; align-items: center; justify-content: center; font-size: 1.6rem; margin-bottom: .75rem; box-shadow: 0 8px 24px rgba(99,102,241,.4); }
.login-logo h1 { font-size: 1.4rem; font-weight: 700; color: var(--text-primary); letter-spacing: -.02em; }
.login-logo p  { font-size: .85rem; color: var(--text-muted); margin-top: .25rem; }
.demo-grid { display: grid; grid-template-columns: 1fr 1fr; gap: .5rem; margin-bottom: 1.5rem; }
.demo-chip { background: var(--bg-deep); border: 1px solid var(--border); border-radius: 8px; padding: .6rem .75rem; font-size: .78rem; cursor: pointer; transition: border-color .15s; }
.demo-chip:hover { border-color: var(--accent); }
.demo-chip strong { display: block; color: var(--text-primary); margin-bottom: .1rem; }
.demo-chip span { color: var(--text-muted); }
.demo-label { font-size: .75rem; color: var(--text-muted); margin-bottom: .5rem; text-transform: uppercase; letter-spacing: .08em; }
</style>
</head>
<body>
<div class="login-wrap">
    <div class="login-card">
        <div class="login-logo">
            <div class="logo-mark">🏢</div>
            <h1>Employee Management</h1>
            <p>Sign in to your account</p>
        </div>

        <?php if ($flash): ?>
            <div class="alert alert-<?= $flash['type'] ?>"><?= h($flash['msg']) ?></div>
        <?php endif; ?>

        <div class="demo-label">Quick demo login</div>
        <div class="demo-grid">
            <div class="demo-chip" onclick="fillLogin('alice','alice123')">
                <strong>Alice Johnson</strong><span>Employee · Engineering</span>
            </div>
            <div class="demo-chip" onclick="fillLogin('eve','eve123')">
                <strong>Eve Thompson</strong><span>Manager · Engineering</span>
            </div>
        </div>

        <form method="POST" action="?action=login">
            <input type="hidden" name="_csrf" value="<?= csrf() ?>">
            <div class="form-group">
                <label>Username</label>
                <input type="text" name="username" id="username" autocomplete="username" required placeholder="your.username">
            </div>
            <div class="form-group">
                <label>Password</label>
                <input type="password" name="password" id="password" autocomplete="current-password" required placeholder="••••••••">
            </div>
            <button type="submit" class="btn btn-primary" style="width:100%;margin-top:.5rem">Sign In</button>
        </form>
    </div>
    <p style="text-align:center;margin-top:1rem;font-size:.8rem;color:var(--text-muted)">
        Employee Management System &copy; <?= date('Y') ?>
    </p>
</div>
<script>
function fillLogin(u, p) {
    document.getElementById('username').value = u;
    document.getElementById('password').value = p;
}
</script>
</body>
</html>
<?php }

// ─────────────────────────────────────────────
//  SHARED LAYOUT
// ─────────────────────────────────────────────
function layout(string $title, callable $body): void {
    $user  = currentUser();
    $flash = flash();
    $page  = $_GET['page'] ?? 'dashboard'; ?>
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title><?= h($title) ?> — EMS</title>
<?= commonStyles() ?>
<style>
/* ── Layout Shell ── */
body { display: flex; min-height: 100vh; background: var(--bg-deep); }

.sidebar {
    width: 240px; flex-shrink: 0;
    background: var(--bg-card);
    border-right: 1px solid var(--border);
    display: flex; flex-direction: column;
    position: fixed; top: 0; left: 0; height: 100vh;
    z-index: 100;
}
.sidebar-logo {
    padding: 1.5rem 1.25rem 1rem;
    border-bottom: 1px solid var(--border);
}
.sidebar-logo .logo-mark {
    width: 38px; height: 38px; background: var(--accent);
    border-radius: 10px; display: inline-flex;
    align-items: center; justify-content: center;
    font-size: 1.1rem; margin-bottom: .5rem;
    box-shadow: 0 4px 12px rgba(99,102,241,.4);
}
.sidebar-logo h2 { font-size: .95rem; font-weight: 700; letter-spacing: -.01em; color: var(--text-primary); line-height: 1.2; }
.sidebar-logo p  { font-size: .75rem; color: var(--text-muted); }

.nav { flex: 1; padding: 1rem 0; }
.nav a {
    display: flex; align-items: center; gap: .65rem;
    padding: .65rem 1.25rem;
    font-size: .88rem; font-weight: 500;
    color: var(--text-muted); text-decoration: none;
    border-radius: 0; transition: background .15s, color .15s;
    position: relative;
}
.nav a:hover { background: var(--bg-hover); color: var(--text-primary); }
.nav a.active { color: var(--accent-light); background: rgba(99,102,241,.12); }
.nav a.active::before {
    content: ''; position: absolute; left: 0; top: 0; bottom: 0;
    width: 3px; background: var(--accent); border-radius: 0 2px 2px 0;
}
.nav .nav-icon { font-size: 1rem; width: 20px; text-align: center; }
.nav-section { padding: .25rem 1.25rem .1rem; font-size: .7rem; text-transform: uppercase;
    letter-spacing: .1em; color: var(--text-muted); margin-top: .5rem; }

.sidebar-user {
    padding: 1rem 1.25rem;
    border-top: 1px solid var(--border);
    display: flex; align-items: center; gap: .75rem;
}
.sidebar-user .avatar-sm { flex-shrink: 0; }
.sidebar-user-info { flex: 1; min-width: 0; }
.sidebar-user-name { font-size: .82rem; font-weight: 600; color: var(--text-primary); white-space: nowrap; overflow: hidden; text-overflow: ellipsis; }
.sidebar-user-role { font-size: .72rem; color: var(--text-muted); }
.logout-btn {
    flex-shrink: 0; background: none; border: 1px solid var(--border);
    border-radius: 6px; padding: .3rem .5rem; cursor: pointer;
    color: var(--text-muted); font-size: .75rem; transition: all .15s;
}
.logout-btn:hover { border-color: #ef4444; color: #ef4444; }

.main-content {
    margin-left: 240px; flex: 1; padding: 2rem 2.5rem;
    max-width: calc(100vw - 240px);
}

/* ── Topbar Flash ── */
.topbar-flash { margin-bottom: 1.5rem; }

/* ── Stats Grid ── */
.stats-grid { display: grid; grid-template-columns: repeat(auto-fill, minmax(180px, 1fr)); gap: 1rem; margin-bottom: 1.5rem; }
.stat-card {
    background: var(--bg-card); border: 1px solid var(--border);
    border-radius: 12px; padding: 1.25rem;
    display: flex; align-items: center; gap: 1rem;
    transition: border-color .15s;
}
.stat-card:hover { border-color: var(--accent); }
.stat-icon { font-size: 1.75rem; }
.stat-value { font-size: 1.6rem; font-weight: 700; color: var(--text-primary); line-height: 1; }
.stat-label { font-size: .78rem; color: var(--text-muted); margin-top: .2rem; }

/* ── Info Cards ── */
.info-cards { display: grid; grid-template-columns: repeat(auto-fill, minmax(300px, 1fr)); gap: 1.25rem; }
.info-card {
    background: var(--bg-card); border: 1px solid var(--border);
    border-radius: 12px; padding: 1.5rem;
}
.info-card.highlight { border-color: var(--accent); background: rgba(99,102,241,.06); }
.info-card h3 { font-size: 1rem; font-weight: 600; margin-bottom: .5rem; color: var(--text-primary); }
.info-card p  { font-size: .88rem; color: var(--text-muted); line-height: 1.5; }

/* ── Profile Layout ── */
.profile-layout { display: grid; grid-template-columns: 280px 1fr; gap: 1.5rem; align-items: start; }
.profile-card {
    background: var(--bg-card); border: 1px solid var(--border);
    border-radius: 14px; padding: 2rem; text-align: center;
}
.avatar-lg {
    width: 72px; height: 72px; background: linear-gradient(135deg, var(--accent), #8b5cf6);
    border-radius: 50%; display: inline-flex; align-items: center;
    justify-content: center; font-size: 1.4rem; font-weight: 700;
    color: #fff; margin: 0 auto .75rem; box-shadow: 0 8px 24px rgba(99,102,241,.3);
}
.profile-name     { font-size: 1.15rem; font-weight: 700; color: var(--text-primary); }
.profile-position { font-size: .85rem; color: var(--text-muted); margin-top: .2rem; }
.form-card {
    background: var(--bg-card); border: 1px solid var(--border);
    border-radius: 14px; padding: 2rem;
}
.form-card h3 { font-size: 1.05rem; font-weight: 600; margin-bottom: .25rem; color: var(--text-primary); }
.form-note { font-size: .85rem; color: var(--text-muted); margin-bottom: 1.5rem; }

/* ── Employee Table ── */
.table-wrapper {
    background: var(--bg-card); border: 1px solid var(--border);
    border-radius: 14px; overflow: hidden;
}
.emp-table { width: 100%; border-collapse: collapse; font-size: .88rem; }
.emp-table th {
    background: var(--bg-deep); padding: .85rem 1rem;
    text-align: left; font-size: .75rem; font-weight: 600;
    text-transform: uppercase; letter-spacing: .08em; color: var(--text-muted);
    border-bottom: 1px solid var(--border);
}
.emp-table td { padding: .9rem 1rem; border-bottom: 1px solid var(--border-light); vertical-align: middle; }
.emp-table tr:last-child td { border-bottom: none; }
.emp-table tr:hover td { background: var(--bg-hover); }
.emp-table tr.row-self td { background: rgba(99,102,241,.05); }
.emp-name-cell { display: flex; align-items: center; gap: .65rem; }
.you-tag { font-size: .68rem; background: rgba(99,102,241,.15); color: var(--accent-light); border-radius: 4px; padding: .1rem .4rem; font-weight: 600; }
.role-form { display: flex; gap: .5rem; align-items: center; }
.role-select {
    background: var(--bg-deep); border: 1px solid var(--border);
    border-radius: 6px; color: var(--text-primary); padding: .3rem .5rem;
    font-size: .82rem; cursor: pointer;
}
</style>
</head>
<body>

<!-- Sidebar -->
<aside class="sidebar">
    <div class="sidebar-logo">
        <div class="logo-mark">🏢</div>
        <h2>Employee<br>Management</h2>
    </div>

    <nav class="nav">
        <div class="nav-section">Main</div>
        <a href="?page=dashboard" class="<?= $page==='dashboard'?'active':'' ?>">
            <span class="nav-icon">📊</span> Dashboard
        </a>
        <a href="?page=profile" class="<?= $page==='profile'?'active':'' ?>">
            <span class="nav-icon">👤</span> My Profile
        </a>

        <?php if ($user && $user['role'] === 'manager'): ?>
        <div class="nav-section">Management</div>
        <a href="?page=employees" class="<?= $page==='employees'?'active':'' ?>">
            <span class="nav-icon">👥</span> All Employees
        </a>
        <?php endif; ?>
    </nav>

    <div class="sidebar-user">
        <div class="avatar-sm"><?= strtoupper(substr($user['name'], 0, 2)) ?></div>
        <div class="sidebar-user-info">
            <div class="sidebar-user-name"><?= h($user['name']) ?></div>
            <div class="sidebar-user-role"><?= ucfirst($user['role']) ?></div>
        </div>
        <form method="POST" action="?action=logout">
            <input type="hidden" name="_csrf" value="<?= csrf() ?>">
            <button class="logout-btn" title="Sign out">⏻</button>
        </form>
    </div>
</aside>

<!-- Main -->
<main class="main-content">
    <?php if ($flash): ?>
    <div class="topbar-flash">
        <div class="alert alert-<?= $flash['type'] ?>"><?= h($flash['msg']) ?></div>
    </div>
    <?php endif; ?>

    <?php $body(); ?>
</main>
</body>
</html>
<?php }

// ─────────────────────────────────────────────
//  SHARED STYLES
// ─────────────────────────────────────────────
function commonStyles(): string { return <<<CSS
<link rel="preconnect" href="https://fonts.googleapis.com">
<link href="https://fonts.googleapis.com/css2?family=Sora:wght@400;500;600;700&family=JetBrains+Mono:wght@400;500&display=swap" rel="stylesheet">
<style>
*, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }

:root {
    --bg-deep:      #0d0f14;
    --bg-card:      #141720;
    --bg-hover:     #1a1e2a;
    --border:       #252a38;
    --border-light: #1e2330;
    --accent:       #6366f1;
    --accent-light: #818cf8;
    --text-primary: #e8eaf0;
    --text-muted:   #6b7280;
    --success:      #22c55e;
    --error:        #ef4444;
}

html, body { height: 100%; font-family: 'Sora', sans-serif; font-size: 15px; color: var(--text-primary); background: var(--bg-deep); }

a { color: var(--accent-light); text-decoration: none; }
a:hover { text-decoration: underline; }

/* ── Page Header ── */
.page-header { display: flex; align-items: center; justify-content: space-between; margin-bottom: 1.75rem; }
.page-title  { font-size: 1.6rem; font-weight: 700; letter-spacing: -.03em; }
.page-sub    { font-size: .88rem; color: var(--text-muted); margin-top: .2rem; }

/* ── Badges ── */
.badge {
    display: inline-block; padding: .25rem .65rem; border-radius: 999px;
    font-size: .73rem; font-weight: 600; text-transform: uppercase; letter-spacing: .08em;
}
.badge-employee { background: rgba(34,197,94,.12);  color: #4ade80; }
.badge-manager  { background: rgba(99,102,241,.15); color: var(--accent-light); }

/* ── Alerts ── */
.alert {
    padding: .85rem 1.1rem; border-radius: 10px; font-size: .88rem; font-weight: 500;
    border-left: 3px solid;
}
.alert-success { background: rgba(34,197,94,.1);  border-color: var(--success); color: #86efac; }
.alert-error   { background: rgba(239,68,68,.1);  border-color: var(--error);   color: #fca5a5; }

/* ── Forms ── */
.form-group { margin-bottom: 1.1rem; }
.form-group label { display: block; font-size: .8rem; font-weight: 600; color: var(--text-muted); text-transform: uppercase; letter-spacing: .07em; margin-bottom: .4rem; }
.form-group input, .form-group select {
    width: 100%; padding: .65rem .9rem;
    background: var(--bg-deep); border: 1px solid var(--border);
    border-radius: 8px; color: var(--text-primary); font-family: inherit; font-size: .9rem;
    transition: border-color .15s;
    outline: none;
}
.form-group input:focus, .form-group select:focus { border-color: var(--accent); }
.form-group input::placeholder { color: var(--text-muted); }

/* ── Buttons ── */
.btn {
    display: inline-block; padding: .65rem 1.2rem; border-radius: 8px;
    font-family: inherit; font-size: .88rem; font-weight: 600; cursor: pointer;
    border: none; transition: all .15s; line-height: 1;
}
.btn-primary   { background: var(--accent); color: #fff; }
.btn-primary:hover { background: #5254cc; box-shadow: 0 4px 14px rgba(99,102,241,.4); }
.btn-secondary { background: var(--bg-deep); color: var(--text-muted); border: 1px solid var(--border); }
.btn-secondary:hover { color: var(--text-primary); border-color: var(--text-muted); }
.btn-sm { padding: .35rem .75rem; font-size: .8rem; border-radius: 6px; }

/* ── Avatars ── */
.avatar-sm {
    width: 34px; height: 34px; border-radius: 50%;
    background: linear-gradient(135deg, var(--accent), #8b5cf6);
    display: inline-flex; align-items: center; justify-content: center;
    font-size: .72rem; font-weight: 700; color: #fff; flex-shrink: 0;
}

/* ── Detail Table ── */
.detail-table { width: 100%; border-collapse: collapse; font-size: .88rem; }
.detail-table th { width: 35%; text-align: left; padding: .5rem 0; color: var(--text-muted); font-weight: 500; vertical-align: top; }
.detail-table td { padding: .5rem 0; color: var(--text-primary); border-bottom: 1px solid var(--border-light); }
.detail-table tr:last-child td { border-bottom: none; }

/* ── Utilities ── */
.muted  { color: var(--text-muted); }
.mt-1   { margin-top: .75rem; }
.mt-2   { margin-top: 1.25rem; }

::-webkit-scrollbar { width: 6px; }
::-webkit-scrollbar-track { background: transparent; }
::-webkit-scrollbar-thumb { background: var(--border); border-radius: 3px; }
</style>
CSS;
}

// ─────────────────────────────────────────────
//  UTILITY
// ─────────────────────────────────────────────
function h(string|int $s): string {
    return htmlspecialchars((string)$s, ENT_QUOTES | ENT_SUBSTITUTE, 'UTF-8');
}

// ─────────────────────────────────────────────
//  ROUTER
// ─────────────────────────────────────────────
try {
    bootstrapDB();
} catch (PDOException $e) {
    http_response_code(500);
    echo '<div style="font-family:monospace;padding:2rem;color:#ef4444;background:#0d0f14;min-height:100vh">';
    echo '<h2>Database Connection Error</h2>';
    echo '<p>Could not connect to MySQL. Please check your configuration constants at the top of this file.</p>';
    echo '<pre style="color:#fca5a5;margin-top:1rem">' . h($e->getMessage()) . '</pre>';
    echo '<hr style="border-color:#252a38;margin:1.5rem 0">';
    echo '<p>Ensure the database <strong>' . DB_NAME . '</strong> exists and the credentials are correct, then reload.</p>';
    echo '</div>';
    exit;
}

$action = $_POST['action'] ?? ($_GET['action'] ?? '');
$page   = $_GET['page'] ?? (currentUser() ? 'dashboard' : 'login');

// Handle POST actions
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    match($action) {
        'login'          => actionLogin(),
        'logout'         => actionLogout(),
        'update_role'    => actionUpdateRole(),
        'update_profile' => actionUpdateProfile(),
        default          => redirect($page, 'Unknown action.'),
    };
    exit;
}

// Handle GET pages
match($page) {
    'login'     => pageLogin(),
    'dashboard' => pageDashboard(),
    'profile'   => pageProfile(),
    'employees' => pageEmployees(),
    default     => redirect('dashboard'),
};