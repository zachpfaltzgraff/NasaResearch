<?php
// ============================================================
//  CONFIGURATION
// ============================================================
define('DB_HOST', 'localhost');
define('DB_USER', 'root');        // Change if needed
define('DB_PASS', '');            // Change if needed
define('DB_NAME', 'company_portal');

session_start();

// ============================================================
//  DATABASE CONNECTION
// ============================================================
function db(): mysqli {
    static $conn = null;
    if ($conn === null) {
        $conn = new mysqli(DB_HOST, DB_USER, DB_PASS, DB_NAME);
        if ($conn->connect_error) {
            die('<div style="font-family:monospace;padding:2rem;color:#ff4444;">
                Database connection failed: ' . $conn->connect_error . '<br>
                Make sure WampServer is running and you have run setup_db.sql
            </div>');
        }
        $conn->set_charset('utf8mb4');
    }
    return $conn;
}

// ============================================================
//  HELPERS
// ============================================================
function redirect(string $page, array $params = []): void {
    $query = array_merge(['page' => $page], $params);
    header('Location: index.php?' . http_build_query($query));
    exit;
}

function isLoggedIn(): bool {
    return isset($_SESSION['user_id']);
}

function isAdmin(): bool {
    return isLoggedIn() && ($_SESSION['role'] ?? '') === 'admin';
}

function requireLogin(): void {
    if (!isLoggedIn()) redirect('login', ['error' => 'Please log in to continue.']);
}

function requireAdmin(): void {
    requireLogin();
    if (!isAdmin()) redirect('home', ['error' => 'Access denied.']);
}

function flash(string $key): string {
    $val = $_SESSION['flash'][$key] ?? ($_GET[$key] ?? '');
    unset($_SESSION['flash'][$key]);
    return htmlspecialchars($val);
}

// ============================================================
//  ACTIONS (POST handlers)
// ============================================================
$action = $_POST['action'] ?? '';

if ($action === 'signup') {
    $username  = trim($_POST['username'] ?? '');
    $password  = $_POST['password'] ?? '';
    $confirm   = $_POST['confirm']  ?? '';
    $job_title = trim($_POST['job_title'] ?? '');

    if (!$username || !$password || !$job_title) {
        redirect('signup', ['error' => 'All fields are required.']);
    }
    if (strlen($username) < 3 || strlen($username) > 50) {
        redirect('signup', ['error' => 'Username must be 3–50 characters.']);
    }
    if (strlen($password) < 6) {
        redirect('signup', ['error' => 'Password must be at least 6 characters.']);
    }
    if ($password !== $confirm) {
        redirect('signup', ['error' => 'Passwords do not match.']);
    }

    $db   = db();
    $stmt = $db->prepare('SELECT id FROM users WHERE username = ?');
    $stmt->bind_param('s', $username);
    $stmt->execute();
    if ($stmt->get_result()->num_rows > 0) {
        redirect('signup', ['error' => 'Username already taken.']);
    }

    $hash = password_hash($password, PASSWORD_DEFAULT);
    $stmt = $db->prepare('INSERT INTO users (username, password, job_title) VALUES (?, ?, ?)');
    $stmt->bind_param('sss', $username, $hash, $job_title);
    $stmt->execute();

    redirect('login', ['success' => 'Account created! Please log in.']);
}

if ($action === 'login') {
    $username = trim($_POST['username'] ?? '');
    $password = $_POST['password'] ?? '';

    if (!$username || !$password) {
        redirect('login', ['error' => 'Username and password are required.']);
    }

    $db   = db();
    $stmt = $db->prepare('SELECT id, password, role, job_title FROM users WHERE username = ?');
    $stmt->bind_param('s', $username);
    $stmt->execute();
    $user = $stmt->get_result()->fetch_assoc();

    if (!$user || !password_verify($password, $user['password'])) {
        redirect('login', ['error' => 'Invalid username or password.']);
    }

    $_SESSION['user_id']   = $user['id'];
    $_SESSION['username']  = $username;
    $_SESSION['role']      = $user['role'];
    $_SESSION['job_title'] = $user['job_title'];

    redirect('home');
}

if ($action === 'logout') {
    session_destroy();
    redirect('login', ['success' => 'You have been logged out.']);
}

if ($action === 'add_announcement' && isAdmin()) {
    $title   = trim($_POST['ann_title']   ?? '');
    $content = trim($_POST['ann_content'] ?? '');
    if ($title && $content) {
        $stmt = db()->prepare('INSERT INTO announcements (title, content) VALUES (?, ?)');
        $stmt->bind_param('ss', $title, $content);
        $stmt->execute();
    }
    redirect('admin');
}

if ($action === 'delete_user' && isAdmin()) {
    $uid = (int)($_POST['uid'] ?? 0);
    if ($uid && $uid !== (int)$_SESSION['user_id']) {
        $stmt = db()->prepare('DELETE FROM users WHERE id = ?');
        $stmt->bind_param('i', $uid);
        $stmt->execute();
    }
    redirect('admin');
}

if ($action === 'delete_announcement' && isAdmin()) {
    $aid = (int)($_POST['aid'] ?? 0);
    if ($aid) {
        $stmt = db()->prepare('DELETE FROM announcements WHERE id = ?');
        $stmt->bind_param('i', $aid);
        $stmt->execute();
    }
    redirect('admin');
}

// ============================================================
//  PAGE ROUTING
// ============================================================
$page = $_GET['page'] ?? (isLoggedIn() ? 'home' : 'login');

// Guard pages
if (in_array($page, ['home', 'employee']) && !isLoggedIn()) {
    redirect('login', ['error' => 'Please log in to continue.']);
}
if ($page === 'admin' && !isAdmin()) {
    if (!isLoggedIn()) redirect('login', ['error' => 'Please log in to continue.']);
    else redirect('home', ['error' => 'Access denied.']);
}
if (in_array($page, ['login', 'signup']) && isLoggedIn()) {
    redirect('home');
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
<title>
    <?= match($page) {
        'login'    => 'Sign In',
        'signup'   => 'Create Account',
        'home'     => 'Dashboard',
        'admin'    => 'Admin Panel',
        'employee' => 'Announcements',
        default    => 'Portal'
    } ?> — NexusPortal
</title>
<link rel="preconnect" href="https://fonts.googleapis.com">
<link href="https://fonts.googleapis.com/css2?family=Syne:wght@400;600;700;800&family=DM+Sans:wght@300;400;500&display=swap" rel="stylesheet">
<style>
/* ── RESET & BASE ── */
*, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }

:root {
    --bg:        #0a0a0f;
    --surface:   #12121a;
    --surface2:  #1a1a26;
    --border:    #2a2a3e;
    --accent:    #6c63ff;
    --accent2:   #ff6584;
    --accent3:   #43e6c5;
    --text:      #e8e8f0;
    --muted:     #7a7a9a;
    --success:   #43e6c5;
    --error:     #ff6584;
    --radius:    14px;
    --radius-sm: 8px;
}

html, body {
    min-height: 100vh;
    background: var(--bg);
    color: var(--text);
    font-family: 'DM Sans', sans-serif;
    font-weight: 400;
    line-height: 1.6;
    overflow-x: hidden;
}

/* ── NOISE TEXTURE OVERLAY ── */
body::before {
    content: '';
    position: fixed;
    inset: 0;
    background-image: url("data:image/svg+xml,%3Csvg viewBox='0 0 256 256' xmlns='http://www.w3.org/2000/svg'%3E%3Cfilter id='n'%3E%3CfeTurbulence type='fractalNoise' baseFrequency='0.9' numOctaves='4' stitchTiles='stitch'/%3E%3C/filter%3E%3Crect width='100%25' height='100%25' filter='url(%23n)' opacity='0.04'/%3E%3C/svg%3E");
    pointer-events: none;
    z-index: 0;
}

/* ── GLOW BLOBS ── */
.blob {
    position: fixed;
    border-radius: 50%;
    filter: blur(100px);
    opacity: 0.12;
    pointer-events: none;
    z-index: 0;
}
.blob-1 { width: 600px; height: 600px; background: var(--accent);  top: -200px; left: -200px; animation: drift1 20s ease-in-out infinite; }
.blob-2 { width: 500px; height: 500px; background: var(--accent2); bottom: -150px; right: -150px; animation: drift2 25s ease-in-out infinite; }
.blob-3 { width: 400px; height: 400px; background: var(--accent3); top: 40%; left: 50%; animation: drift3 18s ease-in-out infinite; }

@keyframes drift1 { 0%,100%{transform:translate(0,0)} 50%{transform:translate(60px,40px)} }
@keyframes drift2 { 0%,100%{transform:translate(0,0)} 50%{transform:translate(-50px,-30px)} }
@keyframes drift3 { 0%,100%{transform:translate(-50%,-50%)} 50%{transform:translate(-45%,-55%)} }

/* ── LAYOUT ── */
.app-wrapper {
    position: relative;
    z-index: 1;
    min-height: 100vh;
    display: flex;
    flex-direction: column;
}

/* ── NAVBAR ── */
nav {
    display: flex;
    align-items: center;
    justify-content: space-between;
    padding: 1.1rem 2.5rem;
    background: rgba(10,10,15,0.7);
    backdrop-filter: blur(20px);
    border-bottom: 1px solid var(--border);
    position: sticky;
    top: 0;
    z-index: 100;
}

.nav-brand {
    font-family: 'Syne', sans-serif;
    font-weight: 800;
    font-size: 1.3rem;
    letter-spacing: -0.02em;
    background: linear-gradient(135deg, var(--accent), var(--accent3));
    -webkit-background-clip: text;
    -webkit-text-fill-color: transparent;
    background-clip: text;
    text-decoration: none;
}

.nav-links {
    display: flex;
    align-items: center;
    gap: 0.4rem;
}

.nav-links a, .nav-links button {
    font-family: 'DM Sans', sans-serif;
    font-size: 0.875rem;
    font-weight: 500;
    color: var(--muted);
    text-decoration: none;
    padding: 0.45rem 1rem;
    border-radius: var(--radius-sm);
    transition: all 0.2s;
    background: none;
    border: none;
    cursor: pointer;
}

.nav-links a:hover, .nav-links button:hover { color: var(--text); background: var(--surface2); }
.nav-links a.active { color: var(--accent); background: rgba(108,99,255,0.12); }
.nav-links .nav-logout { color: var(--error); }
.nav-links .nav-logout:hover { background: rgba(255,101,132,0.1); }

.nav-user {
    font-size: 0.8rem;
    color: var(--muted);
    padding: 0.4rem 1rem;
    background: var(--surface2);
    border-radius: 20px;
    border: 1px solid var(--border);
    margin-right: 0.5rem;
}

/* ── MAIN CONTENT ── */
main {
    flex: 1;
    padding: 3rem 2.5rem;
    max-width: 1100px;
    margin: 0 auto;
    width: 100%;
}

/* ── PAGE HEADER ── */
.page-header {
    margin-bottom: 2.5rem;
    animation: fadeUp 0.5s ease both;
}

.page-header h1 {
    font-family: 'Syne', sans-serif;
    font-weight: 800;
    font-size: 2.2rem;
    letter-spacing: -0.04em;
    line-height: 1.1;
    margin-bottom: 0.4rem;
}

.page-header p {
    color: var(--muted);
    font-size: 0.95rem;
}

/* ── AUTH PAGES ── */
.auth-wrap {
    min-height: 100vh;
    display: flex;
    align-items: center;
    justify-content: center;
    padding: 2rem;
    position: relative;
    z-index: 1;
}

.auth-card {
    width: 100%;
    max-width: 420px;
    background: rgba(18,18,26,0.8);
    backdrop-filter: blur(30px);
    border: 1px solid var(--border);
    border-radius: 20px;
    padding: 2.5rem;
    animation: fadeUp 0.5s ease both;
}

.auth-logo {
    font-family: 'Syne', sans-serif;
    font-weight: 800;
    font-size: 1.6rem;
    letter-spacing: -0.03em;
    background: linear-gradient(135deg, var(--accent), var(--accent3));
    -webkit-background-clip: text;
    -webkit-text-fill-color: transparent;
    background-clip: text;
    margin-bottom: 0.5rem;
}

.auth-card h2 {
    font-family: 'Syne', sans-serif;
    font-weight: 700;
    font-size: 1.5rem;
    margin-bottom: 0.3rem;
}

.auth-card .sub {
    color: var(--muted);
    font-size: 0.875rem;
    margin-bottom: 2rem;
}

/* ── FORM ELEMENTS ── */
.form-group {
    margin-bottom: 1.1rem;
}

label {
    display: block;
    font-size: 0.8rem;
    font-weight: 500;
    color: var(--muted);
    text-transform: uppercase;
    letter-spacing: 0.08em;
    margin-bottom: 0.45rem;
}

input[type="text"],
input[type="password"],
textarea,
select {
    width: 100%;
    background: var(--surface2);
    border: 1px solid var(--border);
    border-radius: var(--radius-sm);
    color: var(--text);
    padding: 0.75rem 1rem;
    font-family: 'DM Sans', sans-serif;
    font-size: 0.9rem;
    transition: border-color 0.2s, box-shadow 0.2s;
    outline: none;
}

input:focus, textarea:focus {
    border-color: var(--accent);
    box-shadow: 0 0 0 3px rgba(108,99,255,0.15);
}

textarea { resize: vertical; min-height: 100px; }

/* ── BUTTONS ── */
.btn {
    display: inline-flex;
    align-items: center;
    justify-content: center;
    gap: 0.4rem;
    padding: 0.75rem 1.6rem;
    border-radius: var(--radius-sm);
    font-family: 'DM Sans', sans-serif;
    font-size: 0.9rem;
    font-weight: 500;
    border: none;
    cursor: pointer;
    transition: all 0.2s;
    text-decoration: none;
}

.btn-primary {
    background: var(--accent);
    color: #fff;
    width: 100%;
}
.btn-primary:hover { background: #5a52e0; transform: translateY(-1px); box-shadow: 0 8px 24px rgba(108,99,255,0.35); }

.btn-sm { padding: 0.45rem 1rem; font-size: 0.8rem; width: auto; }

.btn-danger { background: rgba(255,101,132,0.15); color: var(--error); border: 1px solid rgba(255,101,132,0.3); }
.btn-danger:hover { background: rgba(255,101,132,0.25); }

.btn-ghost { background: var(--surface2); color: var(--text); border: 1px solid var(--border); }
.btn-ghost:hover { background: var(--border); }

.btn-success { background: rgba(67,230,197,0.15); color: var(--success); border: 1px solid rgba(67,230,197,0.3); }
.btn-success:hover { background: rgba(67,230,197,0.25); }

/* ── ALERTS ── */
.alert {
    padding: 0.8rem 1rem;
    border-radius: var(--radius-sm);
    font-size: 0.875rem;
    margin-bottom: 1.2rem;
    display: flex;
    align-items: center;
    gap: 0.6rem;
}
.alert-error   { background: rgba(255,101,132,0.1); border: 1px solid rgba(255,101,132,0.3); color: #ff8fa5; }
.alert-success { background: rgba(67,230,197,0.1);  border: 1px solid rgba(67,230,197,0.3);  color: var(--success); }

/* ── AUTH FOOTER ── */
.auth-footer {
    margin-top: 1.5rem;
    text-align: center;
    font-size: 0.85rem;
    color: var(--muted);
}
.auth-footer a { color: var(--accent); text-decoration: none; }
.auth-footer a:hover { text-decoration: underline; }

/* ── CARDS ── */
.card {
    background: rgba(18,18,26,0.7);
    border: 1px solid var(--border);
    border-radius: var(--radius);
    padding: 1.5rem;
    backdrop-filter: blur(20px);
}

.card-grid {
    display: grid;
    grid-template-columns: repeat(auto-fill, minmax(280px, 1fr));
    gap: 1.2rem;
}

/* ── HOME STATS ── */
.stat-card {
    background: rgba(18,18,26,0.7);
    border: 1px solid var(--border);
    border-radius: var(--radius);
    padding: 1.5rem;
    animation: fadeUp 0.5s ease both;
}
.stat-card:nth-child(2) { animation-delay: 0.1s; }
.stat-card:nth-child(3) { animation-delay: 0.2s; }

.stat-icon {
    width: 44px; height: 44px;
    border-radius: 10px;
    display: flex; align-items: center; justify-content: center;
    font-size: 1.3rem;
    margin-bottom: 1rem;
}

.stat-card h3 {
    font-family: 'Syne', sans-serif;
    font-size: 1.8rem;
    font-weight: 800;
    margin-bottom: 0.2rem;
}

.stat-card p { color: var(--muted); font-size: 0.85rem; }

/* ── TABLE ── */
.table-wrap {
    background: rgba(18,18,26,0.7);
    border: 1px solid var(--border);
    border-radius: var(--radius);
    overflow: hidden;
    backdrop-filter: blur(20px);
    animation: fadeUp 0.4s ease both;
}

table {
    width: 100%;
    border-collapse: collapse;
}

thead th {
    background: var(--surface2);
    padding: 0.9rem 1.2rem;
    text-align: left;
    font-size: 0.75rem;
    font-weight: 600;
    text-transform: uppercase;
    letter-spacing: 0.08em;
    color: var(--muted);
    border-bottom: 1px solid var(--border);
}

tbody td {
    padding: 0.9rem 1.2rem;
    font-size: 0.875rem;
    border-bottom: 1px solid rgba(42,42,62,0.5);
    vertical-align: middle;
}

tbody tr:last-child td { border-bottom: none; }
tbody tr:hover td { background: rgba(255,255,255,0.02); }

/* ── BADGES ── */
.badge {
    display: inline-block;
    padding: 0.25rem 0.65rem;
    border-radius: 20px;
    font-size: 0.75rem;
    font-weight: 600;
    letter-spacing: 0.04em;
}
.badge-admin    { background: rgba(108,99,255,0.15); color: #a09aff; border: 1px solid rgba(108,99,255,0.3); }
.badge-employee { background: rgba(67,230,197,0.1);  color: var(--accent3); border: 1px solid rgba(67,230,197,0.25); }

/* ── ANNOUNCEMENTS ── */
.announcement-card {
    background: rgba(18,18,26,0.7);
    border: 1px solid var(--border);
    border-radius: var(--radius);
    padding: 1.5rem 1.8rem;
    backdrop-filter: blur(20px);
    position: relative;
    overflow: hidden;
    transition: border-color 0.2s, transform 0.2s;
    animation: fadeUp 0.5s ease both;
}
.announcement-card:nth-child(2) { animation-delay: 0.1s; }
.announcement-card:nth-child(3) { animation-delay: 0.2s; }
.announcement-card:nth-child(4) { animation-delay: 0.3s; }

.announcement-card::before {
    content: '';
    position: absolute;
    left: 0; top: 0; bottom: 0;
    width: 3px;
    background: linear-gradient(to bottom, var(--accent), var(--accent3));
    border-radius: 2px;
}

.announcement-card:hover { border-color: rgba(108,99,255,0.4); transform: translateY(-2px); }

.announcement-card h3 {
    font-family: 'Syne', sans-serif;
    font-weight: 700;
    font-size: 1.05rem;
    margin-bottom: 0.5rem;
}

.announcement-card p { color: var(--muted); font-size: 0.9rem; line-height: 1.65; }

.announcement-meta {
    display: flex;
    align-items: center;
    justify-content: space-between;
    margin-top: 1rem;
}

.announcement-date { font-size: 0.78rem; color: var(--muted); }

/* ── ADMIN PANEL GRID ── */
.admin-grid {
    display: grid;
    grid-template-columns: 1fr 1fr;
    gap: 2rem;
}

@media (max-width: 768px) {
    .admin-grid { grid-template-columns: 1fr; }
    nav { padding: 1rem 1.2rem; }
    main { padding: 2rem 1.2rem; }
}

/* ── SECTION TITLE ── */
.section-title {
    font-family: 'Syne', sans-serif;
    font-weight: 700;
    font-size: 1.1rem;
    margin-bottom: 1.2rem;
    display: flex;
    align-items: center;
    gap: 0.6rem;
}

/* ── HOME QUICK LINKS ── */
.quicklink-card {
    background: rgba(18,18,26,0.7);
    border: 1px solid var(--border);
    border-radius: var(--radius);
    padding: 1.5rem;
    text-decoration: none;
    color: var(--text);
    transition: all 0.2s;
    display: flex;
    align-items: center;
    gap: 1rem;
    animation: fadeUp 0.5s ease both;
}
.quicklink-card:nth-child(2) { animation-delay: 0.1s; }
.quicklink-card:nth-child(3) { animation-delay: 0.2s; }
.quicklink-card:hover { border-color: var(--accent); transform: translateY(-3px); box-shadow: 0 12px 30px rgba(0,0,0,0.3); }

.quicklink-icon {
    width: 48px; height: 48px;
    border-radius: 12px;
    display: flex; align-items: center; justify-content: center;
    font-size: 1.4rem;
    flex-shrink: 0;
}

.quicklink-card h3 { font-family: 'Syne', sans-serif; font-size: 1rem; font-weight: 700; margin-bottom: 0.2rem; }
.quicklink-card p  { font-size: 0.82rem; color: var(--muted); }

/* ── DIVIDER ── */
.divider { border: none; border-top: 1px solid var(--border); margin: 2rem 0; }

/* ── WELCOME BANNER ── */
.welcome-banner {
    background: linear-gradient(135deg, rgba(108,99,255,0.15), rgba(67,230,197,0.08));
    border: 1px solid rgba(108,99,255,0.25);
    border-radius: var(--radius);
    padding: 1.8rem 2rem;
    margin-bottom: 2rem;
    display: flex;
    align-items: center;
    justify-content: space-between;
    gap: 1rem;
    animation: fadeUp 0.4s ease both;
}

.welcome-banner h2 { font-family: 'Syne', sans-serif; font-weight: 800; font-size: 1.4rem; margin-bottom: 0.3rem; }
.welcome-banner p  { color: var(--muted); font-size: 0.9rem; }

.welcome-avatar {
    width: 56px; height: 56px;
    border-radius: 50%;
    background: linear-gradient(135deg, var(--accent), var(--accent2));
    display: flex; align-items: center; justify-content: center;
    font-family: 'Syne', sans-serif;
    font-weight: 800;
    font-size: 1.3rem;
    flex-shrink: 0;
}

/* ── ANIMATIONS ── */
@keyframes fadeUp {
    from { opacity: 0; transform: translateY(16px); }
    to   { opacity: 1; transform: translateY(0); }
}

/* ── EMPTY STATE ── */
.empty-state {
    text-align: center;
    padding: 3rem 1rem;
    color: var(--muted);
}
.empty-state .icon { font-size: 2.5rem; margin-bottom: 0.8rem; }
.empty-state p { font-size: 0.9rem; }
</style>
</head>
<body>
<div class="blob blob-1"></div>
<div class="blob blob-2"></div>
<div class="blob blob-3"></div>

<?php if (!in_array($page, ['login', 'signup'])): ?>
<div class="app-wrapper">
<!-- ============ NAVBAR ============ -->
<nav>
    <a href="index.php?page=home" class="nav-brand">✦ NexusPortal</a>
    <div class="nav-links">
        <?php if (isLoggedIn()): ?>
            <span class="nav-user">
                <?= htmlspecialchars($_SESSION['username']) ?> · <?= htmlspecialchars($_SESSION['job_title']) ?>
            </span>
            <a href="index.php?page=home"     class="<?= $page === 'home'     ? 'active' : '' ?>">Dashboard</a>
            <a href="index.php?page=employee" class="<?= $page === 'employee' ? 'active' : '' ?>">Announcements</a>
            <?php if (isAdmin()): ?>
                <a href="index.php?page=admin" class="<?= $page === 'admin' ? 'active' : '' ?>">Admin</a>
            <?php endif; ?>
            <form method="POST" style="display:inline">
                <input type="hidden" name="action" value="logout">
                <button type="submit" class="nav-logout">Log Out</button>
            </form>
        <?php endif; ?>
    </div>
</nav>
<main>
<?php endif; ?>

<?php
// ============================================================
//  PAGES
// ============================================================

// ─── LOGIN ───────────────────────────────────────────────────
if ($page === 'login'):
    $error   = flash('error');
    $success = flash('success');
?>
<div class="auth-wrap">
  <div class="auth-card">
    <div class="auth-logo">✦ NexusPortal</div>
    <h2>Welcome back</h2>
    <p class="sub">Sign in to access your workspace.</p>

    <?php if ($error):   ?><div class="alert alert-error">⚠ <?= $error ?></div><?php endif; ?>
    <?php if ($success): ?><div class="alert alert-success">✓ <?= $success ?></div><?php endif; ?>

    <form method="POST">
      <input type="hidden" name="action" value="login">
      <div class="form-group">
        <label>Username</label>
        <input type="text" name="username" autocomplete="username" required>
      </div>
      <div class="form-group">
        <label>Password</label>
        <input type="password" name="password" autocomplete="current-password" required>
      </div>
      <button type="submit" class="btn btn-primary">Sign In →</button>
    </form>

    <div class="auth-footer">
      Don't have an account? <a href="index.php?page=signup">Create one</a>
    </div>
  </div>
</div>

<?php
// ─── SIGNUP ──────────────────────────────────────────────────
elseif ($page === 'signup'):
    $error = flash('error');
?>
<div class="auth-wrap">
  <div class="auth-card">
    <div class="auth-logo">✦ NexusPortal</div>
    <h2>Create account</h2>
    <p class="sub">Join your team's workspace today.</p>

    <?php if ($error): ?><div class="alert alert-error">⚠ <?= $error ?></div><?php endif; ?>

    <form method="POST">
      <input type="hidden" name="action" value="signup">
      <div class="form-group">
        <label>Username</label>
        <input type="text" name="username" autocomplete="username" required minlength="3" maxlength="50">
      </div>
      <div class="form-group">
        <label>Job Title</label>
        <input type="text" name="job_title" placeholder="e.g. Software Engineer" required>
      </div>
      <div class="form-group">
        <label>Password</label>
        <input type="password" name="password" autocomplete="new-password" required minlength="6">
      </div>
      <div class="form-group">
        <label>Confirm Password</label>
        <input type="password" name="confirm" autocomplete="new-password" required>
      </div>
      <button type="submit" class="btn btn-primary">Create Account →</button>
    </form>

    <div class="auth-footer">
      Already have an account? <a href="index.php?page=login">Sign in</a>
    </div>
  </div>
</div>

<?php
// ─── HOME / DASHBOARD ─────────────────────────────────────────
elseif ($page === 'home'):
    requireLogin();
    $db       = db();
    $userCount = $db->query('SELECT COUNT(*) AS c FROM users')->fetch_assoc()['c'];
    $annCount  = $db->query('SELECT COUNT(*) AS c FROM announcements')->fetch_assoc()['c'];
    $initials  = strtoupper(substr($_SESSION['username'], 0, 2));
    $error     = flash('error');
?>
<?php if ($error): ?><div class="alert alert-error">⚠ <?= $error ?></div><?php endif; ?>

<div class="welcome-banner">
  <div>
    <h2>Good <?= date('H') < 12 ? 'morning' : (date('H') < 17 ? 'afternoon' : 'evening') ?>,
        <?= htmlspecialchars($_SESSION['username']) ?>!</h2>
    <p><?= htmlspecialchars($_SESSION['job_title']) ?> · <?= date('l, F j, Y') ?></p>
  </div>
  <div class="welcome-avatar"><?= $initials ?></div>
</div>

<div class="card-grid" style="margin-bottom:2.5rem">
  <div class="stat-card">
    <div class="stat-icon" style="background:rgba(108,99,255,0.15)">👥</div>
    <h3><?= $userCount ?></h3>
    <p>Registered Users</p>
  </div>
  <div class="stat-card">
    <div class="stat-icon" style="background:rgba(67,230,197,0.12)">📢</div>
    <h3><?= $annCount ?></h3>
    <p>Active Announcements</p>
  </div>
  <div class="stat-card">
    <div class="stat-icon" style="background:rgba(255,101,132,0.12)">🗓</div>
    <h3><?= date('H:i') ?></h3>
    <p>Current Time</p>
  </div>
</div>

<p class="section-title">Quick Navigation</p>
<div class="card-grid">
  <a href="index.php?page=employee" class="quicklink-card">
    <div class="quicklink-icon" style="background:rgba(67,230,197,0.12)">📢</div>
    <div>
      <h3>Announcements</h3>
      <p>Read the latest company news</p>
    </div>
  </a>
  <?php if (isAdmin()): ?>
  <a href="index.php?page=admin" class="quicklink-card">
    <div class="quicklink-icon" style="background:rgba(108,99,255,0.15)">⚙️</div>
    <div>
      <h3>Admin Panel</h3>
      <p>Manage users and announcements</p>
    </div>
  </a>
  <?php endif; ?>
</div>

<?php
// ─── EMPLOYEE / ANNOUNCEMENTS ─────────────────────────────────
elseif ($page === 'employee'):
    requireLogin();
    $announcements = db()->query('SELECT * FROM announcements ORDER BY created_at DESC')->fetch_all(MYSQLI_ASSOC);
?>
<div class="page-header">
  <h1>📢 Announcements</h1>
  <p>Stay up to date with the latest from your team.</p>
</div>

<?php if (empty($announcements)): ?>
  <div class="empty-state">
    <div class="icon">📭</div>
    <p>No announcements yet. Check back soon.</p>
  </div>
<?php else: ?>
  <div style="display:flex; flex-direction:column; gap:1rem;">
    <?php foreach ($announcements as $ann): ?>
      <div class="announcement-card">
        <h3><?= htmlspecialchars($ann['title']) ?></h3>
        <p><?= nl2br(htmlspecialchars($ann['content'])) ?></p>
        <div class="announcement-meta">
          <span class="announcement-date">🕐 <?= date('M j, Y · g:i A', strtotime($ann['created_at'])) ?></span>
        </div>
      </div>
    <?php endforeach; ?>
  </div>
<?php endif; ?>

<?php
// ─── ADMIN ────────────────────────────────────────────────────
elseif ($page === 'admin'):
    requireAdmin();
    $db    = db();
    $users = $db->query('SELECT id, username, job_title, role, created_at FROM users ORDER BY created_at DESC')->fetch_all(MYSQLI_ASSOC);
    $anns  = $db->query('SELECT * FROM announcements ORDER BY created_at DESC')->fetch_all(MYSQLI_ASSOC);
?>
<div class="page-header">
  <h1>⚙️ Admin Panel</h1>
  <p>Manage users and company announcements.</p>
</div>

<div class="admin-grid">

  <!-- LEFT: User List -->
  <div>
    <p class="section-title">👥 Registered Users <span style="color:var(--muted);font-weight:400;font-size:0.85rem">(<?= count($users) ?>)</span></p>
    <div class="table-wrap">
      <table>
        <thead>
          <tr>
            <th>Username</th>
            <th>Job Title</th>
            <th>Role</th>
            <th>Action</th>
          </tr>
        </thead>
        <tbody>
          <?php foreach ($users as $u): ?>
            <tr>
              <td style="font-weight:500"><?= htmlspecialchars($u['username']) ?></td>
              <td style="color:var(--muted)"><?= htmlspecialchars($u['job_title']) ?></td>
              <td><span class="badge badge-<?= $u['role'] ?>"><?= ucfirst($u['role']) ?></span></td>
              <td>
                <?php if ($u['id'] !== (int)$_SESSION['user_id']): ?>
                  <form method="POST" onsubmit="return confirm('Delete user <?= htmlspecialchars($u['username'], ENT_QUOTES) ?>?')">
                    <input type="hidden" name="action" value="delete_user">
                    <input type="hidden" name="uid"    value="<?= $u['id'] ?>">
                    <button type="submit" class="btn btn-sm btn-danger">Delete</button>
                  </form>
                <?php else: ?>
                  <span style="font-size:0.75rem;color:var(--muted)">You</span>
                <?php endif; ?>
              </td>
            </tr>
          <?php endforeach; ?>
        </tbody>
      </table>
    </div>
  </div>

  <!-- RIGHT: Announcements Management -->
  <div>
    <p class="section-title">📢 Manage Announcements</p>

    <!-- Add Announcement Form -->
    <div class="card" style="margin-bottom:1.5rem">
      <p style="font-size:0.85rem;font-weight:600;margin-bottom:1rem;color:var(--muted);text-transform:uppercase;letter-spacing:0.06em">New Announcement</p>
      <form method="POST">
        <input type="hidden" name="action" value="add_announcement">
        <div class="form-group">
          <label>Title</label>
          <input type="text" name="ann_title" placeholder="Announcement title" required>
        </div>
        <div class="form-group">
          <label>Content</label>
          <textarea name="ann_content" placeholder="Write the announcement..." required></textarea>
        </div>
        <button type="submit" class="btn btn-success btn-sm" style="width:100%">+ Post Announcement</button>
      </form>
    </div>

    <!-- Existing Announcements -->
    <?php if (empty($anns)): ?>
      <div class="empty-state"><p>No announcements posted yet.</p></div>
    <?php else: ?>
      <div style="display:flex;flex-direction:column;gap:0.8rem;max-height:500px;overflow-y:auto;padding-right:4px">
        <?php foreach ($anns as $ann): ?>
          <div class="card" style="padding:1.1rem">
            <div style="display:flex;justify-content:space-between;align-items:flex-start;gap:0.8rem">
              <div>
                <p style="font-weight:600;font-size:0.9rem;margin-bottom:0.3rem"><?= htmlspecialchars($ann['title']) ?></p>
                <p style="font-size:0.8rem;color:var(--muted)"><?= date('M j, Y', strtotime($ann['created_at'])) ?></p>
              </div>
              <form method="POST" onsubmit="return confirm('Delete this announcement?')">
                <input type="hidden" name="action" value="delete_announcement">
                <input type="hidden" name="aid"    value="<?= $ann['id'] ?>">
                <button type="submit" class="btn btn-sm btn-danger">✕</button>
              </form>
            </div>
          </div>
        <?php endforeach; ?>
      </div>
    <?php endif; ?>
  </div>
</div>

<?php endif; ?>

<?php if (!in_array($page, ['login', 'signup'])): ?>
</main>
</div>
<?php endif; ?>
</body>
</html>