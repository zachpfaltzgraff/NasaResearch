<?php
include("db.php");

$page = isset($_GET['page']) ? $_GET['page'] : 'login';

// LOGOUT
if ($page == "logout") {
    session_destroy();
    header("Location: index.php");
}

// SIGNUP
if ($page == "signup_action") {
    $user = $_POST['username'];
    $pass = password_hash($_POST['password'], PASSWORD_DEFAULT);
    $job = $_POST['job'];

    $sql = "INSERT INTO users (username, password, job_title)
            VALUES ('$user', '$pass', '$job')";
    
    if ($conn->query($sql)) {
        echo "Signup successful! <a href='index.php'>Login</a>";
    } else {
        echo "Error: " . $conn->error;
    }
}

// LOGIN
if ($page == "login_action") {
    $user = $_POST['username'];
    $pass = $_POST['password'];

    $sql = "SELECT * FROM users WHERE username='$user'";
    $result = $conn->query($sql);

    if ($result->num_rows > 0) {
        $row = $result->fetch_assoc();

        if (password_verify($pass, $row['password'])) {
            $_SESSION['user'] = $row['username'];
            $_SESSION['job'] = $row['job_title'];

            header("Location: index.php?page=home");
        } else {
            echo "Invalid password";
        }
    } else {
        echo "User not found";
    }
}
?>

<!DOCTYPE html>
<html>
<head>
    <title>Company App</title>
</head>
<body>

<?php
// ROUTING
if (!isset($_SESSION['user'])) {

    if ($page == "signup") {
?>
        <h2>Signup</h2>
        <form method="POST" action="index.php?page=signup_action">
            Username: <input type="text" name="username"><br>
            Password: <input type="password" name="password"><br>
            Job Title: <input type="text" name="job"><br>
            <input type="submit" value="Signup">
        </form>
        <a href="index.php">Login</a>

<?php
    } else {
?>
        <h2>Login</h2>
        <form method="POST" action="index.php?page=login_action">
            Username: <input type="text" name="username"><br>
            Password: <input type="password" name="password"><br>
            <input type="submit" value="Login">
        </form>
        <a href="index.php?page=signup">Signup</a>

<?php
    }

} else {

    // HOME PAGE
    if ($page == "home") {
?>
        <h2>Welcome, <?php echo $_SESSION['user']; ?></h2>
        <p>Job Title: <?php echo $_SESSION['job']; ?></p>

        <a href="index.php?page=admin">Admin Page</a><br>
        <a href="index.php?page=employee">Employee Page</a><br>
        <a href="index.php?page=logout">Logout</a>

<?php
    }

    // ADMIN PAGE
    elseif ($page == "admin") {

        if ($_SESSION['job'] != "admin") {
            echo "Access denied.";
            exit();
        }

        $result = $conn->query("SELECT * FROM users");
?>
        <h2>Admin Panel - Users</h2>
        <table border="1">
            <tr><th>ID</th><th>Username</th><th>Job</th></tr>

            <?php while ($row = $result->fetch_assoc()) { ?>
                <tr>
                    <td><?php echo $row['id']; ?></td>
                    <td><?php echo $row['username']; ?></td>
                    <td><?php echo $row['job_title']; ?></td>
                </tr>
            <?php } ?>
        </table>

        <a href="index.php?page=home">Back</a>

<?php
    }

    // EMPLOYEE PAGE
    elseif ($page == "employee") {

        $result = $conn->query("SELECT * FROM announcements");
?>
        <h2>Company Announcements</h2>

        <?php while ($row = $result->fetch_assoc()) { ?>
            <p><?php echo $row['message']; ?></p>
        <?php } ?>

        <a href="index.php?page=home">Back</a>

<?php
    }
}
?>

</body>
</html>