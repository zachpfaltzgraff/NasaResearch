<?php
session_start();

// ─── In-memory "database" ────────────────────────────────────────────────────
// In a real app, use a proper database (MySQL, PostgreSQL, etc.)
if (!isset($_SESSION['users_db'])) {
    $_SESSION['users_db'] = [
        1 => [
            'id'       => 1,
            'name'     => 'Alice Admin',
            'email'    => 'alice@example.com',
            'password' => password_hash('admin123', PASSWORD_DEFAULT),
            'role'     => 'admin',
            'joined'   => '2024-01-15',
            'avatar'   => 'AA',
        ],
        2 => [
            'id'       => 2,
            'name'     => 'Bob Builder',
            'email'    => 'bob@example.com',
            'password' => password_hash('user123', PASSWORD_DEFAULT),
            'role'     => 'user',
            'joined'   => '2024-03-22',
            'avatar'   => 'BB',
        ],
        3 => [
            'id'       => 3,
            'name'     => 'Carol Chen',
            'email'    => 'carol@example.com',
            'password' => password_hash('user123', PASSWORD_DEFAULT),
            'role'     => 'user',
            'joined'   => '2024-06-10',
            'avatar'   => 'CC',
        ],
        4 => [
            'id'       => 4,
            'name'     => 'David Drake',
            'email'    => 'david@example.com',
            'password' => password_hash('user123', PASSWORD_DEFAULT),
            'role'     => 'user',
            'joined'   => '2025-01-05',
            'avatar'   => 'DD',
        ],
    ];
}

$users_db = &$_SESSION['users_db'];
$errors   = [];
$success  = '';

// ─── Actions ─────────────────────────────────────────────────────────────────

// Login
if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['action']) && $_POST['action'] === 'login') {
    $email    = trim($_POST['email'] ?? '');
    $password = $_POST['password'] ?? '';

    $found = null;
    foreach ($users_db as $u) {
        if ($u['email'] === $email) { $found = $u; break; }
    }

    if (!$found || !password_verify($password, $found['password'])) {
        $errors[] = 'Invalid email or password.';
    } else {
        $_SESSION['user_id'] = $found['id'];
    }
}

// Logout
if (isset($_GET['action']) && $_GET['action'] === 'logout') {
    session_destroy();
    header('Location: ' . $_SERVER['PHP_SELF']);
    exit;
}

// Admin: delete user
if (isset($_GET['action']) && $_GET['action'] === 'delete' && isset($_SESSION['user_id'])) {
    $current = $users_db[$_SESSION['user_id']] ?? null;
    if ($current && $current['role'] === 'admin') {
        $del_id = (int)($_GET['id'] ?? 0);
        if ($del_id && $del_id !== $_SESSION['user_id'] && isset($users_db[$del_id])) {
            unset($users_db[$del_id]);
            $success = 'User deleted successfully.';
        }
    }
}

// Admin: add user
if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['action']) && $_POST['action'] === 'add_user') {
    $current = isset($_SESSION['user_id']) ? ($users_db[$_SESSION['user_id']] ?? null) : null;
    if ($current && $current['role'] === 'admin') {
        $new_name  = trim($_POST['new_name'] ?? '');
        $new_email = trim($_POST['new_email'] ?? '');
        $new_pass  = $_POST['new_password'] ?? '';
        $new_role  = in_array($_POST['new_role'] ?? '', ['user','admin']) ? $_POST['new_role'] : 'user';

        if (!$new_name || !$new_email || !$new_pass) {
            $errors[] = 'All fields are required.';
        } else {
            foreach ($users_db as $u) {
                if ($u['email'] === $new_email) { $errors[] = 'Email already in use.'; break; }
            }
            if (!$errors) {
                $new_id = max(array_keys($users_db)) + 1;
                $words  = explode(' ', $new_name);
                $avatar = strtoupper(substr($words[0], 0, 1) . (isset($words[1]) ? substr($words[1], 0, 1) : substr($words[0], 1, 1)));
                $users_db[$new_id] = [
                    'id'       => $new_id,
                    'name'     => $new_name,
                    'email'    => $new_email,
                    'password' => password_hash($new_pass, PASSWORD_DEFAULT),
                    'role'     => $new_role,
                    'joined'   => date('Y-m-d'),
                    'avatar'   => $avatar,
                ];
                $success = "User "{$new_name}" added successfully.";
            }
        }
    }
}

// ─── Routing ──────────────────────────────────────────────────────────────────
$logged_in    = isset($_SESSION['user_id']) && isset($users_db[$_SESSION['user_id']]);
$current_user = $logged_in ? $users_db[$_SESSION['user_id']] : null;
$page         = isset($_GET['page']) ? $_GET['page'] : 'login';

if (!$logged_in) { $page = 'login'; }
elseif ($page === 'admin' && $current_user['role'] !== 'admin') { $page = 'dashboard'; }
elseif ($page === 'login') { $page = ($current_user['role'] === 'admin') ? 'admin' : 'dashboard'; }

// ─── HTML ─────────────────────────────────────────────────────────────────────
?>
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title><?= $page === 'admin' ? 'Admin Panel' : ($page === 'dashboard' ? 'Dashboard' : 'Sign In') ?> — NexusApp</title>
<link rel="preconnect" href="https://fonts.googleapis.com">
<link href="https://fonts.googleapis.com/css2?family=Syne:wght@400;500;600;700;800&family=DM+Sans:ital,opsz,wght@0,9..40,300;0,9..40,400;0,9..40,500;1,9..40,300&display=swap" rel="stylesheet">
<style>
/* ── Reset & tokens ─────────────────────────────────────── */
*, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }

:root {
  --bg:        #0a0b0e;
  --surface:   #111318;
  --surface2:  #181b22;
  --border:    #22262f;
  --border2:   #2e333e;
  --text:      #e8eaf0;
  --muted:     #6b7280;
  --accent:    #6ee7b7;       /* emerald */
  --accent2:   #34d399;
  --danger:    #f87171;
  --warn:      #fbbf24;
  --blue:      #60a5fa;
  --radius:    10px;
  --font-head: 'Syne', sans-serif;
  --font-body: 'DM Sans', sans-serif;
}

html, body {
  height: 100%;
  background: var(--bg);
  color: var(--text);
  font-family: var(--font-body);
  font-size: 15px;
  line-height: 1.6;
  -webkit-font-smoothing: antialiased;
}

/* ── Scrollbar ──────────────────────────────────────────── */
::-webkit-scrollbar { width: 6px; }
::-webkit-scrollbar-track { background: var(--bg); }
::-webkit-scrollbar-thumb { background: var(--border2); border-radius: 99px; }

/* ── Utility ────────────────────────────────────────────── */
.tag {
  display: inline-flex; align-items: center; gap: 5px;
  padding: 2px 10px; border-radius: 99px; font-size: 11px;
  font-weight: 600; letter-spacing: .04em; text-transform: uppercase;
  font-family: var(--font-head);
}
.tag-admin  { background: rgba(96,165,250,.15); color: var(--blue);   border: 1px solid rgba(96,165,250,.3); }
.tag-user   { background: rgba(110,231,183,.12); color: var(--accent); border: 1px solid rgba(110,231,183,.25); }
.tag-danger { background: rgba(248,113,113,.12); color: var(--danger); border: 1px solid rgba(248,113,113,.25); }

.btn {
  display: inline-flex; align-items: center; gap: 8px;
  padding: 10px 20px; border-radius: var(--radius);
  border: none; cursor: pointer; font-family: var(--font-head);
  font-size: 13px; font-weight: 600; letter-spacing: .04em;
  text-decoration: none; transition: all .18s ease;
}
.btn-primary { background: var(--accent); color: #0a0b0e; }
.btn-primary:hover { background: var(--accent2); transform: translateY(-1px); box-shadow: 0 4px 20px rgba(110,231,183,.3); }
.btn-ghost   { background: transparent; color: var(--muted); border: 1px solid var(--border2); }
.btn-ghost:hover { color: var(--text); border-color: var(--muted); }
.btn-danger  { background: rgba(248,113,113,.12); color: var(--danger); border: 1px solid rgba(248,113,113,.25); padding: 7px 14px; font-size: 12px; }
.btn-danger:hover { background: rgba(248,113,113,.22); }
.btn-sm { padding: 7px 14px; font-size: 12px; }

/* ── Alert ──────────────────────────────────────────────── */
.alert {
  padding: 12px 16px; border-radius: var(--radius); font-size: 13.5px;
  display: flex; align-items: center; gap: 10px; margin-bottom: 20px;
}
.alert-error   { background: rgba(248,113,113,.1); border: 1px solid rgba(248,113,113,.3); color: var(--danger); }
.alert-success { background: rgba(110,231,183,.1); border: 1px solid rgba(110,231,183,.3); color: var(--accent); }

/* ════════════════════════════════════════════════════════
   LOGIN PAGE
════════════════════════════════════════════════════════ */
.login-root {
  min-height: 100vh; display: flex; align-items: center; justify-content: center;
  padding: 24px;
  background:
    radial-gradient(ellipse 80% 60% at 10% 90%, rgba(110,231,183,.07) 0%, transparent 60%),
    radial-gradient(ellipse 60% 50% at 90% 10%, rgba(96,165,250,.07) 0%, transparent 60%),
    var(--bg);
}
.login-card {
  width: 100%; max-width: 420px;
  background: var(--surface);
  border: 1px solid var(--border);
  border-radius: 18px;
  padding: 44px 40px;
  animation: fadeUp .5s ease both;
}
.login-logo {
  font-family: var(--font-head); font-size: 26px; font-weight: 800;
  color: var(--accent); letter-spacing: -.02em; margin-bottom: 6px;
}
.login-sub { color: var(--muted); font-size: 14px; margin-bottom: 36px; }

.field-group { margin-bottom: 18px; }
.field-group label { display: block; font-size: 12px; font-weight: 600; letter-spacing: .05em; text-transform: uppercase; color: var(--muted); margin-bottom: 7px; font-family: var(--font-head); }
.field-group input {
  width: 100%; padding: 11px 14px; border-radius: var(--radius);
  background: var(--surface2); border: 1px solid var(--border);
  color: var(--text); font-family: var(--font-body); font-size: 14.5px;
  outline: none; transition: border-color .15s;
}
.field-group input:focus { border-color: var(--accent); box-shadow: 0 0 0 3px rgba(110,231,183,.1); }
.field-group input::placeholder { color: var(--muted); }

.login-card .btn-primary { width: 100%; justify-content: center; padding: 13px; font-size: 14px; margin-top: 8px; }

.demo-creds {
  margin-top: 28px; padding: 16px; border-radius: var(--radius);
  background: var(--surface2); border: 1px solid var(--border);
}
.demo-creds h4 { font-family: var(--font-head); font-size: 11px; letter-spacing: .08em; text-transform: uppercase; color: var(--muted); margin-bottom: 10px; }
.demo-row { display: flex; justify-content: space-between; font-size: 12.5px; margin-bottom: 4px; }
.demo-row:last-child { margin-bottom: 0; }
.demo-label { color: var(--muted); }
.demo-val { font-family: monospace; color: var(--text); }

/* ════════════════════════════════════════════════════════
   APP SHELL (sidebar + main)
════════════════════════════════════════════════════════ */
.app-root { display: flex; min-height: 100vh; }

/* Sidebar */
.sidebar {
  width: 240px; flex-shrink: 0;
  background: var(--surface);
  border-right: 1px solid var(--border);
  display: flex; flex-direction: column;
  padding: 0;
  position: fixed; top: 0; left: 0; bottom: 0;
}
.sidebar-logo {
  padding: 28px 24px 22px;
  font-family: var(--font-head); font-size: 20px; font-weight: 800;
  color: var(--accent); letter-spacing: -.02em;
  border-bottom: 1px solid var(--border);
}
.sidebar-logo span { color: var(--muted); font-weight: 400; }

.nav { padding: 16px 12px; flex: 1; }
.nav-item {
  display: flex; align-items: center; gap: 11px;
  padding: 10px 12px; border-radius: 8px;
  text-decoration: none; color: var(--muted);
  font-size: 14px; font-weight: 500;
  transition: all .15s; margin-bottom: 2px;
  font-family: var(--font-head); letter-spacing: .01em;
}
.nav-item:hover { color: var(--text); background: var(--surface2); }
.nav-item.active { color: var(--accent); background: rgba(110,231,183,.09); }
.nav-item svg { flex-shrink: 0; opacity: .7; }
.nav-item.active svg { opacity: 1; }

.sidebar-user {
  padding: 16px; border-top: 1px solid var(--border);
  display: flex; align-items: center; gap: 11px;
}
.avatar {
  width: 36px; height: 36px; border-radius: 50%; flex-shrink: 0;
  background: linear-gradient(135deg, var(--accent) 0%, #3b82f6 100%);
  display: flex; align-items: center; justify-content: center;
  font-family: var(--font-head); font-weight: 700; font-size: 12px; color: #0a0b0e;
}
.avatar.lg { width: 72px; height: 72px; font-size: 22px; border-radius: 18px; }
.sidebar-user-info { min-width: 0; flex: 1; }
.sidebar-user-name { font-size: 13px; font-weight: 600; white-space: nowrap; overflow: hidden; text-overflow: ellipsis; }
.sidebar-user-role { font-size: 11px; color: var(--muted); }

/* Main */
.main { margin-left: 240px; flex: 1; min-height: 100vh; padding: 36px 40px; }

.page-header { margin-bottom: 32px; }
.page-title { font-family: var(--font-head); font-size: 28px; font-weight: 800; letter-spacing: -.02em; }
.page-sub { color: var(--muted); font-size: 14px; margin-top: 4px; }

/* ── Dashboard ───────────────────────────────────────────── */
.profile-card {
  background: var(--surface); border: 1px solid var(--border);
  border-radius: 16px; padding: 32px;
  display: flex; gap: 28px; align-items: flex-start;
  margin-bottom: 28px; animation: fadeUp .4s ease both;
}
.profile-info { flex: 1; }
.profile-name { font-family: var(--font-head); font-size: 22px; font-weight: 700; margin-bottom: 4px; }
.profile-email { color: var(--muted); font-size: 14px; margin-bottom: 14px; }
.profile-meta { display: flex; gap: 20px; flex-wrap: wrap; }
.meta-item { font-size: 13px; color: var(--muted); }
.meta-item strong { color: var(--text); font-weight: 500; display: block; font-size: 14px; }

.stats-grid { display: grid; grid-template-columns: repeat(3, 1fr); gap: 16px; margin-bottom: 28px; }
.stat-card {
  background: var(--surface); border: 1px solid var(--border);
  border-radius: 12px; padding: 22px 24px;
  animation: fadeUp .4s ease both;
}
.stat-card:nth-child(2) { animation-delay: .06s; }
.stat-card:nth-child(3) { animation-delay: .12s; }
.stat-label { font-size: 11px; font-weight: 600; letter-spacing: .07em; text-transform: uppercase; color: var(--muted); font-family: var(--font-head); margin-bottom: 10px; }
.stat-value { font-family: var(--font-head); font-size: 30px; font-weight: 800; color: var(--accent); }
.stat-sub { font-size: 12px; color: var(--muted); margin-top: 2px; }

.activity-card {
  background: var(--surface); border: 1px solid var(--border);
  border-radius: 16px; padding: 28px;
  animation: fadeUp .4s .18s ease both;
}
.section-title { font-family: var(--font-head); font-size: 14px; font-weight: 700; letter-spacing: .04em; text-transform: uppercase; color: var(--muted); margin-bottom: 20px; }
.activity-list { list-style: none; }
.activity-item {
  display: flex; align-items: center; gap: 14px;
  padding: 12px 0; border-bottom: 1px solid var(--border);
}
.activity-item:last-child { border-bottom: none; padding-bottom: 0; }
.activity-dot { width: 8px; height: 8px; border-radius: 50%; background: var(--accent); flex-shrink: 0; }
.activity-dot.blue { background: var(--blue); }
.activity-dot.warn { background: var(--warn); }
.activity-text { font-size: 13.5px; flex: 1; }
.activity-time { font-size: 12px; color: var(--muted); }

/* ── Admin Panel ─────────────────────────────────────────── */
.admin-toolbar {
  display: flex; align-items: center; justify-content: space-between;
  margin-bottom: 24px; gap: 16px; flex-wrap: wrap;
}
.users-count {
  font-family: var(--font-head); font-size: 13px; color: var(--muted);
}
.users-count strong { color: var(--text); }

.table-wrap {
  background: var(--surface); border: 1px solid var(--border);
  border-radius: 16px; overflow: hidden; margin-bottom: 32px;
  animation: fadeUp .4s ease both;
}
table { width: 100%; border-collapse: collapse; }
thead { background: var(--surface2); }
th {
  text-align: left; padding: 14px 20px;
  font-family: var(--font-head); font-size: 11px; font-weight: 700;
  letter-spacing: .07em; text-transform: uppercase; color: var(--muted);
  border-bottom: 1px solid var(--border);
}
td { padding: 14px 20px; border-bottom: 1px solid var(--border); font-size: 14px; vertical-align: middle; }
tr:last-child td { border-bottom: none; }
tr:hover td { background: rgba(255,255,255,.02); }

.user-cell { display: flex; align-items: center; gap: 12px; }
.user-cell-info { }
.user-cell-name { font-weight: 500; font-size: 14px; }
.user-cell-id { font-size: 11px; color: var(--muted); font-family: monospace; }

.add-user-card {
  background: var(--surface); border: 1px solid var(--border);
  border-radius: 16px; padding: 28px;
  animation: fadeUp .4s .1s ease both;
}
.add-user-card h3 { font-family: var(--font-head); font-size: 16px; font-weight: 700; margin-bottom: 20px; }
.form-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 16px; }
.form-grid .span2 { grid-column: span 2; }
.form-grid .btn-primary { margin-top: 4px; }

/* ── Animations ──────────────────────────────────────────── */
@keyframes fadeUp {
  from { opacity: 0; transform: translateY(14px); }
  to   { opacity: 1; transform: translateY(0); }
}

/* ── Responsive ──────────────────────────────────────────── */
@media (max-width: 860px) {
  .sidebar { width: 200px; }
  .main { margin-left: 200px; padding: 24px; }
  .stats-grid { grid-template-columns: 1fr 1fr; }
  .form-grid { grid-template-columns: 1fr; }
  .form-grid .span2 { grid-column: span 1; }
}
@media (max-width: 640px) {
  .sidebar { display: none; }
  .main { margin-left: 0; padding: 20px 16px; }
  .stats-grid { grid-template-columns: 1fr; }
  .profile-card { flex-direction: column; }
}
</style>
</head>
<body>

<?php if ($page === 'login'): ?>
<!-- ══════════════════════════════════════════════════════
     LOGIN PAGE
══════════════════════════════════════════════════════ -->
<div class="login-root">
  <div class="login-card">
    <div class="login-logo">Nexus<span>App</span></div>
    <p class="login-sub">Sign in to your account to continue.</p>

    <?php if ($errors): ?>
      <div class="alert alert-error">
        <svg width="16" height="16" fill="none" viewBox="0 0 24 24" stroke="currentColor"><circle cx="12" cy="12" r="10" stroke-width="2"/><path d="M12 8v4m0 4h.01" stroke-width="2" stroke-linecap="round"/></svg>
        <?= htmlspecialchars($errors[0]) ?>
      </div>
    <?php endif; ?>

    <form method="POST">
      <input type="hidden" name="action" value="login">
      <div class="field-group">
        <label>Email address</label>
        <input type="email" name="email" placeholder="you@example.com" required autofocus
               value="<?= htmlspecialchars($_POST['email'] ?? '') ?>">
      </div>
      <div class="field-group">
        <label>Password</label>
        <input type="password" name="password" placeholder="••••••••" required>
      </div>
      <button type="submit" class="btn btn-primary">Sign in →</button>
    </form>

    <div class="demo-creds">
      <h4>Demo credentials</h4>
      <div class="demo-row"><span class="demo-label">Admin</span><span class="demo-val">alice@example.com / admin123</span></div>
      <div class="demo-row"><span class="demo-label">User</span><span class="demo-val">bob@example.com / user123</span></div>
    </div>
  </div>
</div>

<?php else: ?>
<!-- ══════════════════════════════════════════════════════
     APP SHELL
══════════════════════════════════════════════════════ -->
<div class="app-root">

  <!-- Sidebar -->
  <aside class="sidebar">
    <div class="sidebar-logo">Nexus<span>App</span></div>
    <nav class="nav">
      <a href="?page=dashboard" class="nav-item <?= $page==='dashboard'?'active':'' ?>">
        <svg width="16" height="16" fill="none" viewBox="0 0 24 24" stroke="currentColor" stroke-width="2"><rect x="3" y="3" width="7" height="7" rx="1"/><rect x="14" y="3" width="7" height="7" rx="1"/><rect x="3" y="14" width="7" height="7" rx="1"/><rect x="14" y="14" width="7" height="7" rx="1"/></svg>
        Dashboard
      </a>
      <?php if ($current_user['role'] === 'admin'): ?>
      <a href="?page=admin" class="nav-item <?= $page==='admin'?'active':'' ?>">
        <svg width="16" height="16" fill="none" viewBox="0 0 24 24" stroke="currentColor" stroke-width="2"><path d="M17 21v-2a4 4 0 0 0-4-4H5a4 4 0 0 0-4 4v2"/><circle cx="9" cy="7" r="4"/><path d="M23 21v-2a4 4 0 0 0-3-3.87M16 3.13a4 4 0 0 1 0 7.75"/></svg>
        Admin Panel
      </a>
      <?php endif; ?>
    </nav>
    <div class="sidebar-user">
      <div class="avatar"><?= htmlspecialchars($current_user['avatar']) ?></div>
      <div class="sidebar-user-info">
        <div class="sidebar-user-name"><?= htmlspecialchars($current_user['name']) ?></div>
        <div class="sidebar-user-role"><?= ucfirst($current_user['role']) ?></div>
      </div>
      <a href="?action=logout" title="Sign out" style="color:var(--muted);margin-left:auto;flex-shrink:0;">
        <svg width="16" height="16" fill="none" viewBox="0 0 24 24" stroke="currentColor" stroke-width="2"><path d="M9 21H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h4"/><polyline points="16 17 21 12 16 7"/><line x1="21" y1="12" x2="9" y2="12"/></svg>
      </a>
    </div>
  </aside>

  <!-- Main content -->
  <main class="main">

    <?php if ($success): ?>
      <div class="alert alert-success">
        <svg width="16" height="16" fill="none" viewBox="0 0 24 24" stroke="currentColor" stroke-width="2"><path d="M22 11.08V12a10 10 0 1 1-5.93-9.14"/><polyline points="22 4 12 14.01 9 11.01"/></svg>
        <?= htmlspecialchars($success) ?>
      </div>
    <?php endif; ?>
    <?php if ($errors): ?>
      <div class="alert alert-error">
        <svg width="16" height="16" fill="none" viewBox="0 0 24 24" stroke="currentColor" stroke-width="2"><circle cx="12" cy="12" r="10"/><path d="M12 8v4m0 4h.01" stroke-linecap="round"/></svg>
        <?= htmlspecialchars($errors[0]) ?>
      </div>
    <?php endif; ?>

    <?php if ($page === 'dashboard'): ?>
    <!-- ── USER DASHBOARD ────────────────────────────────── -->
    <div class="page-header">
      <h1 class="page-title">Welcome back, <?= htmlspecialchars(explode(' ', $current_user['name'])[0]) ?> 👋</h1>
      <p class="page-sub">Here's your account overview.</p>
    </div>

    <div class="profile-card">
      <div class="avatar lg"><?= htmlspecialchars($current_user['avatar']) ?></div>
      <div class="profile-info">
        <div class="profile-name"><?= htmlspecialchars($current_user['name']) ?></div>
        <div class="profile-email"><?= htmlspecialchars($current_user['email']) ?></div>
        <div class="profile-meta">
          <div class="meta-item">
            <strong><?= htmlspecialchars(ucfirst($current_user['role'])) ?></strong>
            Account role
          </div>
          <div class="meta-item">
            <strong>#<?= htmlspecialchars($current_user['id']) ?></strong>
            User ID
          </div>
          <div class="meta-item">
            <strong><?= htmlspecialchars($current_user['joined']) ?></strong>
            Member since
          </div>
        </div>
      </div>
      <div>
        <span class="tag <?= $current_user['role']==='admin'?'tag-admin':'tag-user' ?>"><?= $current_user['role'] ?></span>
      </div>
    </div>

    <div class="stats-grid">
      <div class="stat-card">
        <div class="stat-label">Account Status</div>
        <div class="stat-value" style="color:var(--accent);font-size:20px;padding-top:4px;">● Active</div>
        <div class="stat-sub">All systems operational</div>
      </div>
      <div class="stat-card">
        <div class="stat-label">Member Since</div>
        <div class="stat-value" style="font-size:22px;"><?= date('M Y', strtotime($current_user['joined'])) ?></div>
        <div class="stat-sub"><?= (int)((time() - strtotime($current_user['joined'])) / 86400) ?> days as a member</div>
      </div>
      <div class="stat-card">
        <div class="stat-label">Role</div>
        <div class="stat-value" style="font-size:22px;"><?= ucfirst($current_user['role']) ?></div>
        <div class="stat-sub"><?= $current_user['role']==='admin'?'Full system access':'Standard access' ?></div>
      </div>
    </div>

    <div class="activity-card">
      <div class="section-title">Recent Activity</div>
      <ul class="activity-list">
        <li class="activity-item">
          <span class="activity-dot"></span>
          <span class="activity-text">Signed in successfully</span>
          <span class="activity-time">Just now</span>
        </li>
        <li class="activity-item">
          <span class="activity-dot blue"></span>
          <span class="activity-text">Profile information viewed</span>
          <span class="activity-time">Just now</span>
        </li>
        <li class="activity-item">
          <span class="activity-dot warn"></span>
          <span class="activity-text">Account created</span>
          <span class="activity-time"><?= htmlspecialchars($current_user['joined']) ?></span>
        </li>
      </ul>
    </div>

    <?php elseif ($page === 'admin'): ?>
    <!-- ── ADMIN PANEL ────────────────────────────────────── -->
    <div class="page-header">
      <h1 class="page-title">Admin Panel</h1>
      <p class="page-sub">Manage all registered users.</p>
    </div>

    <div class="admin-toolbar">
      <span class="users-count">Showing <strong><?= count($users_db) ?></strong> registered users</span>
    </div>

    <div class="table-wrap">
      <table>
        <thead>
          <tr>
            <th>User</th>
            <th>Email</th>
            <th>Role</th>
            <th>Joined</th>
            <th>Actions</th>
          </tr>
        </thead>
        <tbody>
          <?php foreach ($users_db as $u): ?>
          <tr>
            <td>
              <div class="user-cell">
                <div class="avatar" style="<?= $u['role']==='admin' ? 'background:linear-gradient(135deg,#60a5fa 0%,#818cf8 100%)' : '' ?>"><?= htmlspecialchars($u['avatar']) ?></div>
                <div class="user-cell-info">
                  <div class="user-cell-name"><?= htmlspecialchars($u['name']) ?></div>
                  <div class="user-cell-id">#<?= $u['id'] ?></div>
                </div>
              </div>
            </td>
            <td style="color:var(--muted)"><?= htmlspecialchars($u['email']) ?></td>
            <td><span class="tag <?= $u['role']==='admin'?'tag-admin':'tag-user' ?>"><?= $u['role'] ?></span></td>
            <td style="color:var(--muted);font-size:13px"><?= htmlspecialchars($u['joined']) ?></td>
            <td>
              <?php if ($u['id'] !== $_SESSION['user_id']): ?>
                <a href="?page=admin&action=delete&id=<?= $u['id'] ?>"
                   class="btn btn-danger"
                   onclick="return confirm('Delete <?= addslashes(htmlspecialchars($u['name'])) ?>? This cannot be undone.')">
                  <svg width="13" height="13" fill="none" viewBox="0 0 24 24" stroke="currentColor" stroke-width="2"><polyline points="3 6 5 6 21 6"/><path d="M19 6l-1 14H6L5 6"/><path d="M10 11v6M14 11v6"/><path d="M9 6V4h6v2"/></svg>
                  Delete
                </a>
              <?php else: ?>
                <span style="font-size:12px;color:var(--muted)">You</span>
              <?php endif; ?>
            </td>
          </tr>
          <?php endforeach; ?>
        </tbody>
      </table>
    </div>

    <!-- Add user form -->
    <div class="add-user-card">
      <h3>➕ Add New User</h3>
      <form method="POST">
        <input type="hidden" name="action" value="add_user">
        <div class="form-grid">
          <div class="field-group">
            <label>Full Name</label>
            <input type="text" name="new_name" placeholder="Jane Doe" required>
          </div>
          <div class="field-group">
            <label>Email Address</label>
            <input type="email" name="new_email" placeholder="jane@example.com" required>
          </div>
          <div class="field-group">
            <label>Password</label>
            <input type="password" name="new_password" placeholder="••••••••" required>
          </div>
          <div class="field-group">
            <label>Role</label>
            <select name="new_role" style="width:100%;padding:11px 14px;border-radius:var(--radius);background:var(--surface2);border:1px solid var(--border);color:var(--text);font-size:14.5px;font-family:var(--font-body);outline:none;">
              <option value="user">User</option>
              <option value="admin">Admin</option>
            </select>
          </div>
          <div class="field-group span2" style="margin-bottom:0">
            <button type="submit" class="btn btn-primary">Create User</button>
          </div>
        </div>
      </form>
    </div>

    <?php endif; ?>
  </main>
</div>
<?php endif; ?>

</body>
</html>