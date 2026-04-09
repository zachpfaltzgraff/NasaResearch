<?php
// ============================================================
//  DATABASE SETUP
// ============================================================
$host = 'localhost';
$db   = 'company_app';
$user = 'root';
$pass = '';          // WampServer default
$charset = 'utf8mb4';

$dsn = "mysql:host=$host;charset=$charset";

try {
    $pdo = new PDO($dsn, $user, $pass, [
        PDO::ATTR_ERRMODE            => PDO::ERRMODE_EXCEPTION,
        PDO::ATTR_DEFAULT_FETCH_MODE => PDO::FETCH_ASSOC,
    ]);

    // Create DB if it doesn't exist
    $pdo->exec("CREATE DATABASE IF NOT EXISTS `$db` CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci");
    $pdo->exec("USE `$db`");

    // Users table
    $pdo->exec("CREATE TABLE IF NOT EXISTS users (
        id          INT AUTO_INCREMENT PRIMARY KEY,
        username    VARCHAR(60)  NOT NULL UNIQUE,
        password    VARCHAR(255) NOT NULL,
        job_title   VARCHAR(100) NOT NULL,
        role        ENUM('admin','employee') NOT NULL DEFAULT 'employee',
        created_at  DATETIME DEFAULT CURRENT_TIMESTAMP
    )");

    // Announcements table
    $pdo->exec("CREATE TABLE IF NOT EXISTS announcements (
        id          INT AUTO_INCREMENT PRIMARY KEY,
        title       VARCHAR(200) NOT NULL,
        body        TEXT         NOT NULL,
        created_at  DATETIME DEFAULT CURRENT_TIMESTAMP
    )");

    // Seed sample announcements once
    $count = $pdo->query("SELECT COUNT(*) FROM announcements")->fetchColumn();
    if ($count == 0) {
        $pdo->exec("INSERT INTO announcements (title, body) VALUES
            ('Welcome to the Company Portal', 'We are excited to launch our new internal portal. Log in to stay updated on company news and announcements.'),
            ('Q2 All-Hands Meeting', 'Our next all-hands meeting is scheduled for the last Friday of the month at 2 PM in the main conference room. Attendance is mandatory for all staff.'),
            ('Updated Remote Work Policy', 'Starting next month, employees may work remotely up to three days per week. Please coordinate with your team lead to arrange schedules.'),
            ('Office Closure – Public Holiday', 'The office will be closed on the upcoming public holiday. Emergency contacts are available on the intranet.')
        ");
    }

} catch (PDOException $e) {
    die("<pre style='color:red;padding:2rem'>Database error: " . htmlspecialchars($e->getMessage()) . "\n\nMake sure WampServer is running and MySQL is active.</pre>");
}

// ============================================================
//  SESSION & HELPERS
// ============================================================
session_start();

function isLoggedIn(): bool  { return isset($_SESSION['user_id']); }
function isAdmin(): bool     { return isLoggedIn() && $_SESSION['role'] === 'admin'; }
function redirect(string $page, array $params = []): void {
    $query = array_merge(['page' => $page], $params);
    header('Location: index.php?' . http_build_query($query));
    exit;
}
function e(string $s): string { return htmlspecialchars($s, ENT_QUOTES, 'UTF-8'); }

// ============================================================
//  ROUTING / ACTION HANDLING
// ============================================================
$page   = $_GET['page']  ?? 'home';
$error  = $_GET['error'] ?? '';
$notice = $_GET['notice'] ?? '';

// ---------- SIGNUP ----------
if ($_SERVER['REQUEST_METHOD'] === 'POST' && ($page === 'signup')) {
    $username  = trim($_POST['username']  ?? '');
    $password  = trim($_POST['password']  ?? '');
    $job_title = trim($_POST['job_title'] ?? '');

    if (!$username || !$password || !$job_title) {
        redirect('signup', ['error' => 'All fields are required.']);
    }
    if (strlen($password) < 6) {
        redirect('signup', ['error' => 'Password must be at least 6 characters.']);
    }

    $stmt = $pdo->prepare("SELECT id FROM users WHERE username = ?");
    $stmt->execute([$username]);
    if ($stmt->fetch()) {
        redirect('signup', ['error' => 'Username already taken.']);
    }

    $hash = password_hash($password, PASSWORD_BCRYPT);
    // First registered user becomes admin
    $totalUsers = (int)$pdo->query("SELECT COUNT(*) FROM users")->fetchColumn();
    $role = ($totalUsers === 0) ? 'admin' : 'employee';

    $stmt = $pdo->prepare("INSERT INTO users (username, password, job_title, role) VALUES (?,?,?,?)");
    $stmt->execute([$username, $hash, $job_title, $role]);

    redirect('login', ['notice' => 'Account created! Please log in.']);
}

// ---------- LOGIN ----------
if ($_SERVER['REQUEST_METHOD'] === 'POST' && ($page === 'login')) {
    $username = trim($_POST['username'] ?? '');
    $password = trim($_POST['password'] ?? '');

    $stmt = $pdo->prepare("SELECT * FROM users WHERE username = ?");
    $stmt->execute([$username]);
    $user = $stmt->fetch();

    if (!$user || !password_verify($password, $user['password'])) {
        redirect('login', ['error' => 'Invalid username or password.']);
    }

    $_SESSION['user_id']   = $user['id'];
    $_SESSION['username']  = $user['username'];
    $_SESSION['job_title'] = $user['job_title'];
    $_SESSION['role']      = $user['role'];

    redirect('home');
}

// ---------- LOGOUT ----------
if ($page === 'logout') {
    session_destroy();
    redirect('login', ['notice' => 'You have been logged out.']);
}

// ---------- GUARDS ----------
$publicPages = ['login', 'signup'];
if (!isLoggedIn() && !in_array($page, $publicPages)) {
    redirect('login');
}
if ($page === 'admin' && !isAdmin()) {
    redirect('home', ['error' => 'Access denied.']);
}

// ---------- DATA FOR PAGES ----------
$allUsers      = [];
$announcements = [];

if ($page === 'admin' && isAdmin()) {
    $allUsers = $pdo->query("SELECT id, username, job_title, role, created_at FROM users ORDER BY created_at DESC")->fetchAll();
}
if ($page === 'employee' || $page === 'home') {
    $announcements = $pdo->query("SELECT * FROM announcements ORDER BY created_at DESC")->fetchAll();
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
<title>NexCorp Portal</title>
<link rel="preconnect" href="https://fonts.googleapis.com">
<link href="https://fonts.googleapis.com/css2?family=Syne:wght@400;600;700;800&family=DM+Sans:ital,wght@0,300;0,400;0,500;1,300&display=swap" rel="stylesheet">
<style>
/* ── Reset & Variables ─────────────────────────────────────── */
*, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }

:root {
    --bg:       #0a0c10;
    --surface:  #12151c;
    --border:   #1f2535;
    --accent:   #e8f040;   /* electric lime */
    --accent2:  #40a8f0;   /* cool blue     */
    --text:     #e8eaf0;
    --muted:    #6b7280;
    --danger:   #f04060;
    --success:  #40f090;
    --radius:   12px;
    --font-head: 'Syne', sans-serif;
    --font-body: 'DM Sans', sans-serif;
}

html { font-size: 16px; }
body {
    background: var(--bg);
    color: var(--text);
    font-family: var(--font-body);
    font-weight: 300;
    min-height: 100vh;
    line-height: 1.6;
}

/* ── Noise texture overlay ─────────────────────────────────── */
body::before {
    content: '';
    position: fixed; inset: 0;
    background-image: url("data:image/svg+xml,%3Csvg viewBox='0 0 256 256' xmlns='http://www.w3.org/2000/svg'%3E%3Cfilter id='noise'%3E%3CfeTurbulence type='fractalNoise' baseFrequency='0.9' numOctaves='4' stitchTiles='stitch'/%3E%3C/filter%3E%3Crect width='100%25' height='100%25' filter='url(%23noise)' opacity='0.03'/%3E%3C/svg%3E");
    pointer-events: none;
    z-index: 0;
}

/* ── Layout ────────────────────────────────────────────────── */
.shell {
    position: relative;
    z-index: 1;
    display: flex;
    flex-direction: column;
    min-height: 100vh;
}

/* ── Nav ───────────────────────────────────────────────────── */
nav {
    display: flex;
    align-items: center;
    justify-content: space-between;
    padding: 0 2rem;
    height: 64px;
    background: rgba(18,21,28,0.85);
    backdrop-filter: blur(12px);
    border-bottom: 1px solid var(--border);
    position: sticky; top: 0; z-index: 100;
}
.nav-brand {
    font-family: var(--font-head);
    font-weight: 800;
    font-size: 1.3rem;
    letter-spacing: -0.03em;
    color: var(--text);
    text-decoration: none;
    display: flex; align-items: center; gap: 0.5rem;
}
.nav-brand span { color: var(--accent); }
.nav-links { display: flex; align-items: center; gap: 0.25rem; }
.nav-links a {
    font-family: var(--font-body);
    font-size: 0.85rem;
    font-weight: 400;
    color: var(--muted);
    text-decoration: none;
    padding: 0.4rem 0.85rem;
    border-radius: 6px;
    transition: color .2s, background .2s;
}
.nav-links a:hover { color: var(--text); background: var(--border); }
.nav-links a.active { color: var(--accent); }
.nav-user {
    font-size: 0.8rem;
    color: var(--muted);
    display: flex; align-items: center; gap: 0.75rem;
}
.nav-user strong { color: var(--text); }
.btn-logout {
    background: transparent;
    border: 1px solid var(--border);
    color: var(--muted);
    font-family: var(--font-body);
    font-size: 0.78rem;
    padding: 0.3rem 0.8rem;
    border-radius: 6px;
    cursor: pointer;
    text-decoration: none;
    transition: border-color .2s, color .2s;
}
.btn-logout:hover { border-color: var(--danger); color: var(--danger); }

/* ── Main content ──────────────────────────────────────────── */
main { flex: 1; padding: 3rem 2rem; max-width: 1100px; width: 100%; margin: 0 auto; }

/* ── Page header ───────────────────────────────────────────── */
.page-header { margin-bottom: 2.5rem; }
.page-header h1 {
    font-family: var(--font-head);
    font-weight: 800;
    font-size: clamp(2rem, 4vw, 3rem);
    letter-spacing: -0.04em;
    line-height: 1.1;
}
.page-header h1 em { color: var(--accent); font-style: normal; }
.page-header p { color: var(--muted); margin-top: 0.5rem; font-size: 0.95rem; }

/* ── Auth pages ────────────────────────────────────────────── */
.auth-wrap {
    display: flex;
    justify-content: center;
    align-items: center;
    min-height: calc(100vh - 64px);
    padding: 2rem;
}
.auth-card {
    background: var(--surface);
    border: 1px solid var(--border);
    border-radius: var(--radius);
    padding: 2.5rem;
    width: 100%;
    max-width: 420px;
    position: relative;
    overflow: hidden;
}
.auth-card::before {
    content: '';
    position: absolute;
    top: -80px; right: -80px;
    width: 220px; height: 220px;
    background: radial-gradient(circle, rgba(232,240,64,0.12) 0%, transparent 70%);
    pointer-events: none;
}
.auth-card h2 {
    font-family: var(--font-head);
    font-weight: 800;
    font-size: 1.8rem;
    letter-spacing: -0.04em;
    margin-bottom: 0.25rem;
}
.auth-card h2 em { color: var(--accent); font-style: normal; }
.auth-card .sub { color: var(--muted); font-size: 0.85rem; margin-bottom: 1.8rem; }

/* ── Forms ─────────────────────────────────────────────────── */
.field { margin-bottom: 1.1rem; }
.field label {
    display: block;
    font-size: 0.78rem;
    font-weight: 500;
    letter-spacing: 0.06em;
    text-transform: uppercase;
    color: var(--muted);
    margin-bottom: 0.4rem;
}
.field input, .field select {
    width: 100%;
    background: var(--bg);
    border: 1px solid var(--border);
    border-radius: 8px;
    color: var(--text);
    font-family: var(--font-body);
    font-size: 0.95rem;
    padding: 0.65rem 0.9rem;
    outline: none;
    transition: border-color .2s;
}
.field input:focus, .field select:focus { border-color: var(--accent); }
.field input::placeholder { color: var(--muted); opacity: 0.6; }

.btn-primary {
    width: 100%;
    background: var(--accent);
    border: none;
    border-radius: 8px;
    color: #0a0c10;
    font-family: var(--font-head);
    font-weight: 700;
    font-size: 0.95rem;
    letter-spacing: 0.02em;
    padding: 0.75rem;
    cursor: pointer;
    transition: filter .2s, transform .1s;
    margin-top: 0.5rem;
}
.btn-primary:hover  { filter: brightness(1.1); }
.btn-primary:active { transform: scale(0.98); }

.auth-alt {
    text-align: center;
    margin-top: 1.5rem;
    font-size: 0.85rem;
    color: var(--muted);
}
.auth-alt a { color: var(--accent2); text-decoration: none; }
.auth-alt a:hover { text-decoration: underline; }

/* ── Alerts ────────────────────────────────────────────────── */
.alert {
    border-radius: 8px;
    padding: 0.75rem 1rem;
    font-size: 0.875rem;
    margin-bottom: 1.25rem;
    display: flex; align-items: flex-start; gap: 0.5rem;
}
.alert-error   { background: rgba(240,64,96,0.12);  border: 1px solid rgba(240,64,96,0.3);  color: #f8b4c2; }
.alert-success { background: rgba(64,240,144,0.1);  border: 1px solid rgba(64,240,144,0.25); color: #9af5cc; }

/* ── Home page ─────────────────────────────────────────────── */
.home-grid {
    display: grid;
    grid-template-columns: repeat(auto-fill, minmax(260px, 1fr));
    gap: 1.25rem;
    margin-bottom: 2.5rem;
}
.stat-card {
    background: var(--surface);
    border: 1px solid var(--border);
    border-radius: var(--radius);
    padding: 1.5rem;
    position: relative;
    overflow: hidden;
    transition: border-color .2s;
}
.stat-card:hover { border-color: rgba(232,240,64,0.3); }
.stat-card .icon {
    font-size: 1.5rem;
    margin-bottom: 0.75rem;
}
.stat-card .label {
    font-size: 0.75rem;
    letter-spacing: 0.08em;
    text-transform: uppercase;
    color: var(--muted);
    margin-bottom: 0.25rem;
}
.stat-card .value {
    font-family: var(--font-head);
    font-weight: 800;
    font-size: 1.4rem;
    letter-spacing: -0.02em;
}
.stat-card .glow {
    position: absolute; bottom: -30px; right: -30px;
    width: 100px; height: 100px;
    border-radius: 50%;
    pointer-events: none;
}
.glow-lime  { background: radial-gradient(circle, rgba(232,240,64,0.15), transparent 70%); }
.glow-blue  { background: radial-gradient(circle, rgba(64,168,240,0.15), transparent 70%); }
.glow-green { background: radial-gradient(circle, rgba(64,240,144,0.15), transparent 70%); }

.section-title {
    font-family: var(--font-head);
    font-weight: 700;
    font-size: 1.1rem;
    letter-spacing: -0.02em;
    margin-bottom: 1rem;
    display: flex; align-items: center; gap: 0.5rem;
}
.section-title::after {
    content: '';
    flex: 1;
    height: 1px;
    background: var(--border);
}

/* ── Announcement cards ────────────────────────────────────── */
.announcements { display: flex; flex-direction: column; gap: 1rem; }
.ann-card {
    background: var(--surface);
    border: 1px solid var(--border);
    border-left: 3px solid var(--accent2);
    border-radius: var(--radius);
    padding: 1.25rem 1.5rem;
    transition: border-color .2s, transform .2s;
}
.ann-card:hover { border-color: var(--accent2); transform: translateX(4px); }
.ann-card h3 {
    font-family: var(--font-head);
    font-weight: 700;
    font-size: 1rem;
    margin-bottom: 0.4rem;
}
.ann-card p   { font-size: 0.9rem; color: var(--muted); line-height: 1.65; }
.ann-card .ts { font-size: 0.75rem; color: var(--muted); margin-top: 0.75rem; opacity: 0.6; }

/* ── Admin users table ─────────────────────────────────────── */
.table-wrap {
    background: var(--surface);
    border: 1px solid var(--border);
    border-radius: var(--radius);
    overflow: hidden;
}
table { width: 100%; border-collapse: collapse; }
thead { background: rgba(255,255,255,0.03); }
th {
    font-family: var(--font-head);
    font-weight: 700;
    font-size: 0.75rem;
    letter-spacing: 0.08em;
    text-transform: uppercase;
    color: var(--muted);
    padding: 0.85rem 1.25rem;
    text-align: left;
    border-bottom: 1px solid var(--border);
}
td {
    padding: 0.85rem 1.25rem;
    font-size: 0.875rem;
    border-bottom: 1px solid rgba(31,37,53,0.6);
    color: var(--text);
}
tr:last-child td { border-bottom: none; }
tr:hover td { background: rgba(255,255,255,0.02); }
.badge {
    display: inline-block;
    font-size: 0.7rem;
    font-weight: 500;
    letter-spacing: 0.06em;
    text-transform: uppercase;
    padding: 0.2rem 0.6rem;
    border-radius: 4px;
}
.badge-admin    { background: rgba(232,240,64,0.15); color: var(--accent); }
.badge-employee { background: rgba(64,168,240,0.12); color: var(--accent2); }

/* ── Employee page ─────────────────────────────────────────── */
.emp-hero {
    background: linear-gradient(135deg, var(--surface) 0%, rgba(64,168,240,0.05) 100%);
    border: 1px solid var(--border);
    border-radius: var(--radius);
    padding: 2rem;
    margin-bottom: 2rem;
    display: flex; align-items: center; gap: 1.5rem;
}
.emp-avatar {
    width: 56px; height: 56px;
    border-radius: 50%;
    background: linear-gradient(135deg, var(--accent), var(--accent2));
    display: flex; align-items: center; justify-content: center;
    font-family: var(--font-head);
    font-weight: 800;
    font-size: 1.3rem;
    color: var(--bg);
    flex-shrink: 0;
}
.emp-info h2 {
    font-family: var(--font-head);
    font-weight: 800;
    font-size: 1.25rem;
    letter-spacing: -0.02em;
}
.emp-info span { font-size: 0.85rem; color: var(--muted); }

/* ── Footer ────────────────────────────────────────────────── */
footer {
    text-align: center;
    padding: 1.5rem;
    font-size: 0.78rem;
    color: var(--muted);
    border-top: 1px solid var(--border);
    opacity: 0.7;
}

/* ── Responsive ────────────────────────────────────────────── */
@media (max-width: 640px) {
    nav { padding: 0 1rem; }
    main { padding: 2rem 1rem; }
    .nav-links a { padding: 0.4rem 0.5rem; font-size: 0.8rem; }
    .emp-hero { flex-direction: column; text-align: center; }
}
</style>
</head>
<body>
<div class="shell">

<?php
// ── NAV ──────────────────────────────────────────────────────
if (isLoggedIn()):
    $cur = $page;
?>
<nav>
    <a class="nav-brand" href="index.php?page=home">Nex<span>Corp</span></a>
    <div class="nav-links">
        <a href="index.php?page=home"     class="<?= $cur==='home'     ? 'active' : '' ?>">Home</a>
        <a href="index.php?page=employee" class="<?= $cur==='employee' ? 'active' : '' ?>">Announcements</a>
        <?php if (isAdmin()): ?>
        <a href="index.php?page=admin"    class="<?= $cur==='admin'    ? 'active' : '' ?>">Admin</a>
        <?php endif; ?>
    </div>
    <div class="nav-user">
        <span>Hello, <strong><?= e($_SESSION['username']) ?></strong></span>
        <a href="index.php?page=logout" class="btn-logout">Sign out</a>
    </div>
</nav>
<?php endif; ?>

<main>

<?php
// ╔══════════════════════════════════════════════════════════╗
//  PAGE: LOGIN
// ╚══════════════════════════════════════════════════════════╝
if ($page === 'login'):
?>
<div class="auth-wrap">
  <div class="auth-card">
    <h2>Welcome <em>back</em></h2>
    <p class="sub">Sign in to access your company portal.</p>

    <?php if ($error):  ?><div class="alert alert-error">⚠ <?= e($error) ?></div><?php endif; ?>
    <?php if ($notice): ?><div class="alert alert-success">✓ <?= e($notice) ?></div><?php endif; ?>

    <form method="POST" action="index.php?page=login">
        <div class="field">
            <label>Username</label>
            <input type="text" name="username" placeholder="your_username" required autocomplete="username">
        </div>
        <div class="field">
            <label>Password</label>
            <input type="password" name="password" placeholder="••••••••" required autocomplete="current-password">
        </div>
        <button type="submit" class="btn-primary">Sign In →</button>
    </form>
    <p class="auth-alt">No account yet? <a href="index.php?page=signup">Create one</a></p>
  </div>
</div>

<?php
// ╔══════════════════════════════════════════════════════════╗
//  PAGE: SIGNUP
// ╚══════════════════════════════════════════════════════════╝
elseif ($page === 'signup'):
?>
<div class="auth-wrap">
  <div class="auth-card">
    <h2>Join <em>NexCorp</em></h2>
    <p class="sub">Create your employee account. The first account registered becomes an admin.</p>

    <?php if ($error): ?><div class="alert alert-error">⚠ <?= e($error) ?></div><?php endif; ?>

    <form method="POST" action="index.php?page=signup">
        <div class="field">
            <label>Username</label>
            <input type="text" name="username" placeholder="choose_a_username" required autocomplete="username">
        </div>
        <div class="field">
            <label>Password <span style="font-size:.7rem;color:var(--muted)">(min. 6 chars)</span></label>
            <input type="password" name="password" placeholder="••••••••" required autocomplete="new-password">
        </div>
        <div class="field">
            <label>Job Title</label>
            <input type="text" name="job_title" placeholder="e.g. Software Engineer" required>
        </div>
        <button type="submit" class="btn-primary">Create Account →</button>
    </form>
    <p class="auth-alt">Already have an account? <a href="index.php?page=login">Sign in</a></p>
  </div>
</div>

<?php
// ╔══════════════════════════════════════════════════════════╗
//  PAGE: HOME
// ╚══════════════════════════════════════════════════════════╝
elseif ($page === 'home'):
    $totalUsers = (int)$pdo->query("SELECT COUNT(*) FROM users")->fetchColumn();
    $totalAnns  = (int)$pdo->query("SELECT COUNT(*) FROM announcements")->fetchColumn();
?>
<div class="page-header">
    <h1>Good day, <em><?= e($_SESSION['username']) ?></em></h1>
    <p><?= e($_SESSION['job_title']) ?> &nbsp;·&nbsp; <?= ucfirst($_SESSION['role']) ?></p>
</div>

<?php if ($error):  ?><div class="alert alert-error">⚠ <?= e($error) ?></div><?php endif; ?>

<div class="home-grid">
    <div class="stat-card">
        <div class="icon">👥</div>
        <div class="label">Total Employees</div>
        <div class="value"><?= $totalUsers ?></div>
        <div class="glow glow-lime"></div>
    </div>
    <div class="stat-card">
        <div class="icon">📢</div>
        <div class="label">Announcements</div>
        <div class="value"><?= $totalAnns ?></div>
        <div class="glow glow-blue"></div>
    </div>
    <div class="stat-card">
        <div class="icon">🏷</div>
        <div class="label">Your Role</div>
        <div class="value"><?= ucfirst($_SESSION['role']) ?></div>
        <div class="glow glow-green"></div>
    </div>
</div>

<div class="section-title">Latest Announcements</div>
<div class="announcements">
<?php foreach (array_slice($announcements, 0, 3) as $ann): ?>
    <div class="ann-card">
        <h3><?= e($ann['title']) ?></h3>
        <p><?= e($ann['body']) ?></p>
        <div class="ts"><?= date('M j, Y', strtotime($ann['created_at'])) ?></div>
    </div>
<?php endforeach; ?>
</div>
<p style="margin-top:1rem; font-size:.85rem; color:var(--muted)">
    <a href="index.php?page=employee" style="color:var(--accent2)">View all announcements →</a>
</p>

<?php
// ╔══════════════════════════════════════════════════════════╗
//  PAGE: EMPLOYEE (Announcements)
// ╚══════════════════════════════════════════════════════════╝
elseif ($page === 'employee'):
?>
<div class="emp-hero">
    <div class="emp-avatar"><?= strtoupper(substr($_SESSION['username'], 0, 1)) ?></div>
    <div class="emp-info">
        <h2><?= e($_SESSION['username']) ?></h2>
        <span><?= e($_SESSION['job_title']) ?></span>
    </div>
</div>

<div class="page-header">
    <h1>Company <em>Announcements</em></h1>
    <p>Stay informed with the latest updates from leadership.</p>
</div>

<?php if (empty($announcements)): ?>
    <p style="color:var(--muted)">No announcements yet.</p>
<?php else: ?>
<div class="announcements">
    <?php foreach ($announcements as $ann): ?>
    <div class="ann-card">
        <h3><?= e($ann['title']) ?></h3>
        <p><?= e($ann['body']) ?></p>
        <div class="ts">Posted <?= date('F j, Y \a\t g:i A', strtotime($ann['created_at'])) ?></div>
    </div>
    <?php endforeach; ?>
</div>
<?php endif; ?>

<?php
// ╔══════════════════════════════════════════════════════════╗
//  PAGE: ADMIN
// ╚══════════════════════════════════════════════════════════╝
elseif ($page === 'admin' && isAdmin()):
?>
<div class="page-header">
    <h1>Admin <em>Panel</em></h1>
    <p>Manage all registered users in the system.</p>
</div>

<div class="section-title">Registered Users (<?= count($allUsers) ?>)</div>
<div class="table-wrap">
    <table>
        <thead>
            <tr>
                <th>#</th>
                <th>Username</th>
                <th>Job Title</th>
                <th>Role</th>
                <th>Joined</th>
            </tr>
        </thead>
        <tbody>
        <?php foreach ($allUsers as $i => $u): ?>
            <tr>
                <td style="color:var(--muted)"><?= $i + 1 ?></td>
                <td><strong><?= e($u['username']) ?></strong></td>
                <td><?= e($u['job_title']) ?></td>
                <td><span class="badge badge-<?= $u['role'] ?>"><?= ucfirst($u['role']) ?></span></td>
                <td style="color:var(--muted)"><?= date('M j, Y', strtotime($u['created_at'])) ?></td>
            </tr>
        <?php endforeach; ?>
        </tbody>
    </table>
</div>

<?php endif; ?>
</main>

<footer>NexCorp Internal Portal &nbsp;·&nbsp; <?= date('Y') ?></footer>
</div><!-- .shell -->
</body>
</html>