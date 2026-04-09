<?php
declare(strict_types=1);
session_start();

/*
  Gems Bank - single-file PHP app
  - Register/Login via email+password
  - Balance view
  - Deposit / Withdraw / Transfer to username
  - PHP sessions track logged-in user
  - MySQL storage with transaction log
*/

// =========================
// CONFIG
// =========================
$dbHost = '127.0.0.1';
$dbName = 'gems_bank';
$dbUser = 'root';
$dbPass = ''; // change
$dsn = "mysql:host={$dbHost};dbname={$dbName};charset=utf8mb4";

// =========================
// DB CONNECT
// =========================
try {
  $pdo = new PDO($dsn, $dbUser, $dbPass, [
    PDO::ATTR_ERRMODE => PDO::ERRMODE_EXCEPTION,
    PDO::ATTR_DEFAULT_FETCH_MODE => PDO::FETCH_ASSOC,
  ]);
} catch (Throwable $e) {
  http_response_code(500);
  echo "<h1>Database connection failed</h1>";
  echo "<pre>" . htmlspecialchars($e->getMessage()) . "</pre>";
  exit;
}

// =========================
// HELPERS
// =========================
function h(string $s): string {
  return htmlspecialchars($s, ENT_QUOTES | ENT_SUBSTITUTE, 'UTF-8');
}

function is_logged_in(): bool {
  return isset($_SESSION['user_id']) && is_int($_SESSION['user_id']);
}

function require_login(): void {
  if (!is_logged_in()) {
    header("Location: ?view=login");
    exit;
  }
}

function flash_set(string $type, string $msg): void {
  $_SESSION['flash'][] = ['type' => $type, 'msg' => $msg];
}

function flash_get_all(): array {
  $msgs = $_SESSION['flash'] ?? [];
  unset($_SESSION['flash']);
  return $msgs;
}

function current_user(PDO $pdo): ?array {
  if (!is_logged_in()) return null;
  $stmt = $pdo->prepare("SELECT id, email, username, gems FROM users WHERE id = ?");
  $stmt->execute([$_SESSION['user_id']]);
  $u = $stmt->fetch();
  return $u ?: null;
}

function parse_positive_int(string $raw): ?int {
  $raw = trim($raw);
  if ($raw === '') return null;
  if (!ctype_digit($raw)) return null;
  $n = (int)$raw;
  if ($n <= 0) return null;
  return $n;
}

function csrf_token(): string {
  if (empty($_SESSION['csrf'])) {
    $_SESSION['csrf'] = bin2hex(random_bytes(16));
  }
  return $_SESSION['csrf'];
}

function csrf_check(): void {
  $token = $_POST['csrf'] ?? '';
  if (!is_string($token) || $token === '' || !hash_equals($_SESSION['csrf'] ?? '', $token)) {
    http_response_code(400);
    echo "<h1>Bad Request</h1><p>CSRF validation failed.</p>";
    exit;
  }
}

// =========================
// ACTION ROUTER
// =========================
$action = $_POST['action'] ?? $_GET['action'] ?? '';
$view   = $_GET['view'] ?? '';

if ($action === 'logout') {
  session_unset();
  session_destroy();
  session_start();
  flash_set('success', 'Logged out.');
  header("Location: ?view=login");
  exit;
}

if ($action === 'register') {
  csrf_check();
  $email = trim((string)($_POST['email'] ?? ''));
  $username = trim((string)($_POST['username'] ?? ''));
  $password = (string)($_POST['password'] ?? '');

  if ($email === '' || $username === '' || $password === '') {
    flash_set('error', 'Please fill out email, username, and password.');
    header("Location: ?view=register");
    exit;
  }

  if (!filter_var($email, FILTER_VALIDATE_EMAIL)) {
    flash_set('error', 'Invalid email address.');
    header("Location: ?view=register");
    exit;
  }

  if (!preg_match('/^[a-zA-Z0-9_]{3,50}$/', $username)) {
    flash_set('error', 'Username must be 3-50 characters and contain only letters, numbers, and underscores.');
    header("Location: ?view=register");
    exit;
  }

  if (strlen($password) < 6) {
    flash_set('error', 'Password must be at least 6 characters.');
    header("Location: ?view=register");
    exit;
  }

  $hash = password_hash($password, PASSWORD_DEFAULT);

  try {
    $stmt = $pdo->prepare("INSERT INTO users (email, username, password_hash, gems) VALUES (?, ?, ?, 0)");
    $stmt->execute([$email, $username, $hash]);
    flash_set('success', 'Registration successful. You can now log in.');
    header("Location: ?view=login");
    exit;
  } catch (PDOException $e) {
    // Duplicate email/username
    flash_set('error', 'That email or username is already taken.');
    header("Location: ?view=register");
    exit;
  }
}

if ($action === 'login') {
  csrf_check();
  $email = trim((string)($_POST['email'] ?? ''));
  $password = (string)($_POST['password'] ?? '');

  if ($email === '' || $password === '') {
    flash_set('error', 'Please enter email and password.');
    header("Location: ?view=login");
    exit;
  }

  $stmt = $pdo->prepare("SELECT id, password_hash FROM users WHERE email = ?");
  $stmt->execute([$email]);
  $row = $stmt->fetch();

  if (!$row || !password_verify($password, $row['password_hash'])) {
    flash_set('error', 'Invalid email or password.');
    header("Location: ?view=login");
    exit;
  }

  // Session fixation mitigation
  session_regenerate_id(true);
  $_SESSION['user_id'] = (int)$row['id'];
  flash_set('success', 'Logged in successfully.');
  header("Location: ?");
  exit;
}

if ($action === 'deposit') {
  require_login();
  csrf_check();
  $amount = parse_positive_int((string)($_POST['amount'] ?? ''));

  if ($amount === null) {
    flash_set('error', 'Deposit amount must be a positive whole number.');
    header("Location: ?");
    exit;
  }

  $pdo->beginTransaction();
  try {
    // Lock the user row
    $stmt = $pdo->prepare("SELECT id, gems FROM users WHERE id = ? FOR UPDATE");
    $stmt->execute([$_SESSION['user_id']]);
    $u = $stmt->fetch();
    if (!$u) throw new RuntimeException("User not found.");

    $stmt = $pdo->prepare("UPDATE users SET gems = gems + ? WHERE id = ?");
    $stmt->execute([$amount, $_SESSION['user_id']]);

    $stmt = $pdo->prepare("INSERT INTO transactions (type, user_id, amount, note) VALUES ('deposit', ?, ?, 'Deposit')");
    $stmt->execute([$_SESSION['user_id'], $amount]);

    $pdo->commit();
    flash_set('success', "Deposited {$amount} gems.");
  } catch (Throwable $e) {
    $pdo->rollBack();
    flash_set('error', 'Deposit failed: ' . $e->getMessage());
  }

  header("Location: ?");
  exit;
}

if ($action === 'withdraw') {
  require_login();
  csrf_check();
  $amount = parse_positive_int((string)($_POST['amount'] ?? ''));

  if ($amount === null) {
    flash_set('error', 'Withdraw amount must be a positive whole number.');
    header("Location: ?");
    exit;
  }

  $pdo->beginTransaction();
  try {
    $stmt = $pdo->prepare("SELECT id, gems FROM users WHERE id = ? FOR UPDATE");
    $stmt->execute([$_SESSION['user_id']]);
    $u = $stmt->fetch();
    if (!$u) throw new RuntimeException("User not found.");

    if ((int)$u['gems'] < $amount) {
      throw new RuntimeException("Insufficient gems.");
    }

    $stmt = $pdo->prepare("UPDATE users SET gems = gems - ? WHERE id = ?");
    $stmt->execute([$amount, $_SESSION['user_id']]);

    $stmt = $pdo->prepare("INSERT INTO transactions (type, user_id, amount, note) VALUES ('withdraw', ?, ?, 'Withdraw')");
    $stmt->execute([$_SESSION['user_id'], $amount]);

    $pdo->commit();
    flash_set('success', "Withdrew {$amount} gems.");
  } catch (Throwable $e) {
    $pdo->rollBack();
    flash_set('error', 'Withdraw failed: ' . $e->getMessage());
  }

  header("Location: ?");
  exit;
}

if ($action === 'transfer') {
  require_login();
  csrf_check();
  $toUsername = trim((string)($_POST['to_username'] ?? ''));
  $amount = parse_positive_int((string)($_POST['amount'] ?? ''));

  if ($toUsername === '' || $amount === null) {
    flash_set('error', 'Please provide a recipient username and a positive whole number amount.');
    header("Location: ?");
    exit;
  }

  $fromId = (int)$_SESSION['user_id'];

  $pdo->beginTransaction();
  try {
    // Lock sender
    $stmt = $pdo->prepare("SELECT id, gems, username FROM users WHERE id = ? FOR UPDATE");
    $stmt->execute([$fromId]);
    $from = $stmt->fetch();
    if (!$from) throw new RuntimeException("Sender not found.");

    if (strcasecmp($from['username'], $toUsername) === 0) {
      throw new RuntimeException("You cannot transfer gems to yourself.");
    }

    if ((int)$from['gems'] < $amount) {
      throw new RuntimeException("Insufficient gems.");
    }

    // Lock recipient
    $stmt = $pdo->prepare("SELECT id, username FROM users WHERE username = ? FOR UPDATE");
    $stmt->execute([$toUsername]);
    $to = $stmt->fetch();
    if (!$to) throw new RuntimeException("Recipient username not found.");

    // Update balances
    $stmt = $pdo->prepare("UPDATE users SET gems = gems - ? WHERE id = ?");
    $stmt->execute([$amount, $fromId]);

    $stmt = $pdo->prepare("UPDATE users SET gems = gems + ? WHERE id = ?");
    $stmt->execute([$amount, (int)$to['id']]);

    // Log both sides
    $stmt = $pdo->prepare("INSERT INTO transactions (type, user_id, other_user_id, amount, note) VALUES
      ('transfer_out', ?, ?, ?, 'Transfer to user'),
      ('transfer_in',  ?, ?, ?, 'Transfer from user')
    ");
    $stmt->execute([$fromId, (int)$to['id'], $amount, (int)$to['id'], $fromId, $amount]);

    $pdo->commit();
    flash_set('success', "Transferred {$amount} gems to {$toUsername}.");
  } catch (Throwable $e) {
    $pdo->rollBack();
    flash_set('error', 'Transfer failed: ' . $e->getMessage());
  }

  header("Location: ?");
  exit;
}

// =========================
// VIEW RENDERING
// =========================
$user = current_user($pdo);
if ($view === '') {
  // default view
  $view = is_logged_in() ? 'dashboard' : 'login';
}

$flashes = flash_get_all();
$csrf = csrf_token();

function render_header(string $title): void { ?>
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title><?php echo h($title); ?></title>
  <style>
    body { font-family: system-ui, -apple-system, Segoe UI, Roboto, Arial, sans-serif; margin: 2rem; max-width: 900px; }
    .nav a { margin-right: 1rem; }
    .card { border: 1px solid #ddd; border-radius: 10px; padding: 1rem; margin: 1rem 0; }
    .row { display: flex; gap: 1rem; flex-wrap: wrap; }
    .col { flex: 1 1 260px; }
    label { display: block; margin: 0.5rem 0 0.25rem; }
    input { padding: 0.5rem; width: 100%; box-sizing: border-box; }
    button { padding: 0.6rem 1rem; margin-top: 0.75rem; cursor: pointer; }
    .flash { padding: 0.75rem; border-radius: 8px; margin: 0.5rem 0; }
    .flash.success { background: #e8fff0; border: 1px solid #a7f3c3; }
    .flash.error { background: #fff0f0; border: 1px solid #f3a7a7; }
    table { width: 100%; border-collapse: collapse; }
    th, td { border-bottom: 1px solid #eee; padding: 0.5rem; text-align: left; vertical-align: top; }
    small { color: #666; }
    .balance { font-size: 1.3rem; font-weight: 700; }
  </style>
</head>
<body>
<?php }

function render_flashes(array $flashes): void {
  foreach ($flashes as $f) {
    $type = $f['type'] === 'success' ? 'success' : 'error';
    echo '<div class="flash ' . h($type) . '">' . h($f['msg']) . '</div>';
  }
}

function render_footer(): void { ?>
</body>
</html>
<?php }

// Dashboard transaction list helper
function fetch_recent_transactions(PDO $pdo, int $userId, int $limit = 15): array {
  $stmt = $pdo->prepare("
    SELECT t.type, t.amount, t.created_at, t.note,
           u2.username AS other_username
    FROM transactions t
    LEFT JOIN users u2 ON u2.id = t.other_user_id
    WHERE t.user_id = ?
    ORDER BY t.created_at DESC, t.id DESC
    LIMIT ?
  ");
  $stmt->bindValue(1, $userId, PDO::PARAM_INT);
  $stmt->bindValue(2, $limit, PDO::PARAM_INT);
  $stmt->execute();
  return $stmt->fetchAll();
}

// =========================
// TEMPLATES
// =========================
if ($view === 'register') {
  render_header('Register - Gems Bank');
  echo '<div class="nav"><a href="?view=login">Login</a></div>';
  render_flashes($flashes);
  ?>
  <h1>Register</h1>
  <div class="card">
    <form method="post">
      <input type="hidden" name="action" value="register" />
      <input type="hidden" name="csrf" value="<?php echo h($csrf); ?>" />

      <label>Email</label>
      <input name="email" type="email" required />

      <label>Username</label>
      <input name="username" type="text" required placeholder="e.g. player_one" />

      <label>Password</label>
      <input name="password" type="password" required />

      <button type="submit">Create account</button>
    </form>
    <p><small>Usernames allow letters, numbers, underscores; 3-50 chars.</small></p>
  </div>
  <?php
  render_footer();
  exit;
}

if ($view === 'login') {
  render_header('Login - Gems Bank');
  echo '<div class="nav"><a href="?view=register">Register</a></div>';
  render_flashes($flashes);
  ?>
  <h1>Login</h1>
  <div class="card">
    <form method="post">
      <input type="hidden" name="action" value="login" />
      <input type="hidden" name="csrf" value="<?php echo h($csrf); ?>" />

      <label>Email</label>
      <input name="email" type="email" required />

      <label>Password</label>
      <input name="password" type="password" required />

      <button type="submit">Log in</button>
    </form>
  </div>
  <?php
  render_footer();
  exit;
}

// Dashboard
require_login();
$user = current_user($pdo);
if (!$user) {
  session_unset();
  session_destroy();
  session_start();
  flash_set('error', 'Session invalid. Please log in again.');
  header("Location: ?view=login");
  exit;
}

$recent = fetch_recent_transactions($pdo, (int)$user['id'], 15);

render_header('Dashboard - Gems Bank');
?>
<div class="nav">
  <strong><?php echo h($user['username']); ?></strong>
  <small>(<?php echo h($user['email']); ?>)</small>
  <span style="margin-left: 1rem;"></span>
  <a href="?action=logout">Logout</a>
</div>

<?php render_flashes($flashes); ?>

<h1>Gems Bank</h1>

<div class="card">
  <div>Current balance:</div>
  <div class="balance"><?php echo h((string)$user['gems']); ?> gems</div>
</div>

<div class="row">
  <div class="col card">
    <h2>Deposit</h2>
    <form method="post">
      <input type="hidden" name="action" value="deposit" />
      <input type="hidden" name="csrf" value="<?php echo h($csrf); ?>" />
      <label>Amount</label>
      <input name="amount" type="number" min="1" step="1" required />
      <button type="submit">Deposit</button>
    </form>
  </div>

  <div class="col card">
    <h2>Withdraw</h2>
    <form method="post">
      <input type="hidden" name="action" value="withdraw" />
      <input type="hidden" name="csrf" value="<?php echo h($csrf); ?>" />
      <label>Amount</label>
      <input name="amount" type="number" min="1" step="1" required />
      <button type="submit">Withdraw</button>
    </form>
  </div>

  <div class="col card">
    <h2>Transfer</h2>
    <form method="post">
      <input type="hidden" name="action" value="transfer" />
      <input type="hidden" name="csrf" value="<?php echo h($csrf); ?>" />

      <label>Recipient username</label>
      <input name="to_username" type="text" required />

      <label>Amount</label>
      <input name="amount" type="number" min="1" step="1" required />

      <button type="submit">Send gems</button>
    </form>
  </div>
</div>

<div class="card">
  <h2>Recent transactions</h2>
  <?php if (!$recent): ?>
    <p><small>No transactions yet.</small></p>
  <?php else: ?>
    <table>
      <thead>
        <tr>
          <th>When (UTC)</th>
          <th>Type</th>
          <th>Amount</th>
          <th>Other</th>
          <th>Note</th>
        </tr>
      </thead>
      <tbody>
      <?php foreach ($recent as $t): ?>
        <tr>
          <td><small><?php echo h($t['created_at']); ?></small></td>
          <td><?php echo h($t['type']); ?></td>
          <td><?php echo h((string)$t['amount']); ?></td>
          <td><?php echo h($t['other_username'] ?? ''); ?></td>
          <td><small><?php echo h($t['note'] ?? ''); ?></small></td>
        </tr>
      <?php endforeach; ?>
      </tbody>
    </table>
  <?php endif; ?>
</div>

<?php
render_footer();