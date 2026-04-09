<?php
declare(strict_types=1);

/**
 * Single-file Company App
 * Pages:
 *  - signup, login, home, admin, employee, logout
 */

session_start();

/* ========= CONFIG ========= */
$DB_HOST = "127.0.0.1";
$DB_NAME = "company_app";
$DB_USER = "root";
$DB_PASS = ""; // Wamp default is often empty

/* ========= DB CONNECT ========= */
$dsn = "mysql:host={$DB_HOST};dbname={$DB_NAME};charset=utf8mb4";
try {
    $pdo = new PDO($dsn, $DB_USER, $DB_PASS, [
        PDO::ATTR_ERRMODE => PDO::ERRMODE_EXCEPTION,
        PDO::ATTR_DEFAULT_FETCH_MODE => PDO::FETCH_ASSOC,
    ]);
} catch (PDOException $e) {
    die("Database connection failed: " . htmlspecialchars($e->getMessage()));
}

/* ========= AUTH HELPERS ========= */
function is_logged_in(): bool {
    return isset($_SESSION["user"]);
}
function current_user(): ?array {
    return $_SESSION["user"] ?? null;
}
function require_login(): void {
    if (!is_logged_in()) {
        header("Location: index.php?page=login");
        exit;
    }
}
// Rule: job_title "Admin" => admin
function is_admin(): bool {
    return is_logged_in() && (strcasecmp($_SESSION["user"]["job_title"], "Admin") === 0);
}

/* ========= ROUTING ========= */
$page = strtolower((string)($_GET["page"] ?? (is_logged_in() ? "home" : "login")));

/* ========= LOGOUT (still within same file) ========= */
if ($page === "logout") {
    session_destroy();
    header("Location: index.php?page=login");
    exit;
}

/* ========= POST HANDLERS ========= */
$errors = [];
$success = "";

if ($_SERVER["REQUEST_METHOD"] === "POST") {
    $action = (string)($_POST["action"] ?? "");

    if ($action === "signup") {
        $username = trim((string)($_POST["username"] ?? ""));
        $password = (string)($_POST["password"] ?? "");
        $job_title = trim((string)($_POST["job_title"] ?? ""));

        if ($username === "" || $password === "" || $job_title === "") {
            $errors[] = "All fields are required.";
        } elseif (strlen($username) < 3) {
            $errors[] = "Username must be at least 3 characters.";
        } elseif (strlen($password) < 6) {
            $errors[] = "Password must be at least 6 characters.";
        }

        if (!$errors) {
            $stmt = $pdo->prepare("SELECT id FROM users WHERE username = ?");
            $stmt->execute([$username]);
            if ($stmt->fetch()) {
                $errors[] = "That username is already taken.";
            } else {
                $hash = password_hash($password, PASSWORD_DEFAULT);
                $stmt = $pdo->prepare("INSERT INTO users (username, password_hash, job_title) VALUES (?, ?, ?)");
                $stmt->execute([$username, $hash, $job_title]);

                $success = "Account created. You can log in now.";
                $page = "login"; // show login page after successful signup
            }
        }
    }

    if ($action === "login") {
        $username = trim((string)($_POST["username"] ?? ""));
        $password = (string)($_POST["password"] ?? "");

        if ($username === "" || $password === "") {
            $errors[] = "Username and password are required.";
        } else {
            $stmt = $pdo->prepare("SELECT id, username, password_hash, job_title FROM users WHERE username = ?");
            $stmt->execute([$username]);
            $user = $stmt->fetch();

            if (!$user || !password_verify($password, $user["password_hash"])) {
                $errors[] = "Invalid username or password.";
            } else {
                $_SESSION["user"] = [
                    "id" => (int)$user["id"],
                    "username" => $user["username"],
                    "job_title" => $user["job_title"],
                ];
                header("Location: index.php?page=home");
                exit;
            }
        }
    }
}

/* ========= ACCESS CONTROL ========= */
$protected = ["home", "admin", "employee"];
if (in_array($page, $protected, true)) {
    require_login();
}
if ($page === "admin" && !is_admin()) {
    header("Location: index.php?page=home");
    exit;
}

/* ========= DATA FOR PAGES ========= */
$user = current_user();
$allUsers = [];

if ($page === "admin") {
    $stmt = $pdo->query("SELECT id, username, job_title, created_at FROM users ORDER BY created_at DESC");
    $allUsers = $stmt->fetchAll();
}

// Announcements (static for now; still meets requirement)
$announcements = [
    "Welcome to the company portal!",
    "Reminder: Submit your weekly report by Friday 5pm.",
    "Next all-hands meeting: Monday at 10:00 AM.",
];

?>
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>Company App (Single File)</title>
  <style>
    body { font-family: Arial, sans-serif; margin: 30px; max-width: 900px; }
    nav a { margin-right: 12px; }
    .card { border: 1px solid #ddd; padding: 16px; border-radius: 8px; margin-top: 16px; }
    .error { background: #ffe9e9; border: 1px solid #ffb3b3; padding: 10px; border-radius: 6px; }
    .success { background: #e8fff0; border: 1px solid #86e0a3; padding: 10px; border-radius: 6px; }
    table { border-collapse: collapse; width: 100%; }
    th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }
    th { background: #f5f5f5; }
    input { padding: 8px; width: 100%; max-width: 360px; }
    button { padding: 10px 14px; cursor: pointer; }
    .muted { color: #666; }
  </style>
</head>
<body>

<h1>Company App</h1>

<nav>
  <?php if (is_logged_in()): ?>
    <a href="index.php?page=home">Home</a>
    <a href="index.php?page=employee">Employee</a>
    <?php if (is_admin()): ?>
      <a href="index.php?page=admin">Admin</a>
    <?php endif; ?>
    <a href="index.php?page=logout">Logout (<?php echo htmlspecialchars($user["username"]); ?>)</a>
  <?php else: ?>
    <a href="index.php?page=login">Login</a>
    <a href="index.php?page=signup">Signup</a>
  <?php endif; ?>
</nav>

<?php if ($errors): ?>
  <div class="error card">
    <strong>Please fix the following:</strong>
    <ul>
      <?php foreach ($errors as $e): ?>
        <li><?php echo htmlspecialchars($e); ?></li>
      <?php endforeach; ?>
    </ul>
  </div>
<?php endif; ?>

<?php if ($success): ?>
  <div class="success card">
    <?php echo htmlspecialchars($success); ?>
  </div>
<?php endif; ?>

<div class="card">
<?php if ($page === "signup"): ?>

  <h2>Signup</h2>
  <p class="muted">Create an account (use job title “Admin” for admin access).</p>

  <form method="post" action="index.php?page=signup">
    <input type="hidden" name="action" value="signup" />

    <p>
      <label>Username</label><br/>
      <input name="username" type="text" autocomplete="username" required />
    </p>

    <p>
      <label>Password</label><br/>
      <input name="password" type="password" autocomplete="new-password" required />
    </p>

    <p>
      <label>Job Title</label><br/>
      <input name="job_title" type="text" placeholder="Admin or Employee" required />
    </p>

    <button type="submit">Create account</button>
  </form>

<?php elseif ($page === "login"): ?>

  <h2>Login</h2>
  <form method="post" action="index.php?page=login">
    <input type="hidden" name="action" value="login" />

    <p>
      <label>Username</label><br/>
      <input name="username" type="text" autocomplete="username" required />
    </p>

    <p>
      <label>Password</label><br/>
      <input name="password" type="password" autocomplete="current-password" required />
    </p>

    <button type="submit">Login</button>
  </form>

<?php elseif ($page === "home"): ?>

  <h2>Home</h2>
  <p>
    Welcome, <strong><?php echo htmlspecialchars($user["username"]); ?></strong>.<br/>
    Job Title: <strong><?php echo htmlspecialchars($user["job_title"]); ?></strong>
  </p>

  <?php if (is_admin()): ?>
    <p>You are an admin. Go to the Admin page to see all users.</p>
  <?php else: ?>
    <p>You are an employee. Go to the Employee page to see announcements.</p>
  <?php endif; ?>

<?php elseif ($page === "admin"): ?>

  <h2>Admin: All Registered Users</h2>
  <p class="muted">Visible only if your job title is “Admin”.</p>

  <?php if (!$allUsers): ?>
    <p>No users found.</p>
  <?php else: ?>
    <table>
      <thead>
        <tr>
          <th>ID</th>
          <th>Username</th>
          <th>Job Title</th>
          <th>Created</th>
        </tr>
      </thead>
      <tbody>
      <?php foreach ($allUsers as $u): ?>
        <tr>
          <td><?php echo (int)$u["id"]; ?></td>
          <td><?php echo htmlspecialchars($u["username"]); ?></td>
          <td><?php echo htmlspecialchars($u["job_title"]); ?></td>
          <td><?php echo htmlspecialchars($u["created_at"]); ?></td>
        </tr>
      <?php endforeach; ?>
      </tbody>
    </table>
  <?php endif; ?>

<?php elseif ($page === "employee"): ?>

  <h2>Employee: Company Announcements</h2>
  <ul>
    <?php foreach ($announcements as $a): ?>
      <li><?php echo htmlspecialchars($a); ?></li>
    <?php endforeach; ?>
  </ul>

<?php else: ?>

  <h2>Page not found</h2>
  <p>Unknown page: <?php echo htmlspecialchars($page); ?></p>

<?php endif; ?>
</div>

</body>
</html>