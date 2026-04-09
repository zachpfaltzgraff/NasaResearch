<?php
/**
 * Single-file PHP app with:
 * - Login + registration
 * - Roles: user/admin
 * - "dashboard.php" and "admin.php" behavior via routing (?page=dashboard|admin)
 * - Admin can list users and delete accounts
 *
 * Run locally:
 *   php -S localhost:8000
 * Then visit:
 *   http://localhost:8000/app.php
 */

declare(strict_types=1);
session_start();

/* -----------------------------
   Config
------------------------------ */
const DB_FILE = __DIR__ . '/app.sqlite';
const APP_TITLE = 'Single-File PHP App';
const DEFAULT_ADMIN_EMAIL = 'admin@example.com';
const DEFAULT_ADMIN_PASSWORD = 'admin123'; // change immediately in real use

/* -----------------------------
   DB helpers
------------------------------ */
function db(): PDO {
    static $pdo = null;
    if ($pdo instanceof PDO) return $pdo;

    $pdo = new PDO('sqlite:' . DB_FILE, null, null, [
        PDO::ATTR_ERRMODE => PDO::ERRMODE_EXCEPTION,
        PDO::ATTR_DEFAULT_FETCH_MODE => PDO::FETCH_ASSOC,
    ]);

    $pdo->exec("
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            email TEXT NOT NULL UNIQUE,
            name TEXT NOT NULL,
            password_hash TEXT NOT NULL,
            role TEXT NOT NULL CHECK(role IN ('user', 'admin')),
            created_at TEXT NOT NULL
        );
    ");

    // Create a default admin if none exists
    $stmt = $pdo->prepare("SELECT COUNT(*) AS c FROM users WHERE role='admin'");
    $stmt->execute();
    $countAdmins = (int)$stmt->fetch()['c'];

    if ($countAdmins === 0) {
        $stmt = $pdo->prepare("
            INSERT OR IGNORE INTO users (email, name, password_hash, role, created_at)
            VALUES (:email, :name, :password_hash, 'admin', :created_at)
        ");
        $stmt->execute([
            ':email' => DEFAULT_ADMIN_EMAIL,
            ':name' => 'Default Admin',
            ':password_hash' => password_hash(DEFAULT_ADMIN_PASSWORD, PASSWORD_DEFAULT),
            ':created_at' => gmdate('c'),
        ]);
    }

    return $pdo;
}

/* -----------------------------
   Auth helpers
------------------------------ */
function current_user(): ?array {
    if (!isset($_SESSION['user_id'])) return null;
    $stmt = db()->prepare("SELECT id, email, name, role, created_at FROM users WHERE id = :id");
    $stmt->execute([':id' => (int)$_SESSION['user_id']]);
    $u = $stmt->fetch();
    return $u ?: null;
}

function require_login(): void {
    if (!current_user()) {
        redirect('?page=login');
    }
}

function require_admin(): void {
    $u = current_user();
    if (!$u) redirect('?page=login');
    if ($u['role'] !== 'admin') {
        http_response_code(403);
        render_page('Forbidden', function() {
            echo "<h2>403 Forbidden</h2>";
            echo "<p>You must be an admin to access this page.</p>";
            echo '<p><a href="?page=dashboard">Go to Dashboard</a></p>';
        });
        exit;
    }
}

function redirect(string $url): void {
    header("Location: $url");
    exit;
}

/* -----------------------------
   CSRF helpers
------------------------------ */
function csrf_token(): string {
    if (empty($_SESSION['csrf'])) {
        $_SESSION['csrf'] = bin2hex(random_bytes(32));
    }
    return $_SESSION['csrf'];
}

function verify_csrf(): void {
    $t = $_POST['csrf'] ?? '';
    if (!is_string($t) || !hash_equals($_SESSION['csrf'] ?? '', $t)) {
        http_response_code(400);
        render_page('Bad Request', function() {
            echo "<h2>400 Bad Request</h2>";
            echo "<p>Invalid CSRF token.</p>";
            echo '<p><a href="?page=dashboard">Back</a></p>';
        });
        exit;
    }
}

/* -----------------------------
   Rendering
------------------------------ */
function h(string $s): string { return htmlspecialchars($s, ENT_QUOTES | ENT_SUBSTITUTE, 'UTF-8'); }

function render_page(string $title, callable $body): void {
    $u = current_user();
    $nav = function() use ($u) {
        echo '<div class="nav">';
        echo '<a href="?page=dashboard">Dashboard</a>';
        if ($u && $u['role'] === 'admin') {
            echo '<a href="?page=admin">Admin Panel</a>';
        }
        if ($u) {
            echo '<span class="spacer"></span>';
            echo '<span class="who">Logged in as <strong>' . h($u['email']) . '</strong> (' . h($u['role']) . ')</span>';
            echo '<a class="btn" href="?page=logout">Logout</a>';
        } else {
            echo '<span class="spacer"></span>';
            echo '<a class="btn" href="?page=login">Login</a>';
            echo '<a class="btn" href="?page=register">Register</a>';
        }
        echo '</div>';
    };

    ?>
    <!doctype html>
    <html lang="en">
    <head>
        <meta charset="utf-8" />
        <meta name="viewport" content="width=device-width, initial-scale=1" />
        <title><?= h(APP_TITLE . ' - ' . $title) ?></title>
        <style>
            body { font-family: system-ui, -apple-system, Segoe UI, Roboto, Arial, sans-serif; margin: 0; background: #0b1220; color: #e7eefc; }
            .nav { display:flex; gap: 12px; align-items:center; padding: 14px 18px; background:#101a33; border-bottom: 1px solid #22315c; }
            .nav a { color:#cfe1ff; text-decoration:none; padding: 6px 10px; border-radius: 8px; }
            .nav a:hover { background: #182754; }
            .spacer { flex:1; }
            .who { color:#9fb6e8; }
            .btn { border: 1px solid #2b3e78; background:#142149; }
            .wrap { max-width: 980px; margin: 0 auto; padding: 18px; }
            .card { background:#101a33; border: 1px solid #22315c; border-radius: 14px; padding: 16px; margin: 14px 0; }
            input, select { width: 100%; padding: 10px; border-radius: 10px; border:1px solid #2b3e78; background:#0b1220; color:#e7eefc; }
            label { display:block; margin: 10px 0 6px; color:#b9cdf8; }
            button { padding: 10px 14px; border-radius: 10px; border:1px solid #2b3e78; background:#182754; color:#e7eefc; cursor:pointer; }
            button:hover { background:#1d2f66; }
            table { width:100%; border-collapse: collapse; }
            th, td { text-align:left; padding: 10px; border-bottom: 1px solid #22315c; }
            .danger { border-color:#7a2b2b; background:#3a1212; }
            .danger:hover { background:#4a1717; }
            .muted { color:#9fb6e8; }
            .row { display:grid; grid-template-columns: 1fr 1fr; gap: 14px; }
            @media (max-width: 800px) { .row { grid-template-columns: 1fr; } }
            .flash { padding: 10px 12px; border-radius: 12px; background:#122047; border:1px solid #22315c; margin: 12px 0; }
        </style>
    </head>
    <body>
        <?php $nav(); ?>
        <div class="wrap">
            <h1><?= h($title) ?></h1>

            <?php if (!empty($_SESSION['flash'])): ?>
                <div class="flash"><?= h((string)$_SESSION['flash']) ?></div>
                <?php unset($_SESSION['flash']); ?>
            <?php endif; ?>

            <?php $body(); ?>
        </div>
    </body>
    </html>
    <?php
}

function flash(string $msg): void {
    $_SESSION['flash'] = $msg;
}

/* -----------------------------
   Actions (POST handlers)
------------------------------ */
function handle_register(): void {
    verify_csrf();

    $email = trim((string)($_POST['email'] ?? ''));
    $name  = trim((string)($_POST['name'] ?? ''));
    $pass  = (string)($_POST['password'] ?? '');

    if ($email === '' || $name === '' || $pass === '') {
        flash('All fields are required.');
        redirect('?page=register');
    }
    if (!filter_var($email, FILTER_VALIDATE_EMAIL)) {
        flash('Invalid email.');
        redirect('?page=register');
    }
    if (strlen($pass) < 6) {
        flash('Password must be at least 6 characters.');
        redirect('?page=register');
    }

    $stmt = db()->prepare("
        INSERT INTO users (email, name, password_hash, role, created_at)
        VALUES (:email, :name, :hash, 'user', :created_at)
    ");

    try {
        $stmt->execute([
            ':email' => $email,
            ':name' => $name,
            ':hash' => password_hash($pass, PASSWORD_DEFAULT),
            ':created_at' => gmdate('c'),
        ]);
    } catch (Throwable $e) {
        flash('That email is already registered.');
        redirect('?page=register');
    }

    flash('Registration successful. Please log in.');
    redirect('?page=login');
}

function handle_login(): void {
    verify_csrf();

    $email = trim((string)($_POST['email'] ?? ''));
    $pass  = (string)($_POST['password'] ?? '');

    $stmt = db()->prepare("SELECT id, password_hash FROM users WHERE email = :email");
    $stmt->execute([':email' => $email]);
    $u = $stmt->fetch();

    if (!$u || !password_verify($pass, $u['password_hash'])) {
        flash('Invalid email or password.');
        redirect('?page=login');
    }

    // Prevent session fixation
    session_regenerate_id(true);

    $_SESSION['user_id'] = (int)$u['id'];
    flash('Logged in.');
    redirect('?page=dashboard');
}

function handle_logout(): void {
    $_SESSION = [];
    if (ini_get("session.use_cookies")) {
        $p = session_get_cookie_params();
        setcookie(session_name(), '', time() - 42000, $p["path"], $p["domain"], (bool)$p["secure"], (bool)$p["httponly"]);
    }
    session_destroy();
    redirect('?page=login');
}

function handle_delete_user(): void {
    require_admin();
    verify_csrf();

    $deleteId = (int)($_POST['user_id'] ?? 0);
    if ($deleteId <= 0) {
        flash('Invalid user id.');
        redirect('?page=admin');
    }

    $me = current_user();
    if ($me && (int)$me['id'] === $deleteId) {
        flash("You can't delete your own account while logged in.");
        redirect('?page=admin');
    }

    // Prevent deleting the last admin
    $stmt = db()->prepare("SELECT role FROM users WHERE id = :id");
    $stmt->execute([':id' => $deleteId]);
    $target = $stmt->fetch();

    if (!$target) {
        flash('User not found.');
        redirect('?page=admin');
    }

    if ($target['role'] === 'admin') {
        $stmt = db()->prepare("SELECT COUNT(*) AS c FROM users WHERE role='admin'");
        $stmt->execute();
        $adminCount = (int)$stmt->fetch()['c'];
        if ($adminCount <= 1) {
            flash("Can't delete the last admin account.");
            redirect('?page=admin');
        }
    }

    $stmt = db()->prepare("DELETE FROM users WHERE id = :id");
    $stmt->execute([':id' => $deleteId]);

    flash('User deleted.');
    redirect('?page=admin');
}

/* -----------------------------
   Pages (GET handlers)
------------------------------ */
function page_home(): void {
    $u = current_user();
    if ($u) redirect('?page=dashboard');
    redirect('?page=login');
}

function page_login(): void {
    render_page('Login', function() {
        ?>
        <div class="card">
            <form method="post" action="?page=login">
                <input type="hidden" name="csrf" value="<?= h(csrf_token()) ?>">
                <label>Email</label>
                <input name="email" type="email" required />

                <label>Password</label>
                <input name="password" type="password" required />

                <div style="margin-top:12px; display:flex; gap:10px; align-items:center;">
                    <button type="submit">Login</button>
                    <span class="muted">No account? <a href="?page=register">Register</a></span>
                </div>
            </form>
        </div>

        <div class="card">
            <h3>Default admin (created automatically if no admin exists)</h3>
            <p class="muted">
                Email: <code><?= h(DEFAULT_ADMIN_EMAIL) ?></code><br>
                Password: <code><?= h(DEFAULT_ADMIN_PASSWORD) ?></code>
            </p>
            <p class="muted">Change/remove this for real deployments.</p>
        </div>
        <?php
    });
}

function page_register(): void {
    render_page('Register', function() {
        ?>
        <div class="card">
            <form method="post" action="?page=register">
                <input type="hidden" name="csrf" value="<?= h(csrf_token()) ?>">

                <div class="row">
                    <div>
                        <label>Name</label>
                        <input name="name" required />
                    </div>
                    <div>
                        <label>Email</label>
                        <input name="email" type="email" required />
                    </div>
                </div>

                <label>Password</label>
                <input name="password" type="password" required />

                <div style="margin-top:12px; display:flex; gap:10px; align-items:center;">
                    <button type="submit">Create account</button>
                    <span class="muted">Already registered? <a href="?page=login">Login</a></span>
                </div>
            </form>
        </div>
        <?php
    });
}

/**
 * This is the "dashboard.php" page behavior.
 */
function page_dashboard(): void {
    require_login();
    $u = current_user();

    render_page('Dashboard', function() use ($u) {
        ?>
        <div class="card">
            <h2>Your profile</h2>
            <table>
                <tr><th>ID</th><td><?= h((string)$u['id']) ?></td></tr>
                <tr><th>Name</th><td><?= h($u['name']) ?></td></tr>
                <tr><th>Email</th><td><?= h($u['email']) ?></td></tr>
                <tr><th>Role</th><td><?= h($u['role']) ?></td></tr>
                <tr><th>Created</th><td><?= h($u['created_at']) ?></td></tr>
            </table>
        </div>

        <?php if ($u['role'] === 'admin'): ?>
            <div class="card">
                <h3>Admin shortcut</h3>
                <p><a href="?page=admin">Go to Admin Panel</a></p>
            </div>
        <?php endif; ?>
        <?php
    });
}

/**
 * This is the "admin.php" page behavior.
 */
function page_admin(): void {
    require_admin();

    $stmt = db()->query("SELECT id, email, name, role, created_at FROM users ORDER BY id ASC");
    $users = $stmt->fetchAll();

    $me = current_user();

    render_page('Admin Panel', function() use ($users, $me) {
        ?>
        <div class="card">
            <h2>All registered users</h2>
            <table>
                <thead>
                    <tr>
                        <th>ID</th><th>Email</th><th>Name</th><th>Role</th><th>Created</th><th>Actions</th>
                    </tr>
                </thead>
                <tbody>
                <?php foreach ($users as $u): ?>
                    <tr>
                        <td><?= h((string)$u['id']) ?></td>
                        <td><?= h($u['email']) ?></td>
                        <td><?= h($u['name']) ?></td>
                        <td><?= h($u['role']) ?></td>
                        <td><?= h($u['created_at']) ?></td>
                        <td>
                            <?php if ((int)$me['id'] === (int)$u['id']): ?>
                                <span class="muted">Current user</span>
                            <?php else: ?>
                                <form method="post" action="?page=delete_user" onsubmit="return confirm('Delete this user?');" style="margin:0;">
                                    <input type="hidden" name="csrf" value="<?= h(csrf_token()) ?>">
                                    <input type="hidden" name="user_id" value="<?= h((string)$u['id']) ?>">
                                    <button class="danger" type="submit">Delete</button>
                                </form>
                            <?php endif; ?>
                        </td>
                    </tr>
                <?php endforeach; ?>
                </tbody>
            </table>
        </div>
        <?php
    });
}

/* -----------------------------
   Router
------------------------------ */
$page = (string)($_GET['page'] ?? 'home');

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    switch ($page) {
        case 'register': handle_register(); break;
        case 'login': handle_login(); break;
        case 'delete_user': handle_delete_user(); break;
        default:
            http_response_code(405);
            render_page('Method Not Allowed', function() {
                echo "<p>Unsupported POST action.</p>";
            });
            break;
    }
    exit;
}

// GET routes
switch ($page) {
    case 'home': page_home(); break;
    case 'login': page_login(); break;
    case 'register': page_register(); break;
    case 'logout': handle_logout(); break;
    case 'dashboard': page_dashboard(); break;
    case 'admin': page_admin(); break;
    default:
        http_response_code(404);
        render_page('Not Found', function() {
            echo "<h2>404 Not Found</h2>";
            echo '<p><a href="?page=home">Go Home</a></p>';
        });
        break;
}