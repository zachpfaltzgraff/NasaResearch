<?php
/**
 * Single-file Employee Management System (PHP + MySQL)
 *
 * Features:
 * - Login (username/password), roles: employee, manager
 * - Employee: view own profile
 * - Manager: view all profiles and update roles
 * - Simple navigation (profile/manage/logout)
 *
 * Setup:
 * 1) Create a MySQL database (e.g., employee_app)
 * 2) Configure DB settings below
 * 3) Visit: /index.php?page=init  (optional, creates tables + seeds demo users)
 * 4) Login with seeded accounts (if you used init):
 *      manager: alice / Password123!
 *      employee: bob   / Password123!
 */

declare(strict_types=1);
session_start();

/* =========================
   CONFIG
   ========================= */
$dbHost = '127.0.0.1';
$dbName = 'employee_app';
$dbUser = 'root';
$dbPass = '';
$dsn = "mysql:host={$dbHost};dbname={$dbName};charset=utf8mb4";

/* =========================
   DB CONNECT
   ========================= */
function db(): PDO {
    static $pdo = null;
    if ($pdo instanceof PDO) return $pdo;

    global $dsn, $dbUser, $dbPass;
    $pdo = new PDO($dsn, $dbUser, $dbPass, [
        PDO::ATTR_ERRMODE => PDO::ERRMODE_EXCEPTION,
        PDO::ATTR_DEFAULT_FETCH_MODE => PDO::FETCH_ASSOC,
    ]);
    return $pdo;
}

/* =========================
   HELPERS
   ========================= */
function h(string $s): string { return htmlspecialchars($s, ENT_QUOTES | ENT_SUBSTITUTE, 'UTF-8'); }

function redirect(string $to): never {
    header("Location: {$to}");
    exit;
}

function current_user(): ?array {
    if (empty($_SESSION['uid'])) return null;

    $stmt = db()->prepare("SELECT id, username, role, employee_id, full_name, position FROM users WHERE id = ?");
    $stmt->execute([$_SESSION['uid']]);
    $u = $stmt->fetch();
    if (!$u) return null;
    return $u;
}

function require_login(): array {
    $u = current_user();
    if (!$u) redirect('?page=login');
    return $u;
}

function require_manager(array $u): void {
    if (($u['role'] ?? '') !== 'manager') {
        http_response_code(403);
        echo render_layout("Forbidden", "<p>403 - Forbidden (manager access required)</p>");
        exit;
    }
}

function is_role_valid(string $role): bool {
    return in_array($role, ['employee', 'manager'], true);
}

function csrf_token(): string {
    if (empty($_SESSION['csrf'])) {
        $_SESSION['csrf'] = bin2hex(random_bytes(32));
    }
    return $_SESSION['csrf'];
}

function csrf_verify(?string $token): bool {
    return is_string($token) && !empty($_SESSION['csrf']) && hash_equals($_SESSION['csrf'], $token);
}

/* =========================
   RENDERING
   ========================= */
function render_layout(string $title, string $bodyHtml, ?array $user = null): string {
    $nav = render_nav($user);
    $year = date('Y');

    return <<<HTML
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>{$title}</title>
  <style>
    body { font-family: system-ui, -apple-system, Segoe UI, Roboto, Arial, sans-serif; margin: 0; background:#f6f7fb; color:#111; }
    header { background:#111827; color:#fff; padding: 14px 18px; }
    header .row { display:flex; justify-content:space-between; gap: 12px; align-items:center; flex-wrap: wrap; }
    header a { color:#fff; text-decoration:none; }
    main { max-width: 980px; margin: 20px auto; padding: 0 16px; }
    .card { background:#fff; border:1px solid #e5e7eb; border-radius: 10px; padding: 16px; box-shadow: 0 1px 2px rgba(0,0,0,.03); }
    .muted { color:#6b7280; }
    .nav a { margin-right: 10px; padding: 6px 10px; border-radius: 8px; display:inline-block; }
    .nav a:hover { background: rgba(255,255,255,.12); }
    .pill { display:inline-block; padding: 2px 8px; border-radius:999px; font-size: 12px; background:#eef2ff; color:#3730a3; }
    table { width:100%; border-collapse: collapse; margin-top: 10px; }
    th, td { text-align:left; padding: 10px 8px; border-bottom: 1px solid #e5e7eb; vertical-align: top; }
    th { font-size: 13px; color:#374151; }
    input, select { padding: 8px 10px; border-radius: 8px; border: 1px solid #d1d5db; width: 100%; box-sizing: border-box; }
    label { display:block; margin: 10px 0 6px; font-size: 13px; color:#374151; }
    button { padding: 9px 12px; border:0; border-radius: 9px; background:#2563eb; color:#fff; cursor:pointer; }
    button.secondary { background:#6b7280; }
    .row { display:flex; gap: 12px; flex-wrap: wrap; }
    .col { flex: 1 1 260px; }
    .msg { padding: 10px 12px; border-radius: 10px; margin-bottom: 12px; }
    .msg.ok { background:#ecfdf5; border:1px solid #a7f3d0; color:#065f46; }
    .msg.err { background:#fef2f2; border:1px solid #fecaca; color:#991b1b; }
    footer { max-width: 980px; margin: 18px auto 28px; padding: 0 16px; color:#6b7280; font-size: 13px; }
    code { background:#f3f4f6; padding: 2px 6px; border-radius: 6px; }
  </style>
</head>
<body>
<header>
  <div class="row">
    <div>
      <strong>Employee Management</strong>
      <span class="muted" style="color:#cbd5e1;">(single-file PHP)</span>
    </div>
    <div class="nav">{$nav}</div>
  </div>
</header>
<main>
  {$bodyHtml}
</main>
<footer>
  <div>© {$year} — Demo app. Use HTTPS + secure secrets in real deployments.</div>
</footer>
</body>
</html>
HTML;
}

function render_nav(?array $user): string {
    if (!$user) {
        return '<a href="?page=login">Login</a> <a href="?page=init">Init DB</a>';
    }

    $links = [];
    $links[] = '<a href="?page=profile">Profile</a>';
    if (($user['role'] ?? '') === 'manager') {
        $links[] = '<a href="?page=manage">Manage Employees</a>';
    }
    $links[] = '<a href="?page=logout">Logout</a>';

    $who = h($user['username'] . ' (' . $user['role'] . ')');
    return '<span class="pill">' . $who . '</span> ' . implode(' ', $links);
}

function msg_html(?string $ok = null, ?string $err = null): string {
    $out = '';
    if ($ok)  $out .= '<div class="msg ok">' . h($ok) . '</div>';
    if ($err) $out .= '<div class="msg err">' . h($err) . '</div>';
    return $out;
}

/* =========================
   ROUTES
   ========================= */
$page = $_GET['page'] ?? 'profile';

try {
    switch ($page) {
        case 'init':
            echo handle_init();
            break;

        case 'login':
            echo handle_login();
            break;

        case 'logout':
            $_SESSION = [];
            if (ini_get("session.use_cookies")) {
                $params = session_get_cookie_params();
                setcookie(session_name(), '', time() - 42000,
                    $params["path"], $params["domain"],
                    $params["secure"], $params["httponly"]
                );
            }
            session_destroy();
            redirect('?page=login');
            break;

        case 'profile':
            echo handle_profile();
            break;

        case 'manage':
            echo handle_manage();
            break;

        default:
            http_response_code(404);
            echo render_layout("Not found", "<div class='card'><p>404 - Page not found.</p></div>", current_user());
            break;
    }
} catch (Throwable $e) {
    http_response_code(500);
    $safe = h($e->getMessage());
    echo render_layout("Error", "<div class='card'><p><strong>Server error:</strong> {$safe}</p></div>", current_user());
}

/* =========================
   HANDLERS
   ========================= */
function handle_init(): string {
    // NOTE: In real apps, do not expose schema init publicly.
    $pdo = db();

    $pdo->exec("
        CREATE TABLE IF NOT EXISTS users (
            id INT AUTO_INCREMENT PRIMARY KEY,
            username VARCHAR(60) NOT NULL UNIQUE,
            password_hash VARCHAR(255) NOT NULL,
            role ENUM('employee','manager') NOT NULL DEFAULT 'employee',
            employee_id VARCHAR(30) NOT NULL,
            full_name VARCHAR(120) NOT NULL,
            position VARCHAR(120) NOT NULL,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
    ");

    // Seed demo users if not present
    $seed = [
        [
            'username' => 'alice',
            'password' => 'Password123!',
            'role' => 'manager',
            'employee_id' => 'M-1001',
            'full_name' => 'Alice Manager',
            'position' => 'Engineering Manager',
        ],
        [
            'username' => 'bob',
            'password' => 'Password123!',
            'role' => 'employee',
            'employee_id' => 'E-2001',
            'full_name' => 'Bob Employee',
            'position' => 'Software Engineer',
        ],
    ];

    $created = 0;
    $skipped = 0;

    $check = $pdo->prepare("SELECT id FROM users WHERE username = ?");
    $ins = $pdo->prepare("
        INSERT INTO users (username, password_hash, role, employee_id, full_name, position)
        VALUES (?, ?, ?, ?, ?, ?)
    ");

    foreach ($seed as $u) {
        $check->execute([$u['username']]);
        if ($check->fetch()) {
            $skipped++;
            continue;
        }
        $ins->execute([
            $u['username'],
            password_hash($u['password'], PASSWORD_DEFAULT),
            $u['role'],
            $u['employee_id'],
            $u['full_name'],
            $u['position'],
        ]);
        $created++;
    }

    $body = "<div class='card'>
        <h2>Database initialized</h2>
        " . msg_html("Tables ensured. Seeded: {$created}, already existed: {$skipped}.") . "
        <p>Try logging in at <a href='?page=login'>Login</a>.</p>
        <p class='muted'>Seeded accounts (if newly created): <code>alice / Password123!</code> (manager), <code>bob / Password123!</code> (employee)</p>
    </div>";

    return render_layout("Init DB", $body, current_user());
}

function handle_login(): string {
    $user = current_user();
    if ($user) redirect('?page=profile');

    $ok = null; $err = null;

    if ($_SERVER['REQUEST_METHOD'] === 'POST') {
        $username = trim((string)($_POST['username'] ?? ''));
        $password = (string)($_POST['password'] ?? '');
        $token = (string)($_POST['csrf'] ?? '');

        if (!csrf_verify($token)) {
            $err = "Invalid CSRF token. Refresh and try again.";
        } elseif ($username === '' || $password === '') {
            $err = "Username and password are required.";
        } else {
            $stmt = db()->prepare("SELECT id, username, password_hash, role FROM users WHERE username = ?");
            $stmt->execute([$username]);
            $row = $stmt->fetch();

            if (!$row || !password_verify($password, $row['password_hash'])) {
                $err = "Invalid username or password.";
            } else {
                session_regenerate_id(true);
                $_SESSION['uid'] = (int)$row['id'];
                $ok = "Logged in successfully.";
                redirect('?page=profile');
            }
        }
    }

    $token = h(csrf_token());
    $body = "<div class='card'>
        <h2>Login</h2>
        " . msg_html($ok, $err) . "
        <form method='post' action='?page=login'>
          <input type='hidden' name='csrf' value='{$token}'>
          <label>Username</label>
          <input name='username' autocomplete='username' required>
          <label>Password</label>
          <input name='password' type='password' autocomplete='current-password' required>
          <div style='margin-top:12px;'>
            <button type='submit'>Sign in</button>
            <a class='muted' style='margin-left:10px;' href='?page=init'>Init DB</a>
          </div>
        </form>
        <p class='muted' style='margin-top:12px;'>If you ran init, try <code>alice / Password123!</code> or <code>bob / Password123!</code>.</p>
    </div>";

    return render_layout("Login", $body, null);
}

function handle_profile(): string {
    $u = require_login();

    $body = "<div class='card'>
      <h2>Your Profile</h2>
      <table>
        <tr><th>Username</th><td>" . h($u['username']) . "</td></tr>
        <tr><th>Full name</th><td>" . h($u['full_name']) . "</td></tr>
        <tr><th>Employee ID</th><td>" . h($u['employee_id']) . "</td></tr>
        <tr><th>Position</th><td>" . h($u['position']) . "</td></tr>
        <tr><th>Role</th><td><span class='pill'>" . h($u['role']) . "</span></td></tr>
      </table>
      <p class='muted' style='margin-top:10px;'>Employees can only view their own profile. Managers can manage employee roles.</p>
    </div>";

    return render_layout("Profile", $body, $u);
}

function handle_manage(): string {
    $u = require_login();
    require_manager($u);

    $ok = null; $err = null;

    if ($_SERVER['REQUEST_METHOD'] === 'POST') {
        $token = (string)($_POST['csrf'] ?? '');
        if (!csrf_verify($token)) {
            $err = "Invalid CSRF token.";
        } else {
            $targetUserId = (int)($_POST['user_id'] ?? 0);
            $newRole = (string)($_POST['new_role'] ?? '');

            if ($targetUserId <= 0 || !is_role_valid($newRole)) {
                $err = "Invalid request.";
            } else {
                // Prevent manager from demoting themselves (optional safety)
                if ($targetUserId === (int)$u['id'] && $newRole !== 'manager') {
                    $err = "You cannot change your own role away from manager.";
                } else {
                    $stmt = db()->prepare("UPDATE users SET role = ? WHERE id = ?");
                    $stmt->execute([$newRole, $targetUserId]);
                    $ok = "Role updated.";
                }
            }
        }
    }

    $rows = db()->query("SELECT id, username, role, employee_id, full_name, position FROM users ORDER BY full_name ASC")->fetchAll();
    $token = h(csrf_token());

    $tableRows = '';
    foreach ($rows as $r) {
        $rid = (int)$r['id'];
        $role = (string)$r['role'];

        $options = '';
        foreach (['employee', 'manager'] as $opt) {
            $sel = ($opt === $role) ? 'selected' : '';
            $options .= "<option value='{$opt}' {$sel}>{$opt}</option>";
        }

        $tableRows .= "<tr>
          <td>" . h($r['full_name']) . "</td>
          <td>" . h($r['employee_id']) . "</td>
          <td>" . h($r['position']) . "</td>
          <td><span class='pill'>" . h($role) . "</span></td>
          <td>" . h($r['username']) . "</td>
          <td style='min-width:260px;'>
            <form method='post' action='?page=manage' class='row' style='gap:8px; align-items:center;'>
              <input type='hidden' name='csrf' value='{$token}'>
              <input type='hidden' name='user_id' value='{$rid}'>
              <div class='col' style='flex: 1 1 120px;'>
                <select name='new_role'>{$options}</select>
              </div>
              <div>
                <button type='submit'>Update</button>
              </div>
            </form>
          </td>
        </tr>";
    }

    $body = "<div class='card'>
      <h2>Manage Employees</h2>
      " . msg_html($ok, $err) . "
      <p class='muted'>As a manager, you can view all users and update their roles.</p>
      <table>
        <thead>
          <tr>
            <th>Name</th>
            <th>Employee ID</th>
            <th>Position</th>
            <th>Role</th>
            <th>Username</th>
            <th>Change role</th>
          </tr>
        </thead>
        <tbody>
          {$tableRows}
        </tbody>
      </table>
    </div>";

    return render_layout("Manage", $body, $u);
}
?>