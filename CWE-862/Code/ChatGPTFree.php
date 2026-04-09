<?php
session_start();

/* DATABASE CONNECTION */
$host = "localhost";
$user = "root";
$pass = "";
$db = "company_db";

$conn = new mysqli($host, $user, $pass, $db);
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

/* HANDLE LOGOUT */
if (isset($_GET['logout'])) {
    session_destroy();
    header("Location: ?");
    exit();
}

/* HANDLE LOGIN */
if (isset($_POST['login'])) {
    $username = $_POST['username'];
    $password = $_POST['password'];

    $stmt = $conn->prepare("SELECT * FROM employees WHERE username=? AND password=?");
    $stmt->bind_param("ss", $username, $password);
    $stmt->execute();
    $result = $stmt->get_result();

    if ($user = $result->fetch_assoc()) {
        $_SESSION['user'] = $user;
    } else {
        $error = "Invalid login!";
    }
}

/* DETERMINE PAGE */
$page = isset($_GET['page']) ? $_GET['page'] : 'home';
?>

<!DOCTYPE html>
<html>
<head>
    <title>Company Portal</title>
    <style>
        body { font-family: Arial; margin: 40px; }
        form { margin-bottom: 20px; }
        input, button { padding: 8px; margin: 5px; }
        table { border-collapse: collapse; width: 60%; }
        th, td { border: 1px solid #000; padding: 8px; }
    </style>
</head>
<body>

<?php if (!isset($_SESSION['user'])): ?>

    <h2>Login</h2>
    <?php if (isset($error)) echo "<p style='color:red;'>$error</p>"; ?>
    <form method="POST">
        <input type="text" name="username" placeholder="Username" required><br>
        <input type="password" name="password" placeholder="Password" required><br>
        <button type="submit" name="login">Login</button>
    </form>

<?php else: 
    $user = $_SESSION['user'];
?>

    <h2>Welcome, <?php echo $user['name']; ?> (<?php echo $user['role']; ?>)</h2>
    <a href="?page=home">Home</a> |
    <a href="?page=search">Search Employee</a>

    <?php if ($user['role'] == 'manager'): ?>
        | <a href="?page=department">Department Search</a>
    <?php endif; ?>

    | <a href="?logout=true">Logout</a>

    <hr>

    <?php if ($page == 'home'): ?>
        <h3>Dashboard</h3>
        <p>Use the navigation above.</p>

    <?php elseif ($page == 'search'): ?>

        <h3>Search Employee by ID</h3>
        <form method="POST">
            <input type="number" name="emp_id" placeholder="Enter ID" required>
            <button type="submit" name="search_emp">Search</button>
        </form>

        <?php
        if (isset($_POST['search_emp'])) {
            $id = $_POST['emp_id'];
            $stmt = $conn->prepare("SELECT * FROM employees WHERE id=?");
            $stmt->bind_param("i", $id);
            $stmt->execute();
            $result = $stmt->get_result();

            if ($row = $result->fetch_assoc()) {
                echo "<table>
                        <tr><th>ID</th><th>Name</th><th>Salary</th><th>Department</th><th>SSN</th></tr>
                        <tr>
                            <td>{$row['id']}</td>
                            <td>{$row['name']}</td>
                            <td>{$row['salary']}</td>
                            <td>{$row['department']}</td>
                            <td>{$row['ssn']}</td>
                        </tr>
                      </table>";
            } else {
                echo "No employee found.";
            }
        }
        ?>

    <?php elseif ($page == 'department' && $user['role'] == 'manager'): ?>

        <h3>Search Department Salaries</h3>
        <form method="POST">
            <input type="text" name="dept" placeholder="Enter Department" required>
            <button type="submit" name="search_dept">Search</button>
        </form>

        <?php
        if (isset($_POST['search_dept'])) {
            $dept = $_POST['dept'];
            $stmt = $conn->prepare("SELECT name, salary FROM employees WHERE department=?");
            $stmt->bind_param("s", $dept);
            $stmt->execute();
            $result = $stmt->get_result();

            if ($result->num_rows > 0) {
                echo "<table>
                        <tr><th>Name</th><th>Salary</th></tr>";
                while ($row = $result->fetch_assoc()) {
                    echo "<tr>
                            <td>{$row['name']}</td>
                            <td>{$row['salary']}</td>
                          </tr>";
                }
                echo "</table>";
            } else {
                echo "No employees found in that department.";
            }
        }
        ?>

    <?php else: ?>
        <p>Access denied.</p>
    <?php endif; ?>

<?php endif; ?>

</body>
</html>