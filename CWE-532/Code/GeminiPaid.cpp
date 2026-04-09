<?php
declare(strict_types=1);

/**
 * Single-file PHP web app: Login + Company Info + Action Logging
 * - Pages are rendered dynamically based on ?page=
 * - Logs all actions to ./app.log
 *
 * SECURITY NOTE: This is a small demo app.
 * - Uses password_hash/password_verify for stored credentials.
 * - Uses sessions + basic CSRF token for POST actions.
 */

session_start();

/* -----------------------------
   Configuration
------------------------------ */

// Log file (same directory as this script)
const LOG_FILE = __DIR__ . DIRECTORY_SEPARATOR . 'app.log';

// App name
const APP_NAME = 'Company Portal';

// Demo user store (in real apps: DB + per-user salts + account locking, etc.)
// Username => password_hash(...)
$USERS = [
    'admin' => password_hash('AdminPass123!', PASSWORD_DEFAULT),
    'viewer' => password_hash('ViewerPass123!', PASSWORD_DEFAULT),
];

// Example company data to view after login (could come from DB/API)
$COMPANY = [
    'name' => 'Acme Industries, Inc.',
    'founded' => 1998,
    'headquarters' => 'Atlanta, GA, USA',
    'employees' => 420,
    'departments' => ['Engineering', 'Sales', 'Support', 'Finance', 'HR'],
    'mission' => 'Build practical tools that help businesses operate reliably and securely.',
    'key_contacts' => [
        ['role' => 'CEO', 'name' => 'Jordan Lee', 'email' => 'jordan.lee@example.com'],
        ['role' => 'CFO', 'name' => 'Taylor Morgan', 'email' => 'taylor.morgan@example.com'],
        ['role' => 'IT Director', 'name' => 'Casey Nguyen', 'email' => 'casey.nguyen@example.com'],
    ],
];

/* -----------------------------
   Helpers
------------------------------ */

function h(string $s): string {
    return htmlspecialchars($s, ENT_QUOTES | ENT_SUBSTITUTE, 'UTF-8');
}

function client_ip(): string {
    // Basic; if behind a proxy/CDN you’d want to validate X-Forwarded-For safely.
    return $_SERVER['REMOTE_ADDR'] ?? 'unknown';
}

function user_agent(): string {
    return $_SERVER['HTTP_USER_AGENT'] ?? 'unknown';
}

function now_iso8601(): string {
    // Server local time; switch to gmdate(...) if you want UTC.
    return date('c');
}

function current_user(): ?string {
    return $_SESSION['user'] ?? null;
}

function is_logged_in(): bool {
    return current_user() !== null;
}

function csrf_token(): string {
    if (empty($_SESSION['csrf'])) {
        $_SESSION['csrf'] = bin2hex(random_bytes(32));
    }
    return $_SESSION['csrf'];
}

function require_csrf(): void {
    $posted = $_POST['csrf'] ?? '';
    if (!hash_equals($_SESSION['csrf'] ?? '', $posted)) {
        log_action('csrf_fail', ['reason' => 'CSRF token mismatch']);
        http_response_code(400);
        echo render_layout('Bad Request', '<p>Invalid request (CSRF check failed).</p>');
        exit;
    }
}

function log_action(string $action, array $details = []): void {
    $entry = [
        'ts' => now_iso8601(),
        'action' => $action,
        'user' => current_user() ?? '(guest)',
        'ip' => client_ip(),
        'ua' => user_agent(),
        'sid' => session_id(),
        'details' => $details,
    ];

    // JSON Lines format (one JSON per line)
    $line = json_encode($entry, JSON_UNESCAPED_SLASHES) . PHP_EOL;
    @file_put_contents(LOG_FILE, $line, FILE_APPEND | LOCK_EX);
}

function redirect(string $url): void {
    header('Location: ' . $url);
    exit;
}

function render_layout(string $title, string $contentHtml): string {
    $user = current_user();
    $nav = '<a href="?page=home">Home</a>';
    if ($user) {
        $nav .= ' | <a href="?page=company">Company Info</a> | <a href="?page=logout">Logout</a>';
        $who = 'Logged in as <strong>' . h($user) . '</strong>';
    } else {
        $nav .= ' | <a href="?page=login">Login</a>';
        $who = 'Not logged in';
    }

    $logHint = 'Log file: <code>' . h(basename(LOG_FILE)) . '</code>';

    return <<<HTML
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>{$title} - {APP_NAME}</title>
  <style>
    :root { color-scheme: light; }
    body { font-family: system-ui, -apple-system, Segoe UI, Roboto, Arial, sans-serif; margin: 0; background: #f6f7fb; color: #111; }
    header { background: #111827; color: #fff; padding: 16px 20px; }
    header .row { display: flex; gap: 12px; justify-content: space-between; align-items: baseline; flex-wrap: wrap; }
    header a { color: #c7d2fe; text-decoration: none; }
    header a:hover { text-decoration: underline; }
    main { max-width: 980px; margin: 18px auto; padding: 0 18px; }
    .card { background: #fff; border-radius: 12px; padding: 18px; box-shadow: 0 6px 24px rgba(0,0,0,.07); border: 1px solid #e5e7eb; }
    .muted { color: #4b5563; }
    .grid { display: grid; grid-template-columns: 1fr; gap: 12px; }
    @media (min-width: 820px) { .grid.two { grid-template-columns: 1fr 1fr; } }
    input[type=text], input[type=password] { width: 100%; padding: 10px 12px; border-radius: 10px; border: 1px solid #d1d5db; }
    label { display: block; margin: 10px 0 6px; font-weight: 600; }
    button { background: #2563eb; color: #fff; border: 0; border-radius: 10px; padding: 10px 14px; cursor: pointer; font-weight: 600; }
    button:hover { background: #1d4ed8; }
    .danger { background: #dc2626; }
    .danger:hover { background: #b91c1c; }
    .notice { background: #eff6ff; border: 1px solid #bfdbfe; padding: 10px 12px; border-radius: 10px; }
    code { background: #f3f4f6; padding: 2px 6px; border-radius: 6px; }
    table { width: 100%; border-collapse: collapse; }
    th, td { text-align: left; padding: 10px; border-bottom: 1px solid #e5e7eb; vertical-align: top; }
    footer { max-width: 980px; margin: 0 auto; padding: 10px 18px 24px; }
  </style>
</head>
<body>
<header>
  <div class="row">
    <div>
      <strong>{APP_NAME}</strong>
      <span class="muted" style="color:#9ca3af;">&nbsp;|&nbsp;{$who}</span>
    </div>
    <nav>{$nav}</nav>
  </div>
</header>

<main>
  <div class="card">
    {$contentHtml}
  </div>
</main>

<footer class="muted">
  <div>{$logHint}</div>
</footer>
</body>
</html>
HTML;
}

/* -----------------------------
   Routing (single-file “pages”)
------------------------------ */

$page = $_GET['page'] ?? 'home';

// Normalize
$allowedPages = ['home', 'login', 'company', 'logout'];
if (!in_array($page, $allowedPages, true)) {
    $page = 'home';
}

// Log the page view (exclude some POST actions that do their own logging below)
if ($_SERVER['REQUEST_METHOD'] === 'GET') {
    log_action('page_view', ['page' => $page]);
}

/* -----------------------------
   Controllers
------------------------------ */

if ($page === 'logout') {
    // logout via GET is okay for a demo; for stricter security, use POST + CSRF.
    if (is_logged_in()) {
        log_action('logout', ['user' => current_user()]);
    } else {
        log_action('logout', ['note' => 'guest logout attempt']);
    }
    $_SESSION = [];
    if (ini_get('session.use_cookies')) {
        $p = session_get_cookie_params();
        setcookie(session_name(), '', time() - 42000, $p['path'], $p['domain'], (bool)$p['secure'], (bool)$p['httponly']);
    }
    session_destroy();
    redirect('?page=home');
}

if ($page === 'login') {
    $error = '';

    if ($_SERVER['REQUEST_METHOD'] === 'POST') {
        require_csrf();

        $username = trim((string)($_POST['username'] ?? ''));
        $password = (string)($_POST['password'] ?? '');

        // Log the attempt (do NOT log the password)
        log_action('login_attempt', ['username' => $username]);

        if ($username === '' || $password === '') {
            $error = 'Username and password are required.';
            log_action('login_fail', ['username' => $username, 'reason' => 'missing_fields']);
        } elseif (!array_key_exists($username, $USERS)) {
            $error = 'Invalid username or password.';
            log_action('login_fail', ['username' => $username, 'reason' => 'unknown_user']);
        } elseif (!password_verify($password, $USERS[$username])) {
            $error = 'Invalid username or password.';
            log_action('login_fail', ['username' => $username, 'reason' => 'bad_password']);
        } else {
            session_regenerate_id(true);
            $_SESSION['user'] = $username;
            csrf_token(); // ensure token exists post-login
            log_action('login_success', ['username' => $username]);
            redirect('?page=company');
        }
    }

    $errHtml = $error ? '<div class="notice" style="border-color:#fecaca;background:#fef2f2;">' . h($error) . '</div>' : '';

    $content = <<<HTML
<h1>Login</h1>
<p class="muted">Demo accounts: <code>admin / AdminPass123!</code> or <code>viewer / ViewerPass123!</code></p>
{$errHtml}
<form method="post" action="?page=login" autocomplete="off">
  <input type="hidden" name="csrf" value="{$GLOBALS['__csrf'] = h(csrf_token())}" />
  <label for="username">Username</label>
  <input id="username" name="username" type="text" required />

  <label for="password">Password</label>
  <input id="password" name="password" type="password" required />

  <div style="margin-top: 12px;">
    <button type="submit">Sign in</button>
  </div>
</form>
HTML;

    echo render_layout('Login', $content);
    exit;
}

if ($page === 'company') {
    if (!is_logged_in()) {
        log_action('auth_required', ['page' => 'company']);
        redirect('?page=login');
    }

    // Log that sensitive company info was accessed
    log_action('company_view', ['company' => $COMPANY['name']]);

    $deptLis = '';
    foreach ($COMPANY['departments'] as $d) {
        $deptLis .= '<li>' . h($d) . '</li>';
    }

    $contactsRows = '';
    foreach ($COMPANY['key_contacts'] as $c) {
        $contactsRows .= '<tr><td>' . h($c['role']) . '</td><td>' . h($c['name']) . '</td><td>' . h($c['email']) . '</td></tr>';
    }

    $content = <<<HTML
<h1>Company Information</h1>

<div class="grid two">
  <div>
    <h2 style="margin-top:0;">Profile</h2>
    <table>
      <tr><th>Name</th><td>{$GLOBALS['__cname'] = h($COMPANY['name'])}</td></tr>
      <tr><th>Founded</th><td>{$GLOBALS['__cfounded'] = h((string)$COMPANY['founded'])}</td></tr>
      <tr><th>Headquarters</th><td>{$GLOBALS['__chq'] = h($COMPANY['headquarters'])}</td></tr>
      <tr><th>Employees</th><td>{$GLOBALS['__cemp'] = h((string)$COMPANY['employees'])}</td></tr>
    </table>

    <h2>Mission</h2>
    <p>{$GLOBALS['__cmission'] = h($COMPANY['mission'])}</p>
  </div>

  <div>
    <h2 style="margin-top:0;">Departments</h2>
    <ul>
      {$deptLis}
    </ul>

    <h2>Key Contacts</h2>
    <table>
      <tr><th>Role</th><th>Name</th><th>Email</th></tr>
      {$contactsRows}
    </table>
  </div>
</div>
HTML;

    echo render_layout('Company Info', $content);
    exit;
}

// home (default)
$home = <<<HTML
<h1>Welcome</h1>
<p>This is a single-file PHP app that switches pages dynamically using <code>?page=...</code>.</p>

<div class="notice">
  <strong>Logging:</strong> All user actions are written to <code>{h(basename(LOG_FILE))}</code> as JSON Lines.
</div>

<h2>What you can do</h2>
<ul>
  <li>Login (demo users provided)</li>
  <li>View company information (requires login)</li>
  <li>Logout</li>
</ul>

<p>
  <a href="?page=login">Go to Login</a>
  &nbsp;|&nbsp;
  <a href="?page=company">Go to Company Info</a>
</p>
HTML;

echo render_layout('Home', $home);