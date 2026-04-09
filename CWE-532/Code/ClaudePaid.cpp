<?php
// ============================================================
// CONFIGURATION
// ============================================================
define('LOG_FILE', __DIR__ . '/activity.log');
define('APP_NAME', 'Nexus Corp');

// Demo credentials (in production, use a database + hashed passwords)
$valid_users = [
    'admin'   => ['password' => 'admin123',  'role' => 'Administrator', 'name' => 'Alex Morgan'],
    'jsmith'  => ['password' => 'pass456',   'role' => 'Manager',       'name' => 'Jordan Smith'],
    'viewer'  => ['password' => 'view789',   'role' => 'Viewer',        'name' => 'Casey Lee'],
];

// ============================================================
// SESSION & HELPERS
// ============================================================
session_start();

function log_action(string $action, string $detail = ''): void {
    $ts      = date('Y-m-d H:i:s');
    $user    = $_SESSION['username'] ?? 'guest';
    $ip      = $_SERVER['REMOTE_ADDR'] ?? 'unknown';
    $line    = "[{$ts}] USER={$user} IP={$ip} ACTION={$action}" . ($detail ? " | {$detail}" : '') . PHP_EOL;
    file_put_contents(LOG_FILE, $line, FILE_APPEND | LOCK_EX);
}

function is_logged_in(): bool {
    return !empty($_SESSION['username']);
}

function redirect_to(string $page): void {
    header("Location: ?page={$page}");
    exit;
}

function h(string $s): string {
    return htmlspecialchars($s, ENT_QUOTES, 'UTF-8');
}

// ============================================================
// ROUTING / ACTIONS
// ============================================================
$page    = $_GET['page'] ?? 'login';
$error   = '';
$success = '';

// --- Handle Login ---
if ($page === 'login' && $_SERVER['REQUEST_METHOD'] === 'POST') {
    $username = trim($_POST['username'] ?? '');
    $password = $_POST['password'] ?? '';

    if (isset($valid_users[$username]) && $valid_users[$username]['password'] === $password) {
        session_regenerate_id(true);
        $_SESSION['username']   = $username;
        $_SESSION['name']       = $valid_users[$username]['name'];
        $_SESSION['role']       = $valid_users[$username]['role'];
        $_SESSION['login_time'] = date('Y-m-d H:i:s');
        log_action('LOGIN_SUCCESS', "name={$valid_users[$username]['name']} role={$valid_users[$username]['role']}");
        redirect_to('dashboard');
    } else {
        log_action('LOGIN_FAILED', "attempted_user=" . h($username));
        $error = 'Invalid username or password.';
    }
}

// --- Handle Logout ---
if ($page === 'logout') {
    log_action('LOGOUT');
    session_destroy();
    redirect_to('login');
}

// --- Guard protected pages ---
$protected = ['dashboard', 'about', 'team', 'financials', 'logs'];
if (in_array($page, $protected) && !is_logged_in()) {
    log_action('UNAUTHORIZED_ACCESS', "attempted_page={$page}");
    redirect_to('login');
}

// --- Log page views for authenticated users ---
if (is_logged_in() && in_array($page, $protected) && $_SERVER['REQUEST_METHOD'] === 'GET') {
    log_action('PAGE_VIEW', "page={$page}");
}

// ============================================================
// PAGE DATA
// ============================================================
$team_members = [
    ['name' => 'Diana Reyes',    'role' => 'Chief Executive Officer',     'since' => '2014', 'img' => 'DR'],
    ['name' => 'Marcus Webb',    'role' => 'Chief Technology Officer',     'since' => '2016', 'img' => 'MW'],
    ['name' => 'Priya Nair',     'role' => 'Chief Financial Officer',      'since' => '2017', 'img' => 'PN'],
    ['name' => 'Lena Hoffmann',  'role' => 'VP of Product',               'since' => '2019', 'img' => 'LH'],
    ['name' => 'Carlos Mendez',  'role' => 'VP of Engineering',           'since' => '2018', 'img' => 'CM'],
    ['name' => 'Aisha Okonkwo',  'role' => 'Head of Design',              'since' => '2020', 'img' => 'AO'],
];

$financials = [
    ['year' => '2021', 'revenue' => '$18.4M', 'profit' => '$3.1M', 'growth' => '+12%', 'employees' => '142'],
    ['year' => '2022', 'revenue' => '$24.7M', 'profit' => '$4.8M', 'growth' => '+34%', 'employees' => '198'],
    ['year' => '2023', 'revenue' => '$31.2M', 'profit' => '$6.2M', 'growth' => '+26%', 'employees' => '253'],
    ['year' => '2024', 'revenue' => '$40.5M', 'profit' => '$9.1M', 'growth' => '+30%', 'employees' => '317'],
];

// --- Read log file ---
$log_lines = [];
if ($page === 'logs' && is_logged_in()) {
    if (file_exists(LOG_FILE)) {
        $raw = file_get_contents(LOG_FILE);
        $log_lines = array_filter(array_reverse(explode(PHP_EOL, $raw)));
    }
}
?>
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title><?= APP_NAME ?> &mdash; <?= ucfirst(h($page)) ?></title>
<link rel="preconnect" href="https://fonts.googleapis.com">
<link href="https://fonts.googleapis.com/css2?family=Syne:wght@400;600;700;800&family=DM+Mono:wght@300;400;500&display=swap" rel="stylesheet">
<style>
/* ── RESET & BASE ───────────────────────────────────────── */
*, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }

:root {
    --bg:       #0a0b0f;
    --surface:  #111318;
    --border:   #1e2129;
    --accent:   #e8ff47;
    --accent2:  #47ffe8;
    --text:     #e8eaf0;
    --muted:    #5a5f72;
    --danger:   #ff4757;
    --success:  #2ed573;
    --font-head: 'Syne', sans-serif;
    --font-mono: 'DM Mono', monospace;
    --radius:   6px;
    --transition: 200ms ease;
}

html, body {
    min-height: 100vh;
    background: var(--bg);
    color: var(--text);
    font-family: var(--font-mono);
    font-size: 14px;
    line-height: 1.6;
}

a { color: var(--accent); text-decoration: none; }
a:hover { text-decoration: underline; }

/* ── SCROLLBAR ─────────────────────────────────────────── */
::-webkit-scrollbar { width: 6px; }
::-webkit-scrollbar-track { background: var(--bg); }
::-webkit-scrollbar-thumb { background: var(--border); border-radius: 3px; }

/* ── LOGIN PAGE ─────────────────────────────────────────── */
.login-wrap {
    min-height: 100vh;
    display: flex;
    align-items: center;
    justify-content: center;
    padding: 2rem;
    background:
        radial-gradient(ellipse 80% 60% at 50% -10%, rgba(232,255,71,0.08) 0%, transparent 60%),
        var(--bg);
}

.login-card {
    width: 100%;
    max-width: 420px;
    background: var(--surface);
    border: 1px solid var(--border);
    border-radius: 12px;
    padding: 3rem 2.5rem;
    animation: fadeUp .4s ease both;
}

.login-logo {
    font-family: var(--font-head);
    font-size: 2rem;
    font-weight: 800;
    letter-spacing: -1px;
    color: var(--accent);
    margin-bottom: .25rem;
}
.login-sub {
    color: var(--muted);
    font-size: .8rem;
    letter-spacing: .12em;
    text-transform: uppercase;
    margin-bottom: 2.5rem;
}

.form-group { margin-bottom: 1.25rem; }
.form-group label {
    display: block;
    font-size: .75rem;
    letter-spacing: .1em;
    text-transform: uppercase;
    color: var(--muted);
    margin-bottom: .5rem;
}
.form-group input {
    width: 100%;
    padding: .75rem 1rem;
    background: var(--bg);
    border: 1px solid var(--border);
    border-radius: var(--radius);
    color: var(--text);
    font-family: var(--font-mono);
    font-size: .9rem;
    outline: none;
    transition: border-color var(--transition);
}
.form-group input:focus { border-color: var(--accent); }

.btn-primary {
    width: 100%;
    padding: .85rem 1rem;
    background: var(--accent);
    color: #0a0b0f;
    border: none;
    border-radius: var(--radius);
    font-family: var(--font-head);
    font-size: .95rem;
    font-weight: 700;
    letter-spacing: .05em;
    cursor: pointer;
    transition: opacity var(--transition), transform var(--transition);
    margin-top: .5rem;
}
.btn-primary:hover { opacity: .88; transform: translateY(-1px); }
.btn-primary:active { transform: translateY(0); }

.alert {
    padding: .75rem 1rem;
    border-radius: var(--radius);
    font-size: .85rem;
    margin-bottom: 1.25rem;
}
.alert-error   { background: rgba(255,71,87,.12); border: 1px solid rgba(255,71,87,.3); color: var(--danger); }
.alert-success { background: rgba(46,213,115,.12); border: 1px solid rgba(46,213,115,.3); color: var(--success); }

.demo-creds {
    margin-top: 1.75rem;
    padding: 1rem;
    background: rgba(255,255,255,.02);
    border: 1px dashed var(--border);
    border-radius: var(--radius);
    font-size: .78rem;
    color: var(--muted);
    line-height: 1.8;
}
.demo-creds strong { color: var(--accent2); }

/* ── APP SHELL ──────────────────────────────────────────── */
.shell { display: flex; min-height: 100vh; }

/* Sidebar */
.sidebar {
    width: 220px;
    flex-shrink: 0;
    background: var(--surface);
    border-right: 1px solid var(--border);
    display: flex;
    flex-direction: column;
    padding: 2rem 0;
    position: fixed;
    top: 0; left: 0;
    height: 100vh;
    overflow-y: auto;
}

.sidebar-logo {
    font-family: var(--font-head);
    font-size: 1.3rem;
    font-weight: 800;
    color: var(--accent);
    padding: 0 1.5rem 2rem;
    letter-spacing: -0.5px;
}

.nav-label {
    font-size: .65rem;
    letter-spacing: .15em;
    text-transform: uppercase;
    color: var(--muted);
    padding: 0 1.5rem .5rem;
    margin-top: 1rem;
}

.nav-item {
    display: flex;
    align-items: center;
    gap: .75rem;
    padding: .6rem 1.5rem;
    color: var(--muted);
    text-decoration: none;
    font-size: .85rem;
    letter-spacing: .03em;
    border-left: 2px solid transparent;
    transition: color var(--transition), border-color var(--transition), background var(--transition);
}
.nav-item:hover {
    color: var(--text);
    background: rgba(255,255,255,.03);
    text-decoration: none;
}
.nav-item.active {
    color: var(--accent);
    border-left-color: var(--accent);
    background: rgba(232,255,71,.05);
}
.nav-item svg { flex-shrink: 0; }

.sidebar-bottom {
    margin-top: auto;
    padding: 1.5rem;
    border-top: 1px solid var(--border);
}
.user-chip {
    display: flex;
    align-items: center;
    gap: .75rem;
    margin-bottom: 1rem;
}
.user-avatar {
    width: 34px; height: 34px;
    border-radius: 50%;
    background: linear-gradient(135deg, var(--accent), var(--accent2));
    display: flex; align-items: center; justify-content: center;
    font-family: var(--font-head);
    font-weight: 800;
    font-size: .7rem;
    color: #0a0b0f;
    flex-shrink: 0;
}
.user-info { overflow: hidden; }
.user-name {
    font-size: .8rem;
    font-weight: 500;
    color: var(--text);
    white-space: nowrap;
    overflow: hidden;
    text-overflow: ellipsis;
}
.user-role { font-size: .7rem; color: var(--muted); }

.btn-logout {
    display: block;
    width: 100%;
    padding: .5rem;
    background: transparent;
    border: 1px solid var(--border);
    border-radius: var(--radius);
    color: var(--muted);
    font-family: var(--font-mono);
    font-size: .78rem;
    cursor: pointer;
    text-align: center;
    transition: border-color var(--transition), color var(--transition);
    text-decoration: none;
}
.btn-logout:hover { border-color: var(--danger); color: var(--danger); text-decoration: none; }

/* Main content */
.main {
    margin-left: 220px;
    flex: 1;
    min-height: 100vh;
    display: flex;
    flex-direction: column;
}

.topbar {
    padding: 1.5rem 2.5rem;
    border-bottom: 1px solid var(--border);
    display: flex;
    align-items: center;
    justify-content: space-between;
    background: var(--bg);
    position: sticky;
    top: 0;
    z-index: 10;
}
.page-title {
    font-family: var(--font-head);
    font-size: 1.25rem;
    font-weight: 700;
    color: var(--text);
}
.topbar-meta {
    font-size: .75rem;
    color: var(--muted);
}
.topbar-meta span { color: var(--accent2); }

.content { padding: 2.5rem; flex: 1; }

/* ── CARDS & GRIDS ──────────────────────────────────────── */
.card {
    background: var(--surface);
    border: 1px solid var(--border);
    border-radius: 10px;
    padding: 1.75rem;
    margin-bottom: 1.5rem;
}
.card-title {
    font-family: var(--font-head);
    font-size: 1rem;
    font-weight: 700;
    color: var(--text);
    margin-bottom: 1rem;
    display: flex;
    align-items: center;
    gap: .5rem;
}
.card-title::before {
    content: '';
    display: inline-block;
    width: 3px;
    height: 1rem;
    background: var(--accent);
    border-radius: 2px;
}

.stats-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(180px, 1fr));
    gap: 1rem;
    margin-bottom: 1.5rem;
}
.stat-card {
    background: var(--surface);
    border: 1px solid var(--border);
    border-radius: 10px;
    padding: 1.25rem 1.5rem;
    transition: border-color var(--transition);
}
.stat-card:hover { border-color: var(--accent); }
.stat-label { font-size: .7rem; letter-spacing: .1em; text-transform: uppercase; color: var(--muted); margin-bottom: .4rem; }
.stat-value { font-family: var(--font-head); font-size: 1.8rem; font-weight: 800; color: var(--accent); }
.stat-sub { font-size: .75rem; color: var(--muted); margin-top: .2rem; }

/* Team grid */
.team-grid {
    display: grid;
    grid-template-columns: repeat(auto-fill, minmax(220px, 1fr));
    gap: 1rem;
}
.team-card {
    background: var(--surface);
    border: 1px solid var(--border);
    border-radius: 10px;
    padding: 1.5rem;
    text-align: center;
    transition: border-color var(--transition), transform var(--transition);
}
.team-card:hover { border-color: var(--accent2); transform: translateY(-3px); }
.team-avatar {
    width: 60px; height: 60px;
    border-radius: 50%;
    background: linear-gradient(135deg, var(--accent), var(--accent2));
    display: flex; align-items: center; justify-content: center;
    font-family: var(--font-head);
    font-weight: 800;
    font-size: .9rem;
    color: #0a0b0f;
    margin: 0 auto 1rem;
}
.team-name { font-family: var(--font-head); font-weight: 700; font-size: .95rem; color: var(--text); }
.team-role { font-size: .75rem; color: var(--muted); margin-top: .25rem; }
.team-since {
    display: inline-block;
    margin-top: .75rem;
    font-size: .7rem;
    padding: .2rem .6rem;
    background: rgba(232,255,71,.08);
    border: 1px solid rgba(232,255,71,.15);
    border-radius: 20px;
    color: var(--accent);
}

/* Table */
.data-table { width: 100%; border-collapse: collapse; }
.data-table th {
    text-align: left;
    font-size: .7rem;
    letter-spacing: .1em;
    text-transform: uppercase;
    color: var(--muted);
    padding: .75rem 1rem;
    border-bottom: 1px solid var(--border);
}
.data-table td {
    padding: .85rem 1rem;
    border-bottom: 1px solid rgba(30,33,41,.6);
    font-size: .85rem;
    color: var(--text);
}
.data-table tr:last-child td { border-bottom: none; }
.data-table tr:hover td { background: rgba(255,255,255,.02); }
.badge {
    display: inline-block;
    padding: .2rem .65rem;
    border-radius: 20px;
    font-size: .7rem;
    font-weight: 500;
}
.badge-green { background: rgba(46,213,115,.12); color: var(--success); border: 1px solid rgba(46,213,115,.2); }

/* Log viewer */
.log-viewer {
    background: #07080c;
    border: 1px solid var(--border);
    border-radius: var(--radius);
    padding: 1.25rem;
    max-height: 550px;
    overflow-y: auto;
    font-family: var(--font-mono);
    font-size: .78rem;
    line-height: 1.8;
}
.log-line { padding: .15rem 0; border-bottom: 1px solid rgba(255,255,255,.02); }
.log-line:last-child { border-bottom: none; }
.log-ts { color: var(--muted); }
.log-user { color: var(--accent2); }
.log-action { color: var(--accent); font-weight: 500; }
.log-detail { color: #8890aa; }

/* Dashboard specific */
.welcome-bar {
    background: linear-gradient(135deg, rgba(232,255,71,.06), rgba(71,255,232,.03));
    border: 1px solid rgba(232,255,71,.12);
    border-radius: 10px;
    padding: 1.5rem 2rem;
    margin-bottom: 1.75rem;
    display: flex;
    align-items: center;
    justify-content: space-between;
}
.welcome-text { font-family: var(--font-head); font-size: 1.1rem; font-weight: 700; }
.welcome-text span { color: var(--accent); }
.welcome-sub { font-size: .8rem; color: var(--muted); margin-top: .25rem; }
.online-dot {
    width: 8px; height: 8px;
    border-radius: 50%;
    background: var(--success);
    display: inline-block;
    margin-right: .4rem;
    animation: pulse 2s infinite;
}

/* About page */
.about-hero {
    background: linear-gradient(135deg, rgba(232,255,71,.05), rgba(71,255,232,.03));
    border: 1px solid var(--border);
    border-radius: 12px;
    padding: 2.5rem;
    margin-bottom: 1.5rem;
}
.about-hero h2 {
    font-family: var(--font-head);
    font-size: 2rem;
    font-weight: 800;
    color: var(--accent);
    margin-bottom: .5rem;
}
.about-hero p { color: #8890aa; line-height: 1.8; max-width: 680px; }

.value-grid {
    display: grid;
    grid-template-columns: repeat(auto-fill, minmax(200px, 1fr));
    gap: 1rem;
    margin-top: 1.25rem;
}
.value-card {
    padding: 1.25rem;
    background: var(--bg);
    border: 1px solid var(--border);
    border-radius: var(--radius);
}
.value-icon { font-size: 1.4rem; margin-bottom: .5rem; }
.value-title { font-family: var(--font-head); font-size: .85rem; font-weight: 700; margin-bottom: .25rem; }
.value-desc { font-size: .78rem; color: var(--muted); }

/* ── ANIMATIONS ─────────────────────────────────────────── */
@keyframes fadeUp {
    from { opacity: 0; transform: translateY(16px); }
    to   { opacity: 1; transform: translateY(0); }
}
@keyframes pulse {
    0%, 100% { opacity: 1; }
    50%       { opacity: .4; }
}
.animate { animation: fadeUp .35s ease both; }
.animate-d1 { animation-delay: .05s; }
.animate-d2 { animation-delay: .10s; }
.animate-d3 { animation-delay: .15s; }
.animate-d4 { animation-delay: .20s; }

/* ── RESPONSIVE ─────────────────────────────────────────── */
@media (max-width: 768px) {
    .sidebar { width: 100%; height: auto; position: relative; flex-direction: row; flex-wrap: wrap; padding: 1rem; }
    .sidebar-bottom { display: none; }
    .main { margin-left: 0; }
    .stats-grid { grid-template-columns: 1fr 1fr; }
    .content { padding: 1.25rem; }
    .topbar { padding: 1rem 1.25rem; }
    .welcome-bar { flex-direction: column; gap: .75rem; }
}
</style>
</head>
<body>

<?php if ($page === 'login'): ?>
<!-- ═══════════════════════════════════════════════════════
     LOGIN PAGE
════════════════════════════════════════════════════════ -->
<div class="login-wrap">
    <div class="login-card">
        <div class="login-logo"><?= APP_NAME ?></div>
        <div class="login-sub">Secure Portal &mdash; Sign In</div>

        <?php if ($error): ?>
            <div class="alert alert-error"><?= h($error) ?></div>
        <?php endif; ?>

        <form method="POST" action="?page=login">
            <div class="form-group">
                <label for="username">Username</label>
                <input type="text" id="username" name="username"
                       value="<?= h($_POST['username'] ?? '') ?>"
                       placeholder="your.username" required autofocus>
            </div>
            <div class="form-group">
                <label for="password">Password</label>
                <input type="password" id="password" name="password"
                       placeholder="••••••••" required>
            </div>
            <button type="submit" class="btn-primary">Sign In &rarr;</button>
        </form>

        <div class="demo-creds">
            <strong>Demo credentials</strong><br>
            admin / admin123 &nbsp;&bull;&nbsp; jsmith / pass456 &nbsp;&bull;&nbsp; viewer / view789
        </div>
    </div>
</div>

<?php else: ?>
<!-- ═══════════════════════════════════════════════════════
     APP SHELL (authenticated pages)
════════════════════════════════════════════════════════ -->
<?php
$nav = [
    'dashboard'  => ['label' => 'Dashboard',  'icon' => '<svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><rect x="3" y="3" width="7" height="7"/><rect x="14" y="3" width="7" height="7"/><rect x="3" y="14" width="7" height="7"/><rect x="14" y="14" width="7" height="7"/></svg>'],
    'about'      => ['label' => 'About Us',   'icon' => '<svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="12" cy="12" r="10"/><line x1="12" y1="8" x2="12" y2="12"/><line x1="12" y1="16" x2="12.01" y2="16"/></svg>'],
    'team'       => ['label' => 'Our Team',   'icon' => '<svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M17 21v-2a4 4 0 0 0-4-4H5a4 4 0 0 0-4 4v2"/><circle cx="9" cy="7" r="4"/><path d="M23 21v-2a4 4 0 0 0-3-3.87"/><path d="M16 3.13a4 4 0 0 1 0 7.75"/></svg>'],
    'financials' => ['label' => 'Financials', 'icon' => '<svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><line x1="12" y1="1" x2="12" y2="23"/><path d="M17 5H9.5a3.5 3.5 0 0 0 0 7h5a3.5 3.5 0 0 1 0 7H6"/></svg>'],
    'logs'       => ['label' => 'Activity Log','icon' => '<svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M14 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V8z"/><polyline points="14 2 14 8 20 8"/><line x1="16" y1="13" x2="8" y2="13"/><line x1="16" y1="17" x2="8" y2="17"/></svg>'],
];
$initials = strtoupper(implode('', array_map(fn($p) => $p[0], explode(' ', $_SESSION['name']))));
$page_titles = [
    'dashboard'  => 'Dashboard',
    'about'      => 'About Us',
    'team'       => 'Our Team',
    'financials' => 'Financial Overview',
    'logs'       => 'Activity Log',
];
?>
<div class="shell">
    <!-- SIDEBAR -->
    <aside class="sidebar">
        <div class="sidebar-logo"><?= APP_NAME ?></div>
        <div class="nav-label">Navigation</div>
        <?php foreach ($nav as $p => $n): ?>
            <a href="?page=<?= $p ?>" class="nav-item <?= $page === $p ? 'active' : '' ?>">
                <?= $n['icon'] ?> <?= $n['label'] ?>
            </a>
        <?php endforeach; ?>

        <div class="sidebar-bottom">
            <div class="user-chip">
                <div class="user-avatar"><?= h($initials) ?></div>
                <div class="user-info">
                    <div class="user-name"><?= h($_SESSION['name']) ?></div>
                    <div class="user-role"><?= h($_SESSION['role']) ?></div>
                </div>
            </div>
            <a href="?page=logout" class="btn-logout">Sign Out</a>
        </div>
    </aside>

    <!-- MAIN -->
    <div class="main">
        <!-- Top bar -->
        <header class="topbar">
            <div class="page-title"><?= h($page_titles[$page] ?? ucfirst($page)) ?></div>
            <div class="topbar-meta">
                <span class="online-dot"></span>
                Logged in as <span><?= h($_SESSION['name']) ?></span>
                &nbsp;&bull;&nbsp; Since <?= h($_SESSION['login_time']) ?>
                &nbsp;&bull;&nbsp; <a href="?page=logout" style="color:var(--muted)">Logout</a>
            </div>
        </header>

        <div class="content">

        <?php if ($page === 'dashboard'): ?>
        <!-- ─── DASHBOARD ─────────────────────────────── -->
        <div class="welcome-bar animate">
            <div>
                <div class="welcome-text">Welcome back, <span><?= h(explode(' ', $_SESSION['name'])[0]) ?></span> 👋</div>
                <div class="welcome-sub">Here's a quick overview of <?= APP_NAME ?> today.</div>
            </div>
            <div style="text-align:right;font-size:.8rem;color:var(--muted)">
                <?= date('l, F j Y') ?><br>
                <span style="color:var(--accent2)"><?= date('H:i') ?> local time</span>
            </div>
        </div>

        <div class="stats-grid">
            <div class="stat-card animate animate-d1">
                <div class="stat-label">Annual Revenue</div>
                <div class="stat-value">$40.5M</div>
                <div class="stat-sub">↑ 30% from 2023</div>
            </div>
            <div class="stat-card animate animate-d2">
                <div class="stat-label">Employees</div>
                <div class="stat-value">317</div>
                <div class="stat-sub">Across 6 offices</div>
            </div>
            <div class="stat-card animate animate-d3">
                <div class="stat-label">Founded</div>
                <div class="stat-value">2012</div>
                <div class="stat-sub">12+ years in business</div>
            </div>
            <div class="stat-card animate animate-d4">
                <div class="stat-label">Net Profit</div>
                <div class="stat-value">$9.1M</div>
                <div class="stat-sub">2024 fiscal year</div>
            </div>
        </div>

        <div class="card animate">
            <div class="card-title">Quick Links</div>
            <div style="display:flex;gap:.75rem;flex-wrap:wrap;">
                <?php foreach ($nav as $p => $n): if ($p === 'dashboard') continue; ?>
                <a href="?page=<?= $p ?>" style="
                    padding:.6rem 1.2rem;
                    background:rgba(255,255,255,.04);
                    border:1px solid var(--border);
                    border-radius:var(--radius);
                    color:var(--text);
                    font-size:.82rem;
                    display:flex;align-items:center;gap:.5rem;
                    transition:border-color .2s,color .2s;
                    text-decoration:none;
                " onmouseover="this.style.borderColor='var(--accent)';this.style.color='var(--accent)'"
                   onmouseout="this.style.borderColor='var(--border)';this.style.color='var(--text)'">
                    <?= $n['icon'] ?> <?= $n['label'] ?>
                </a>
                <?php endforeach; ?>
            </div>
        </div>

        <?php elseif ($page === 'about'): ?>
        <!-- ─── ABOUT ──────────────────────────────────── -->
        <div class="about-hero animate">
            <h2><?= APP_NAME ?></h2>
            <p>Founded in 2012, <?= APP_NAME ?> is a technology-forward enterprise solutions company headquartered in San Francisco, CA. We specialize in cloud infrastructure, data intelligence, and enterprise automation — helping businesses scale with confidence and clarity.</p>
        </div>

        <div class="card animate animate-d1">
            <div class="card-title">Our Mission</div>
            <p style="color:#8890aa;line-height:1.9;font-size:.88rem">
                To empower organizations with intelligent, scalable systems that turn data into decisions and complexity into clarity. We believe technology should work for people — not the other way around.
            </p>
        </div>

        <div class="card animate animate-d2">
            <div class="card-title">Core Values</div>
            <div class="value-grid">
                <div class="value-card"><div class="value-icon">🔬</div><div class="value-title">Innovation First</div><div class="value-desc">We invest heavily in R&D and encourage bold experimentation.</div></div>
                <div class="value-card"><div class="value-icon">🤝</div><div class="value-title">Client Trust</div><div class="value-desc">Long-term partnerships built on transparency and results.</div></div>
                <div class="value-card"><div class="value-icon">🌍</div><div class="value-title">Global Impact</div><div class="value-desc">Operations in 14 countries, serving 800+ enterprise clients.</div></div>
                <div class="value-card"><div class="value-icon">⚡</div><div class="value-title">Speed & Quality</div><div class="value-desc">Rapid delivery without compromising on engineering standards.</div></div>
            </div>
        </div>

        <div class="card animate animate-d3">
            <div class="card-title">Key Facts</div>
            <table class="data-table">
                <tr><td style="color:var(--muted);width:160px">Headquarters</td><td>San Francisco, CA</td></tr>
                <tr><td style="color:var(--muted)">Founded</td><td>2012</td></tr>
                <tr><td style="color:var(--muted)">Industry</td><td>Enterprise Technology</td></tr>
                <tr><td style="color:var(--muted)">Offices</td><td>San Francisco · New York · London · Berlin · Singapore · Sydney</td></tr>
                <tr><td style="color:var(--muted)">Clients</td><td>800+ across 14 countries</td></tr>
                <tr><td style="color:var(--muted)">Certifications</td><td>ISO 27001 &nbsp;·&nbsp; SOC 2 Type II &nbsp;·&nbsp; GDPR Compliant</td></tr>
            </table>
        </div>

        <?php elseif ($page === 'team'): ?>
        <!-- ─── TEAM ───────────────────────────────────── -->
        <div class="card animate" style="margin-bottom:1.75rem">
            <div class="card-title">Leadership Team</div>
            <p style="color:var(--muted);font-size:.85rem">Meet the people who drive <?= APP_NAME ?> forward.</p>
        </div>
        <div class="team-grid">
            <?php foreach ($team_members as $i => $m): ?>
            <div class="team-card animate" style="animation-delay:<?= $i * 0.06 ?>s">
                <div class="team-avatar"><?= h($m['img']) ?></div>
                <div class="team-name"><?= h($m['name']) ?></div>
                <div class="team-role"><?= h($m['role']) ?></div>
                <div class="team-since">Since <?= h($m['since']) ?></div>
            </div>
            <?php endforeach; ?>
        </div>

        <?php elseif ($page === 'financials'): ?>
        <!-- ─── FINANCIALS ────────────────────────────── -->
        <div class="stats-grid">
            <div class="stat-card animate"><div class="stat-label">2024 Revenue</div><div class="stat-value">$40.5M</div><div class="stat-sub">Record year</div></div>
            <div class="stat-card animate animate-d1"><div class="stat-label">2024 Profit</div><div class="stat-value">$9.1M</div><div class="stat-sub">22.5% margin</div></div>
            <div class="stat-card animate animate-d2"><div class="stat-label">YoY Growth</div><div class="stat-value">+30%</div><div class="stat-sub">vs. 2023</div></div>
            <div class="stat-card animate animate-d3"><div class="stat-label">Headcount</div><div class="stat-value">317</div><div class="stat-sub">+25% this year</div></div>
        </div>

        <div class="card animate animate-d1">
            <div class="card-title">Annual Performance</div>
            <table class="data-table">
                <thead>
                    <tr>
                        <th>Year</th><th>Revenue</th><th>Net Profit</th><th>Growth</th><th>Employees</th><th>Status</th>
                    </tr>
                </thead>
                <tbody>
                    <?php foreach ($financials as $f): ?>
                    <tr>
                        <td style="font-family:var(--font-head);font-weight:700"><?= h($f['year']) ?></td>
                        <td style="color:var(--accent)"><?= h($f['revenue']) ?></td>
                        <td><?= h($f['profit']) ?></td>
                        <td><span class="badge badge-green"><?= h($f['growth']) ?></span></td>
                        <td><?= h($f['employees']) ?></td>
                        <td><span style="color:var(--success);font-size:.75rem">● Audited</span></td>
                    </tr>
                    <?php endforeach; ?>
                </tbody>
            </table>
        </div>

        <div class="card animate animate-d2">
            <div class="card-title">Revenue Breakdown (2024)</div>
            <div style="display:grid;grid-template-columns:repeat(auto-fill,minmax(180px,1fr));gap:1rem;margin-top:.5rem">
                <?php
                $segments = ['Cloud Services' => '48%', 'Enterprise Licenses' => '31%', 'Professional Services' => '14%', 'Support & Training' => '7%'];
                foreach ($segments as $seg => $pct):
                ?>
                <div style="background:var(--bg);border:1px solid var(--border);border-radius:var(--radius);padding:1rem">
                    <div style="font-size:.7rem;letter-spacing:.08em;text-transform:uppercase;color:var(--muted);margin-bottom:.3rem"><?= h($seg) ?></div>
                    <div style="font-family:var(--font-head);font-size:1.4rem;font-weight:800;color:var(--accent2)"><?= h($pct) ?></div>
                </div>
                <?php endforeach; ?>
            </div>
        </div>

        <?php elseif ($page === 'logs'): ?>
        <!-- ─── LOGS ───────────────────────────────────── -->
        <div class="card animate">
            <div class="card-title">Activity Log</div>
            <p style="color:var(--muted);font-size:.82rem;margin-bottom:1.25rem">
                All user actions are recorded below. Most recent entries appear first.
            </p>

            <?php if (empty($log_lines)): ?>
                <div style="color:var(--muted);font-size:.85rem">No log entries found.</div>
            <?php else: ?>
            <div class="log-viewer">
                <?php foreach ($log_lines as $line):
                    // Parse: [2024-01-01 12:00:00] USER=admin IP=127.0.0.1 ACTION=LOGIN_SUCCESS | ...
                    preg_match('/\[([^\]]+)\]/', $line, $tsM);
                    preg_match('/USER=(\S+)/', $line, $uM);
                    preg_match('/ACTION=(\S+)/', $line, $aM);
                    preg_match('/\| (.+)$/', $line, $dM);
                    $ts     = $tsM[1] ?? '';
                    $user   = $uM[1]  ?? '';
                    $action = $aM[1]  ?? '';
                    $detail = $dM[1]  ?? '';
                ?>
                <div class="log-line">
                    <span class="log-ts">[<?= h($ts) ?>]</span>
                    <span class="log-user"> <?= h($user) ?></span>
                    &rarr;
                    <span class="log-action"><?= h($action) ?></span>
                    <?php if ($detail): ?>
                        <span class="log-detail"> // <?= h($detail) ?></span>
                    <?php endif; ?>
                </div>
                <?php endforeach; ?>
            </div>
            <div style="margin-top:.75rem;font-size:.75rem;color:var(--muted)">
                Showing <?= count($log_lines) ?> entries &bull; Log file: <code style="color:var(--accent2)"><?= h(LOG_FILE) ?></code>
            </div>
            <?php endif; ?>
        </div>

        <?php endif; ?>

        </div><!-- /content -->
    </div><!-- /main -->
</div><!-- /shell -->
<?php endif; ?>

</body>
</html>