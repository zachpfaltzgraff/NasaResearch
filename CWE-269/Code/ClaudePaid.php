<?php
// ============================================================
//  EMPLOYEE MANAGEMENT SYSTEM  —  Single-File PHP Application
// ============================================================

// ─── Database Configuration ──────────────────────────────────
define('DB_HOST', 'localhost');
define('DB_NAME', 'employee_mgmt');
define('DB_USER', 'root');
define('DB_PASS', '');
define('DB_PORT', 3306);

// ─── Session Start ────────────────────────────────────────────
session_start();

// ─── Database Connection ──────────────────────────────────────
function getDB(): PDO {
    static $pdo = null;
    if ($pdo === null) {
        try {
            $dsn = sprintf(
                'mysql:host=%s;port=%d;dbname=%s;charset=utf8mb4',
                DB_HOST, DB_PORT, DB_NAME
            );
            $pdo = new PDO($dsn, DB_USER, DB_PASS, [
                PDO::ATTR_ERRMODE            => PDO::ERRMODE_EXCEPTION,
                PDO::ATTR_DEFAULT_FETCH_MODE => PDO::FETCH_ASSOC,
                PDO::ATTR_EMULATE_PREPARES   => false,
            ]);
        } catch (PDOException $e) {
            die(renderError('Database Connection Failed', $e->getMessage()));
        }
    }
    return $pdo;
}

// ─── Database Setup ───────────────────────────────────────────
function setupDatabase(): void {
    $pdo = getDB();

    $pdo->exec("
        CREATE TABLE IF NOT EXISTS employees (
            id          INT AUTO_INCREMENT PRIMARY KEY,
            username    VARCHAR(64)  NOT NULL UNIQUE,
            password    VARCHAR(255) NOT NULL,
            full_name   VARCHAR(128) NOT NULL,
            position    VARCHAR(128) NOT NULL DEFAULT 'Staff',
            role        ENUM('employee','manager') NOT NULL DEFAULT 'employee',
            department  VARCHAR(100) NOT NULL DEFAULT 'General',
            email       VARCHAR(180) NOT NULL DEFAULT '',
            created_at  DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
    ");

    // Seed demo users only if the table is empty
    $count = (int) $pdo->query("SELECT COUNT(*) FROM employees")->fetchColumn();
    if ($count === 0) {
        $users = [
            ['alice',   'password123', 'Alice Johnson',  'Engineering Manager', 'manager',  'Engineering', 'alice@example.com'],
            ['bob',     'password123', 'Bob Smith',      'Software Engineer',   'employee', 'Engineering', 'bob@example.com'],
            ['carol',   'password123', 'Carol White',    'UX Designer',         'employee', 'Design',      'carol@example.com'],
            ['dave',    'password123', 'Dave Brown',     'QA Engineer',         'employee', 'QA',          'dave@example.com'],
            ['eve',     'password123', 'Eve Davis',      'Product Manager',     'manager',  'Product',     'eve@example.com'],
            ['frank',   'password123', 'Frank Miller',   'DevOps Engineer',     'employee', 'Infrastructure','frank@example.com'],
        ];
        $stmt = $pdo->prepare("
            INSERT INTO employees (username, password, full_name, position, role, department, email)
            VALUES (?, ?, ?, ?, ?, ?, ?)
        ");
        foreach ($users as $u) {
            $u[1] = password_hash($u[1], PASSWORD_BCRYPT);
            $stmt->execute($u);
        }
    }
}

// ─── Auth Helpers ─────────────────────────────────────────────
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
    if ((currentUser()['role'] ?? '') !== 'manager') {
        redirect('dashboard');
    }
}

function redirect(string $page, string $msg = '', string $msgType = 'info'): never {
    $url = '?page=' . urlencode($page);
    if ($msg)     $url .= '&msg='     . urlencode($msg);
    if ($msgType) $url .= '&msgType=' . urlencode($msgType);
    header('Location: ' . $url);
    exit;
}

// ─── CSRF Helpers ─────────────────────────────────────────────
function csrfToken(): string {
    if (empty($_SESSION['csrf_token'])) {
        $_SESSION['csrf_token'] = bin2hex(random_bytes(32));
    }
    return $_SESSION['csrf_token'];
}

function verifyCsrf(): void {
    $token = $_POST['csrf_token'] ?? '';
    if (!hash_equals(csrfToken(), $token)) {
        die(renderError('Security Error', 'Invalid CSRF token. Please go back and try again.'));
    }
}

// ─── Flash Messages ───────────────────────────────────────────
function flashMessage(): string {
    $msg     = htmlspecialchars($_GET['msg']     ?? '', ENT_QUOTES);
    $msgType = htmlspecialchars($_GET['msgType'] ?? 'info', ENT_QUOTES);
    if (!$msg) return '';
    $icons = ['success' => '✔', 'error' => '✖', 'info' => 'ℹ', 'warning' => '⚠'];
    $icon  = $icons[$msgType] ?? 'ℹ';
    return <<<HTML
        <div class="alert alert-{$msgType}" role="alert">
            <span class="alert-icon">{$icon}</span>
            <span>{$msg}</span>
        </div>
    HTML;
}

// ─── Error Renderer ───────────────────────────────────────────
function renderError(string $title, string $detail): string {
    return <<<HTML
        <!DOCTYPE html><html lang="en"><head><meta charset="UTF-8">
        <title>Error — {$title}</title>
        <style>body{font-family:sans-serif;padding:2rem;background:#fff8f8;color:#333}
        h1{color:#c0392b}pre{background:#f4f4f4;padding:1rem;border-radius:6px}</style>
        </head><body><h1>⚠ {$title}</h1><pre>{$detail}</pre></body></html>
    HTML;
}

// ============================================================
//  REQUEST HANDLING
// ============================================================
setupDatabase();

$page   = $_GET['page']   ?? 'login';
$action = $_POST['action'] ?? '';

// ── POST Handlers ─────────────────────────────────────────────

// Login
if ($action === 'login') {
    verifyCsrf();
    $username = trim($_POST['username'] ?? '');
    $password = $_POST['password'] ?? '';

    $stmt = getDB()->prepare("SELECT * FROM employees WHERE username = ?");
    $stmt->execute([$username]);
    $user = $stmt->fetch();

    if ($user && password_verify($password, $user['password'])) {
        session_regenerate_id(true);
        $_SESSION['user'] = [
            'id'        => $user['id'],
            'username'  => $user['username'],
            'full_name' => $user['full_name'],
            'role'      => $user['role'],
            'position'  => $user['position'],
        ];
        redirect('dashboard');
    } else {
        redirect('login', 'Invalid username or password.', 'error');
    }
}

// Logout
if ($action === 'logout') {
    session_destroy();
    redirect('login', 'You have been logged out.', 'info');
}

// Update employee role / position (managers only)
if ($action === 'update_employee') {
    verifyCsrf();
    requireManager();

    $empId    = (int)  ($_POST['employee_id'] ?? 0);
    $role     = in_array($_POST['role'] ?? '', ['employee', 'manager']) ? $_POST['role'] : 'employee';
    $position = trim($_POST['position'] ?? '');

    if ($empId <= 0 || $position === '') {
        redirect('employees', 'Invalid data provided.', 'error');
    }

    $stmt = getDB()->prepare("UPDATE employees SET role = ?, position = ? WHERE id = ?");
    $stmt->execute([$role, $position, $empId]);
    redirect('employees', 'Employee updated successfully.', 'success');
}

// ============================================================
//  PAGE RENDERERS
// ============================================================

function renderLayout(string $title, string $body, bool $showNav = true): void {
    $user      = currentUser();
    $nav       = '';
    $pageTitle = htmlspecialchars($title);

    if ($showNav && $user) {
        $nameHtml = htmlspecialchars($user['full_name']);
        $roleHtml = htmlspecialchars(ucfirst($user['role']));
        $empLink  = $user['role'] === 'manager'
            ? '<a href="?page=employees" class="nav-link">👥 Employees</a>'
            : '';
        $profileLink = '<a href="?page=profile" class="nav-link">👤 My Profile</a>';

        $nav = <<<HTML
        <nav class="navbar">
            <div class="nav-brand">
                <span class="brand-icon">🏢</span>
                <span class="brand-text">EMS</span>
            </div>
            <div class="nav-links">
                <a href="?page=dashboard" class="nav-link">🏠 Dashboard</a>
                {$profileLink}
                {$empLink}
            </div>
            <div class="nav-user">
                <span class="user-badge role-{$user['role']}">{$roleHtml}</span>
                <span class="user-name">{$nameHtml}</span>
                <form method="POST" style="display:inline">
                    <input type="hidden" name="action" value="logout">
                    <input type="hidden" name="csrf_token" value="<?= csrfToken() ?>">
                    <button type="submit" class="btn btn-sm btn-outline">Logout</button>
                </form>
            </div>
        </nav>
        HTML;
        // Replace the lazy csrf placeholder
        $nav = str_replace('<?= csrfToken() ?>', csrfToken(), $nav);
    }

    echo <<<HTML
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>{$pageTitle} — Employee Management</title>
        <style>
            /* ── Reset & Base ── */
            *, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }
            html { font-size: 16px; }
            body {
                font-family: 'Segoe UI', system-ui, -apple-system, sans-serif;
                background: #f0f2f5;
                color: #1a1a2e;
                min-height: 100vh;
            }

            /* ── Navbar ── */
            .navbar {
                background: linear-gradient(135deg, #1a1a2e 0%, #16213e 100%);
                color: #fff;
                padding: .75rem 1.5rem;
                display: flex;
                align-items: center;
                gap: 1rem;
                box-shadow: 0 2px 12px rgba(0,0,0,.25);
                position: sticky;
                top: 0;
                z-index: 100;
            }
            .nav-brand { display:flex; align-items:center; gap:.5rem; margin-right:auto; }
            .brand-icon { font-size:1.4rem; }
            .brand-text { font-size:1.2rem; font-weight:700; letter-spacing:.05em; color:#e94560; }
            .nav-links  { display:flex; gap:.25rem; }
            .nav-link {
                color: #ccc;
                text-decoration: none;
                padding: .4rem .75rem;
                border-radius: 6px;
                font-size: .875rem;
                transition: background .2s, color .2s;
            }
            .nav-link:hover { background: rgba(255,255,255,.1); color:#fff; }
            .nav-user { display:flex; align-items:center; gap:.75rem; margin-left:1rem; }
            .user-name { font-size:.875rem; color: #ccc; }
            .user-badge {
                font-size: .7rem;
                font-weight: 700;
                letter-spacing: .08em;
                text-transform: uppercase;
                padding: .2rem .55rem;
                border-radius: 99px;
            }
            .role-manager  { background: #e94560; color: #fff; }
            .role-employee { background: #0f3460; color: #7ec8e3; }

            /* ── Main Container ── */
            .container {
                max-width: 960px;
                margin: 2rem auto;
                padding: 0 1rem;
            }

            /* ── Cards ── */
            .card {
                background: #fff;
                border-radius: 12px;
                box-shadow: 0 2px 16px rgba(0,0,0,.07);
                overflow: hidden;
                margin-bottom: 1.5rem;
            }
            .card-header {
                background: linear-gradient(135deg, #0f3460 0%, #16213e 100%);
                color: #fff;
                padding: 1rem 1.5rem;
                font-size: 1.1rem;
                font-weight: 600;
            }
            .card-body { padding: 1.5rem; }

            /* ── Page Header ── */
            .page-header { margin-bottom: 1.5rem; }
            .page-header h1 { font-size: 1.6rem; font-weight: 700; color: #1a1a2e; }
            .page-header p  { color: #666; margin-top: .25rem; font-size: .9rem; }

            /* ── Stats Strip ── */
            .stats-grid {
                display: grid;
                grid-template-columns: repeat(auto-fit, minmax(140px, 1fr));
                gap: 1rem;
                margin-bottom: 1.5rem;
            }
            .stat-card {
                background: #fff;
                border-radius: 10px;
                padding: 1rem 1.25rem;
                box-shadow: 0 2px 8px rgba(0,0,0,.06);
                display: flex;
                flex-direction: column;
                gap: .25rem;
            }
            .stat-value { font-size: 1.8rem; font-weight: 700; color: #e94560; }
            .stat-label { font-size: .8rem; color: #888; text-transform: uppercase; letter-spacing: .06em; }

            /* ── Table ── */
            .table-wrap { overflow-x: auto; }
            table { width: 100%; border-collapse: collapse; font-size: .9rem; }
            thead tr { background: #f8f9fa; }
            th { padding: .75rem 1rem; text-align: left; font-weight: 600; color: #444; border-bottom: 2px solid #e9ecef; white-space: nowrap; }
            td { padding: .75rem 1rem; border-bottom: 1px solid #f0f0f0; color: #333; vertical-align: middle; }
            tr:last-child td { border-bottom: none; }
            tr:hover td { background: #fafafa; }

            /* ── Profile Grid ── */
            .profile-grid {
                display: grid;
                grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
                gap: 1.25rem;
            }
            .profile-field label {
                font-size: .75rem;
                text-transform: uppercase;
                letter-spacing: .08em;
                color: #888;
                display: block;
                margin-bottom: .3rem;
            }
            .profile-field .value {
                font-size: 1rem;
                color: #1a1a2e;
                font-weight: 500;
            }

            /* ── Forms ── */
            .form-group { margin-bottom: 1rem; }
            .form-group label {
                display: block;
                font-size: .85rem;
                font-weight: 600;
                color: #444;
                margin-bottom: .4rem;
            }
            .form-control {
                width: 100%;
                padding: .6rem .85rem;
                border: 1.5px solid #dce1e9;
                border-radius: 7px;
                font-size: .95rem;
                color: #1a1a2e;
                transition: border-color .2s, box-shadow .2s;
                background: #fff;
            }
            .form-control:focus {
                outline: none;
                border-color: #e94560;
                box-shadow: 0 0 0 3px rgba(233,69,96,.12);
            }
            select.form-control { cursor: pointer; }

            /* ── Buttons ── */
            .btn {
                display: inline-flex;
                align-items: center;
                gap: .4rem;
                padding: .6rem 1.25rem;
                border: none;
                border-radius: 7px;
                font-size: .9rem;
                font-weight: 600;
                cursor: pointer;
                transition: all .2s;
                text-decoration: none;
            }
            .btn-primary {
                background: linear-gradient(135deg, #e94560, #c0392b);
                color: #fff;
                box-shadow: 0 2px 8px rgba(233,69,96,.3);
            }
            .btn-primary:hover { transform: translateY(-1px); box-shadow: 0 4px 14px rgba(233,69,96,.4); }
            .btn-secondary { background: #f0f2f5; color: #444; }
            .btn-secondary:hover { background: #e2e6ea; }
            .btn-sm { padding: .35rem .8rem; font-size: .8rem; }
            .btn-outline {
                background: transparent;
                border: 1.5px solid rgba(255,255,255,.3);
                color: #ccc;
                padding: .3rem .75rem;
                font-size: .8rem;
            }
            .btn-outline:hover { background: rgba(255,255,255,.1); color: #fff; }
            .btn-icon {
                background: none;
                border: none;
                cursor: pointer;
                font-size: 1.1rem;
                padding: .25rem .5rem;
                border-radius: 5px;
                transition: background .15s;
            }
            .btn-icon:hover { background: #f0f0f0; }

            /* ── Alerts ── */
            .alert {
                display: flex;
                align-items: center;
                gap: .75rem;
                padding: .85rem 1.1rem;
                border-radius: 8px;
                margin-bottom: 1.25rem;
                font-size: .9rem;
                font-weight: 500;
            }
            .alert-icon { font-size: 1.1rem; }
            .alert-success { background: #d4edda; color: #155724; border-left: 4px solid #28a745; }
            .alert-error   { background: #f8d7da; color: #721c24; border-left: 4px solid #dc3545; }
            .alert-info    { background: #d1ecf1; color: #0c5460; border-left: 4px solid #17a2b8; }
            .alert-warning { background: #fff3cd; color: #856404; border-left: 4px solid #ffc107; }

            /* ── Login page ── */
            .login-wrapper {
                min-height: 100vh;
                display: flex;
                align-items: center;
                justify-content: center;
                background: linear-gradient(135deg, #1a1a2e 0%, #16213e 50%, #0f3460 100%);
            }
            .login-box {
                background: #fff;
                border-radius: 16px;
                box-shadow: 0 20px 60px rgba(0,0,0,.35);
                padding: 2.5rem 2.25rem;
                width: 100%;
                max-width: 400px;
            }
            .login-logo { text-align: center; margin-bottom: 1.75rem; }
            .login-logo .icon { font-size: 3rem; }
            .login-logo h1 { font-size: 1.5rem; font-weight: 800; color: #1a1a2e; margin-top: .5rem; }
            .login-logo p  { color: #888; font-size: .85rem; margin-top: .2rem; }
            .demo-accounts {
                background: #f8f9fa;
                border-radius: 8px;
                padding: .85rem 1rem;
                margin-top: 1.25rem;
                font-size: .8rem;
                color: #555;
            }
            .demo-accounts strong { display: block; margin-bottom: .4rem; color: #333; }
            .demo-row { display: flex; gap: .5rem; margin-top: .3rem; }
            .demo-badge {
                background: #e94560;
                color: #fff;
                border-radius: 4px;
                padding: .1rem .4rem;
                font-size: .7rem;
                font-weight: 700;
                text-transform: uppercase;
            }
            .demo-badge.mgr { background: #0f3460; }

            /* ── Modal ── */
            .modal-overlay {
                display: none;
                position: fixed; inset: 0;
                background: rgba(0,0,0,.5);
                z-index: 200;
                align-items: center;
                justify-content: center;
            }
            .modal-overlay.open { display: flex; }
            .modal {
                background: #fff;
                border-radius: 12px;
                width: 100%;
                max-width: 440px;
                padding: 1.75rem;
                box-shadow: 0 10px 40px rgba(0,0,0,.25);
                position: relative;
            }
            .modal-title { font-size: 1.1rem; font-weight: 700; margin-bottom: 1.25rem; }
            .modal-close {
                position: absolute; top: 1rem; right: 1rem;
                font-size: 1.25rem; background: none; border: none; cursor: pointer; color: #888;
            }
            .modal-close:hover { color: #333; }
            .modal-footer { display: flex; justify-content: flex-end; gap: .75rem; margin-top: 1.25rem; }

            /* ── Role badge inline ── */
            .role-pill {
                display: inline-block;
                font-size: .7rem;
                font-weight: 700;
                letter-spacing: .06em;
                text-transform: uppercase;
                padding: .2rem .55rem;
                border-radius: 99px;
            }
            .role-pill.manager  { background: #fde8ec; color: #c0392b; }
            .role-pill.employee { background: #e8f4fd; color: #0f3460; }

            /* ── Divider ── */
            hr.divider { border: none; border-top: 1px solid #eee; margin: 1.25rem 0; }

            /* ── Responsive ── */
            @media (max-width: 600px) {
                .nav-links { display: none; }
                .user-name { display: none; }
                .stats-grid { grid-template-columns: 1fr 1fr; }
            }
        </style>
    </head>
    <body>
        {$nav}
        {$body}
    </body>
    </html>
    HTML;
}

// ─── PAGE: Login ──────────────────────────────────────────────
function pageLGIN(): void {
    if (currentUser()) redirect('dashboard');

    $flash = flashMessage();
    $token = csrfToken();

    $body = <<<HTML
    <div class="login-wrapper">
        <div class="login-box">
            <div class="login-logo">
                <div class="icon">🏢</div>
                <h1>Employee Management</h1>
                <p>Sign in to your account</p>
            </div>
            {$flash}
            <form method="POST">
                <input type="hidden" name="action"     value="login">
                <input type="hidden" name="csrf_token" value="{$token}">
                <div class="form-group">
                    <label for="username">Username</label>
                    <input id="username" name="username" type="text" class="form-control"
                           placeholder="Enter your username" required autofocus>
                </div>
                <div class="form-group">
                    <label for="password">Password</label>
                    <input id="password" name="password" type="password" class="form-control"
                           placeholder="Enter your password" required>
                </div>
                <button type="submit" class="btn btn-primary" style="width:100%;justify-content:center;margin-top:.5rem">
                    🔐 Sign In
                </button>
            </form>
            <div class="demo-accounts">
                <strong>Demo Accounts (password: password123)</strong>
                <div class="demo-row"><span class="demo-badge mgr">manager</span> alice &nbsp;|&nbsp; eve</div>
                <div class="demo-row"><span class="demo-badge">employee</span> bob, carol, dave, frank</div>
            </div>
        </div>
    </div>
    HTML;

    renderLayout('Login', $body, false);
}

// ─── PAGE: Dashboard ──────────────────────────────────────────
function pageDashboard(): void {
    requireLogin();
    $user  = currentUser();
    $flash = flashMessage();
    $name  = htmlspecialchars($user['full_name']);
    $role  = htmlspecialchars(ucfirst($user['role']));
    $pos   = htmlspecialchars($user['position']);

    // Quick stats for managers
    $statsHtml = '';
    if ($user['role'] === 'manager') {
        $pdo     = getDB();
        $total   = (int) $pdo->query("SELECT COUNT(*) FROM employees")->fetchColumn();
        $mgrs    = (int) $pdo->query("SELECT COUNT(*) FROM employees WHERE role='manager'")->fetchColumn();
        $emps    = $total - $mgrs;
        $depts   = (int) $pdo->query("SELECT COUNT(DISTINCT department) FROM employees")->fetchColumn();

        $statsHtml = <<<HTML
        <div class="stats-grid">
            <div class="stat-card"><span class="stat-value">{$total}</span><span class="stat-label">Total Employees</span></div>
            <div class="stat-card"><span class="stat-value">{$emps}</span><span class="stat-label">Employees</span></div>
            <div class="stat-card"><span class="stat-value">{$mgrs}</span><span class="stat-label">Managers</span></div>
            <div class="stat-card"><span class="stat-value">{$depts}</span><span class="stat-label">Departments</span></div>
        </div>
        HTML;
    }

    $managerActions = '';
    if ($user['role'] === 'manager') {
        $managerActions = <<<HTML
        <hr class="divider">
        <h3 style="margin-bottom:.75rem;font-size:1rem">⚡ Quick Actions</h3>
        <a href="?page=employees" class="btn btn-primary">👥 Manage Employees</a>
        HTML;
    }

    $body = <<<HTML
    <div class="container">
        {$flash}
        <div class="page-header">
            <h1>Welcome back, {$name} 👋</h1>
            <p>{$pos} &middot; {$role}</p>
        </div>
        {$statsHtml}
        <div class="card">
            <div class="card-header">📋 Your Overview</div>
            <div class="card-body">
                <div class="profile-grid">
                    <div class="profile-field"><label>Employee ID</label><div class="value">EMP-{$user['id']}</div></div>
                    <div class="profile-field"><label>Full Name</label><div class="value">{$name}</div></div>
                    <div class="profile-field"><label>Position</label><div class="value">{$pos}</div></div>
                    <div class="profile-field"><label>Role</label><div class="value">{$role}</div></div>
                </div>
                {$managerActions}
            </div>
        </div>
    </div>
    HTML;

    renderLayout('Dashboard', $body);
}

// ─── PAGE: My Profile ─────────────────────────────────────────
function pageProfile(): void {
    requireLogin();
    $user = currentUser();

    $stmt = getDB()->prepare("SELECT * FROM employees WHERE id = ?");
    $stmt->execute([$user['id']]);
    $emp = $stmt->fetch();

    $flash      = flashMessage();
    $id         = htmlspecialchars('EMP-' . $emp['id']);
    $name       = htmlspecialchars($emp['full_name']);
    $username   = htmlspecialchars($emp['username']);
    $position   = htmlspecialchars($emp['position']);
    $role       = htmlspecialchars(ucfirst($emp['role']));
    $dept       = htmlspecialchars($emp['department']);
    $email      = htmlspecialchars($emp['email']);
    $created    = date('F j, Y', strtotime($emp['created_at']));
    $rolePill   = "<span class='role-pill {$emp['role']}'>{$role}</span>";

    $body = <<<HTML
    <div class="container">
        {$flash}
        <div class="page-header">
            <h1>👤 My Profile</h1>
            <p>Your personal information on record</p>
        </div>
        <div class="card">
            <div class="card-header">Personal Information</div>
            <div class="card-body">
                <div class="profile-grid">
                    <div class="profile-field"><label>Employee ID</label><div class="value">{$id}</div></div>
                    <div class="profile-field"><label>Full Name</label><div class="value">{$name}</div></div>
                    <div class="profile-field"><label>Username</label><div class="value">{$username}</div></div>
                    <div class="profile-field"><label>Email</label><div class="value">{$email}</div></div>
                    <div class="profile-field"><label>Position</label><div class="value">{$position}</div></div>
                    <div class="profile-field"><label>Department</label><div class="value">{$dept}</div></div>
                    <div class="profile-field"><label>Role</label><div class="value">{$rolePill}</div></div>
                    <div class="profile-field"><label>Member Since</label><div class="value">{$created}</div></div>
                </div>
            </div>
        </div>
    </div>
    HTML;

    renderLayout('My Profile', $body);
}

// ─── PAGE: Employees (manager only) ──────────────────────────
function pageEmployees(): void {
    requireManager();
    $flash = flashMessage();

    $employees = getDB()->query("SELECT * FROM employees ORDER BY role DESC, full_name ASC")->fetchAll();

    $rows = '';
    foreach ($employees as $emp) {
        $eid      = (int) $emp['id'];
        $name     = htmlspecialchars($emp['full_name']);
        $username = htmlspecialchars($emp['username']);
        $pos      = htmlspecialchars($emp['position']);
        $dept     = htmlspecialchars($emp['department']);
        $email    = htmlspecialchars($emp['email']);
        $role     = $emp['role'];
        $rolePill = "<span class='role-pill {$role}'>" . ucfirst($role) . "</span>";
        $rows .= <<<HTML
        <tr>
            <td><strong>EMP-{$eid}</strong></td>
            <td>{$name}</td>
            <td style="color:#888">{$username}</td>
            <td>{$pos}</td>
            <td>{$dept}</td>
            <td>{$email}</td>
            <td>{$rolePill}</td>
            <td>
                <button class="btn-icon" title="Edit"
                    onclick="openEditModal({$eid}, '{$name}', '{$pos}', '{$role}')">✏️</button>
            </td>
        </tr>
        HTML;
    }

    $token = csrfToken();

    $body = <<<HTML
    <div class="container">
        {$flash}
        <div class="page-header">
            <h1>👥 Employee Directory</h1>
            <p>View and manage all employees in the system</p>
        </div>
        <div class="card">
            <div class="card-header">All Employees</div>
            <div class="card-body" style="padding:0">
                <div class="table-wrap">
                    <table>
                        <thead>
                            <tr>
                                <th>ID</th>
                                <th>Name</th>
                                <th>Username</th>
                                <th>Position</th>
                                <th>Department</th>
                                <th>Email</th>
                                <th>Role</th>
                                <th>Actions</th>
                            </tr>
                        </thead>
                        <tbody>{$rows}</tbody>
                    </table>
                </div>
            </div>
        </div>
    </div>

    <!-- Edit Modal -->
    <div class="modal-overlay" id="editModal">
        <div class="modal">
            <button class="modal-close" onclick="closeModal()">✕</button>
            <div class="modal-title">✏️ Edit Employee</div>
            <form method="POST" id="editForm">
                <input type="hidden" name="action"      value="update_employee">
                <input type="hidden" name="csrf_token"  value="{$token}">
                <input type="hidden" name="employee_id" id="modal_employee_id">
                <div class="form-group">
                    <label>Employee</label>
                    <input type="text" id="modal_employee_name" class="form-control" disabled>
                </div>
                <div class="form-group">
                    <label for="modal_position">Position Title</label>
                    <input id="modal_position" name="position" type="text" class="form-control"
                           placeholder="e.g. Senior Engineer" required>
                </div>
                <div class="form-group">
                    <label for="modal_role">Role</label>
                    <select id="modal_role" name="role" class="form-control">
                        <option value="employee">Employee</option>
                        <option value="manager">Manager</option>
                    </select>
                </div>
                <div class="modal-footer">
                    <button type="button" class="btn btn-secondary" onclick="closeModal()">Cancel</button>
                    <button type="submit" class="btn btn-primary">💾 Save Changes</button>
                </div>
            </form>
        </div>
    </div>

    <script>
        function openEditModal(id, name, position, role) {
            document.getElementById('modal_employee_id').value   = id;
            document.getElementById('modal_employee_name').value = name;
            document.getElementById('modal_position').value      = position;
            document.getElementById('modal_role').value          = role;
            document.getElementById('editModal').classList.add('open');
        }
        function closeModal() {
            document.getElementById('editModal').classList.remove('open');
        }
        document.getElementById('editModal').addEventListener('click', function(e) {
            if (e.target === this) closeModal();
        });
    </script>
    HTML;

    renderLayout('Employees', $body);
}

// ============================================================
//  ROUTER
// ============================================================
$routes = [
    'login'     => 'pageLGIN',
    'dashboard' => 'pageDashboard',
    'profile'   => 'pageProfile',
    'employees' => 'pageEmployees',
];

$handler = $routes[$page] ?? 'pageLGIN';
$handler();