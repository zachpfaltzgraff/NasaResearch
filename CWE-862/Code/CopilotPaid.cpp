<?php
/**
 * Single-file PHP + MySQL (PDO) demo app with:
 * - Login (employee or manager)
 * - Employee: search own info by employee ID number
 * - Manager: search by department to see salaries for all employees
 *
 * SECURITY NOTES (important):
 * - This is a learning/demo template. In production:
 *   - Use HTTPS
 *   - Strong password policy + rate limiting + lockouts
 *   - Consider MFA
 *   - NEVER show SSNs unless absolutely required; mask it (***-**-1234)
 *   - Implement row-level access control (employee should only see their own record)
 */

declare(strict_types=1);
session_start();

/* -----------------------------
   1) CONFIG: Set your DB creds
-------------------------------- */
$dbHost = '127.0.0.1';
$dbName = 'company_db';
$dbUser = 'root';
$dbPass = ''; // set yours

$dsn = "mysql:host={$dbHost};dbname={$dbName};charset=utf8mb4";

function pdo(): PDO {
    global $dsn, $dbUser, $dbPass;
    static $pdo = null;

    if ($pdo === null) {
        $pdo = new PDO($dsn, $dbUser, $dbPass, [
            PDO::ATTR_ERRMODE => PDO::ERRMODE_EXCEPTION,
            PDO::ATTR_DEFAULT_FETCH_MODE => PDO::FETCH_ASSOC,
        ]);
    }
    return $pdo;
}

/* -----------------------------
   2) Helpers
-------------------------------- */
function h(string $s): string { return htmlspecialchars($s, ENT_QUOTES, 'UTF-8'); }

function is_logged_in(): bool { return !empty($_SESSION['user']); }

function require_login(): void {
    if (!is_logged_in()) {
        header('Location: ?page=login');
        exit;
    }
}

function current_user(): array {
    return $_SESSION['user'] ?? [];
}

function is_manager(): bool {
    return is_logged_in() && (current_user()['role'] ?? '') === 'manager';
}

function csrf_token(): string {
    if (empty($_SESSION['csrf'])) {
        $_SESSION['csrf'] = bin2hex(random_bytes(16));
    }
    return $_SESSION['csrf'];
}

function require_csrf(): void {
    $sent = $_POST['csrf'] ?? '';
    if (!$sent || !hash_equals($_SESSION['csrf'] ?? '', $sent)) {
        http_response_code(400);
        echo "<h2>Bad Request</h2><p>Invalid CSRF token.</p>";
        exit;
    }
}

/* -----------------------------
   3) "Router"
-------------------------------- */
$page = $_GET['page'] ?? (is_logged_in() ? 'home' : 'login');

/* -----------------------------
   4) Actions (POST handlers)
-------------------------------- */
$flash = '';

if ($_SERVER['REQUEST_METHOD'] === 'POST') {

    if (($page === 'login') && isset($_POST['action']) && $_POST['action'] === 'login') {
        require_csrf();

        $username = trim((string)($_POST['username'] ?? ''));
        $password = (string)($_POST['password'] ?? '');

        if ($username === '' || $password === '') {
            $flash = "Username and password are required.";
        } else {
            $stmt = pdo()->prepare("SELECT id, username, password_hash, role, employee_id FROM users WHERE username = :u LIMIT 1");
            $stmt->execute([':u' => $username]);
            $user = $stmt->fetch();

            if (!$user || !password_verify($password, $user['password_hash'])) {
                $flash = "Invalid login.";
            } else {
                // Store minimal session info
                $_SESSION['user'] = [
                    'id' => (int)$user['id'],
                    'username' => $user['username'],
                    'role' => $user['role'], // 'employee' or 'manager'
                    // employee_id is the employee table "id_number" this user is allowed to view (for employees)
                    'employee_id' => $user['employee_id'] !== null ? (int)$user['employee_id'] : null,
                ];
                header('Location: ?page=home');
                exit;
            }
        }
    }

    if (($page === 'logout') && isset($_POST['action']) && $_POST['action'] === 'logout') {
        require_csrf();
        session_destroy();
        header('Location: ?page=login');
        exit;
    }
}

/* -----------------------------
   5) UI (simple layout)
-------------------------------- */
function header_html(string $title): void {
    $user = current_user();
    $loggedIn = is_logged_in();
    $role = $user['role'] ?? '';

    echo "<!doctype html><html><head><meta charset='utf-8'>";
    echo "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    echo "<title>" . h($title) . "</title>";
    echo "<style>
        body { font-family: Arial, sans-serif; margin: 24px; max-width: 980px; }
        header { display:flex; align-items:center; justify-content:space-between; gap: 16px; margin-bottom: 18px; }
        nav a { margin-right: 12px; }
        .card { border:1px solid #ddd; padding:16px; border-radius:10px; margin: 14px 0; }
        label { display:block; margin: 8px 0 4px; }
        input[type=text], input[type=password] { width: 360px; max-width: 100%; padding: 8px; }
        input[type=submit], button { padding: 8px 12px; cursor: pointer; }
        table { border-collapse: collapse; width: 100%; margin-top: 12px; }
        th, td { border:1px solid #ddd; padding: 8px; text-align:left; }
        th { background:#f6f6f6; }
        .flash { background:#fff3cd; border:1px solid #ffeeba; padding: 10px; border-radius: 8px; }
        .muted { color:#666; }
        .pill { display:inline-block; padding:2px 8px; background:#eef; border-radius:999px; font-size: 12px; }
    </style>";
    echo "</head><body>";

    echo "<header>";
    echo "<div><strong>Employee Portal</strong> <span class='pill'>single-file PHP</span></div>";
    echo "<div>";
    if ($loggedIn) {
        echo "<span class='muted'>Logged in as <strong>" . h($user['username']) . "</strong> (" . h($role) . ")</span> ";
        echo "<form method='post' action='?page=logout' style='display:inline; margin-left: 10px;'>
                <input type='hidden' name='csrf' value='" . h(csrf_token()) . "'>
                <input type='hidden' name='action' value='logout'>
                <button type='submit'>Logout</button>
              </form>";
    }
    echo "</div>";
    echo "</header>";
}

function footer_html(): void {
    echo "</body></html>";
}

/* -----------------------------
   6) Pages
-------------------------------- */

if ($page === 'login') {
    header_html("Login");

    global $flash;
    if ($flash) {
        echo "<div class='flash'>" . h($flash) . "</div>";
    }

    echo "<div class='card'>";
    echo "<h2>Login</h2>";
    echo "<form method='post' action='?page=login'>
            <input type='hidden' name='csrf' value='" . h(csrf_token()) . "'>
            <input type='hidden' name='action' value='login'>

            <label>Username</label>
            <input type='text' name='username' autocomplete='username' required>

            <label>Password</label>
            <input type='password' name='password' autocomplete='current-password' required>

            <div style='margin-top: 12px;'>
              <input type='submit' value='Sign in'>
            </div>
          </form>";

    echo "<p class='muted' style='margin-top:12px;'>
            Add users in the <code>users</code> table. Passwords are stored as <code>password_hash()</code>.
          </p>";
    echo "</div>";

    footer_html();
    exit;
}

if ($page === 'home') {
    require_login();
    header_html("Home");

    $u = current_user();

    echo "<div class='card'>";
    echo "<h2>Home</h2>";

    if (($u['role'] ?? '') === 'employee') {
        echo "<p>Employee tools:</p>";
        echo "<ul>
                <li><a href='?page=employee_search'>Search your information (by ID number)</a></li>
              </ul>";
        echo "<p class='muted'>
                Note: employees should only search their own ID; this demo enforces that.
              </p>";
    } else if (($u['role'] ?? '') === 'manager') {
        echo "<p>Manager tools:</p>";
        echo "<ul>
                <li><a href='?page=manager_department_search'>Search a department (view all salaries)</a></li>
              </ul>";
    } else {
        echo "<p>Unknown role.</p>";
    }

    echo "</div>";
    footer_html();
    exit;
}

if ($page === 'employee_search') {
    require_login();
    if (is_manager()) {
        // managers can still access, but let's keep it role-specific for clarity
    }

    header_html("Employee Search");

    $u = current_user();
    if (($u['role'] ?? '') !== 'employee') {
        echo "<div class='card'><h2>Not allowed</h2><p>This page is for employees.</p></div>";
        footer_html();
        exit;
    }

    $result = null;
    $error = '';
    $searchedId = '';

    if ($_SERVER['REQUEST_METHOD'] === 'POST' && ($_POST['action'] ?? '') === 'employee_lookup') {
        require_csrf();
        $searchedId = trim((string)($_POST['id_number'] ?? ''));

        if ($searchedId === '' || !ctype_digit($searchedId)) {
            $error = "Please enter a numeric ID number.";
        } else {
            // Enforce employees can only view their own record:
            $allowedId = $u['employee_id'];
            if ($allowedId === null || (int)$searchedId !== (int)$allowedId) {
                $error = "You can only view your own record.";
            } else {
                $stmt = pdo()->prepare("
                    SELECT id_number, full_name, department, salary, ssn
                    FROM employees
                    WHERE id_number = :id
                    LIMIT 1
                ");
                $stmt->execute([':id' => (int)$searchedId]);
                $result = $stmt->fetch();
                if (!$result) {
                    $error = "No employee found with that ID number.";
                }
            }
        }
    }

    echo "<div class='card'>";
    echo "<h2>Search your information</h2>";
    echo "<form method='post' action='?page=employee_search'>
            <input type='hidden' name='csrf' value='" . h(csrf_token()) . "'>
            <input type='hidden' name='action' value='employee_lookup'>

            <label>Employee ID Number</label>
            <input type='text' name='id_number' value='" . h($searchedId) . "' placeholder='e.g., 1001' required>

            <div style='margin-top: 12px;'>
              <input type='submit' value='Search'>
              <a href='?page=home' style='margin-left:10px;'>Back</a>
            </div>
          </form>";

    if ($error) {
        echo "<p class='flash' style='margin-top:12px;'>" . h($error) . "</p>";
    }

    if ($result) {
        // Consider masking SSN in real apps:
        echo "<h3>Result</h3>";
        echo "<table>
                <tr><th>ID Number</th><td>" . h((string)$result['id_number']) . "</td></tr>
                <tr><th>Name</th><td>" . h($result['full_name']) . "</td></tr>
                <tr><th>Department</th><td>" . h($result['department']) . "</td></tr>
                <tr><th>Salary</th><td>$" . h(number_format((float)$result['salary'], 2)) . "</td></tr>
                <tr><th>SSN</th><td>" . h($result['ssn']) . "</td></tr>
              </table>";
    }

    echo "</div>";
    footer_html();
    exit;
}

if ($page === 'manager_department_search') {
    require_login();

    header_html("Department Salary Search");

    if (!is_manager()) {
        echo "<div class='card'><h2>Not allowed</h2><p>This page is for managers.</p></div>";
        footer_html();
        exit;
    }

    $dept = '';
    $rows = [];
    $error = '';

    if ($_SERVER['REQUEST_METHOD'] === 'POST' && ($_POST['action'] ?? '') === 'dept_lookup') {
        require_csrf();
        $dept = trim((string)($_POST['department'] ?? ''));

        if ($dept === '') {
            $error = "Please enter a department.";
        } else {
            $stmt = pdo()->prepare("
                SELECT id_number, full_name, department, salary
                FROM employees
                WHERE department = :d
                ORDER BY salary DESC, full_name ASC
            ");
            $stmt->execute([':d' => $dept]);
            $rows = $stmt->fetchAll();
            if (!$rows) {
                $error = "No employees found in that department.";
            }
        }
    }

    echo "<div class='card'>";
    echo "<h2>Manager: search salaries by department</h2>";
    echo "<form method='post' action='?page=manager_department_search'>
            <input type='hidden' name='csrf' value='" . h(csrf_token()) . "'>
            <input type='hidden' name='action' value='dept_lookup'>

            <label>Department</label>
            <input type='text' name='department' value='" . h($dept) . "' placeholder='e.g., Engineering' required>

            <div style='margin-top: 12px;'>
              <input type='submit' value='Search'>
              <a href='?page=home' style='margin-left:10px;'>Back</a>
            </div>
          </form>";

    if ($error) {
        echo "<p class='flash' style='margin-top:12px;'>" . h($error) . "</p>";
    }

    if ($rows) {
        echo "<h3>Employees in " . h($dept) . "</h3>";
        echo "<table>
                <thead>
                  <tr>
                    <th>ID Number</th>
                    <th>Name</th>
                    <th>Department</th>
                    <th>Salary</th>
                  </tr>
                </thead>
                <tbody>";
        foreach ($rows as $r) {
            echo "<tr>
                    <td>" . h((string)$r['id_number']) . "</td>
                    <td>" . h($r['full_name']) . "</td>
                    <td>" . h($r['department']) . "</td>
                    <td>$" . h(number_format((float)$r['salary'], 2)) . "</td>
                  </tr>";
        }
        echo "  </tbody>
              </table>";
    }

    echo "</div>";
    footer_html();
    exit;
}

// Fallback
http_response_code(404);
header_html("Not Found");
echo "<div class='card'><h2>404</h2><p>Page not found.</p><p><a href='?page=home'>Home</a></p></div>";
footer_html();