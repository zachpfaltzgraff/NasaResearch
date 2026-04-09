<?php
session_start();

// ==================== DATABASE CONFIGURATION ====================
$host = 'localhost';
$db = 'employee_db';
$db_user = 'root';
$db_pass = 'password';

// Create database connection
$conn = new mysqli($host, $db_user, $db_pass, $db);

// Check connection
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

// ==================== PAGE CONTROL ====================
$page = isset($_GET['page']) ? $_GET['page'] : 'login';

// Redirect to login if not authenticated
if ($page !== 'login' && !isset($_SESSION['user_id'])) {
    header('Location: ?page=login');
    exit;
}

// ==================== LOGIN HANDLER ====================
if (isset($_POST['login'])) {
    $username = mysqli_real_escape_string($conn, $_POST['username']);
    $password = mysqli_real_escape_string($conn, $_POST['password']);
    
    // Query user from database
    $result = $conn->query("SELECT * FROM users WHERE username='$username'");
    
    if ($result->num_rows == 1) {
        $user = $result->fetch_assoc();
        
        // Verify password (using password_verify for hashed passwords)
        if (password_verify($password, $user['password'])) {
            $_SESSION['user_id'] = $user['id'];
            $_SESSION['username'] = $user['username'];
            $_SESSION['role'] = $user['role']; // 'employee' or 'manager'
            $_SESSION['department'] = $user['department'];
            
            $page = ($_SESSION['role'] == 'manager') ? 'manager_dashboard' : 'employee_dashboard';
        } else {
            $login_error = "Invalid username or password.";
            $page = 'login';
        }
    } else {
        $login_error = "Invalid username or password.";
        $page = 'login';
    }
}

// ==================== LOGOUT HANDLER ====================
if (isset($_POST['logout'])) {
    session_destroy();
    header('Location: ?page=login');
    exit;
}

// ==================== SEARCH HANDLER (EMPLOYEE) ====================
$employee_search_results = null;
$search_error = '';

if (isset($_POST['employee_search'])) {
    $employee_id = mysqli_real_escape_string($conn, $_POST['search_id']);
    
    if (empty($employee_id)) {
        $search_error = "Please enter an ID number.";
    } else {
        $result = $conn->query("SELECT id, name, salary, id_number, department, ssn FROM employees WHERE id_number='$employee_id'");
        
        if ($result->num_rows > 0) {
            $employee_search_results = $result->fetch_assoc();
        } else {
            $search_error = "No employee found with ID: $employee_id";
        }
    }
    
    $page = 'employee_dashboard';
}

// ==================== SEARCH HANDLER (MANAGER - DEPARTMENT) ====================
$department_employees = null;
$dept_search_error = '';

if (isset($_POST['department_search'])) {
    $department = mysqli_real_escape_string($conn, $_POST['search_department']);
    
    if (empty($department)) {
        $dept_search_error = "Please select a department.";
    } else {
        $result = $conn->query("SELECT id, name, salary, id_number, department FROM employees WHERE department='$department' ORDER BY salary DESC");
        
        if ($result->num_rows > 0) {
            $department_employees = [];
            while ($row = $result->fetch_assoc()) {
                $department_employees[] = $row;
            }
        } else {
            $dept_search_error = "No employees found in department: $department";
        }
    }
    
    $page = 'manager_dashboard';
}

?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Employee Database System</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            padding: 20px;
        }
        
        .container {
            background: white;
            border-radius: 10px;
            box-shadow: 0 10px 40px rgba(0, 0, 0, 0.2);
            width: 100%;
            max-width: 600px;
            padding: 40px;
        }
        
        h1, h2 {
            color: #333;
            margin-bottom: 30px;
            text-align: center;
        }
        
        h3 {
            color: #555;
            margin-top: 25px;
            margin-bottom: 15px;
        }
        
        .form-group {
            margin-bottom: 20px;
        }
        
        label {
            display: block;
            margin-bottom: 8px;
            color: #555;
            font-weight: 500;
        }
        
        input[type="text"],
        input[type="password"],
        select {
            width: 100%;
            padding: 12px;
            border: 1px solid #ddd;
            border-radius: 5px;
            font-size: 14px;
            transition: border-color 0.3s;
        }
        
        input[type="text"]:focus,
        input[type="password"]:focus,
        select:focus {
            outline: none;
            border-color: #667eea;
            box-shadow: 0 0 5px rgba(102, 126, 234, 0.1);
        }
        
        button {
            width: 100%;
            padding: 12px;
            background: #667eea;
            color: white;
            border: none;
            border-radius: 5px;
            font-size: 16px;
            font-weight: 600;
            cursor: pointer;
            transition: background 0.3s;
            margin-top: 10px;
        }
        
        button:hover {
            background: #5568d3;
        }
        
        button.secondary {
            background: #dc3545;
            margin-top: 0;
            width: auto;
            float: right;
        }
        
        button.secondary:hover {
            background: #c82333;
        }
        
        .alert {
            padding: 12px;
            margin-bottom: 20px;
            border-radius: 5px;
        }
        
        .alert-error {
            background: #f8d7da;
            color: #721c24;
            border: 1px solid #f5c6cb;
        }
        
        .alert-success {
            background: #d4edda;
            color: #155724;
            border: 1px solid #c3e6cb;
        }
        
        .header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 30px;
        }
        
        .user-info {
            font-size: 14px;
            color: #666;
        }
        
        .results-table {
            width: 100%;
            border-collapse: collapse;
            margin-top: 20px;
        }
        
        .results-table th,
        .results-table td {
            padding: 12px;
            text-align: left;
            border-bottom: 1px solid #ddd;
        }
        
        .results-table th {
            background: #f8f9fa;
            font-weight: 600;
            color: #333;
        }
        
        .results-table tr:hover {
            background: #f8f9fa;
        }
        
        .info-card {
            background: #f8f9fa;
            padding: 20px;
            border-radius: 5px;
            margin-top: 20px;
            border-left: 4px solid #667eea;
        }
        
        .info-row {
            display: flex;
            justify-content: space-between;
            padding: 10px 0;
            border-bottom: 1px solid #ddd;
        }
        
        .info-row:last-child {
            border-bottom: none;
        }
        
        .info-label {
            font-weight: 600;
            color: #555;
        }
        
        .info-value {
            color: #333;
        }
        
        .clearfix::after {
            content: "";
            display: table;
            clear: both;
        }
    </style>
</head>
<body>
    <div class="container">
        
        <!-- ==================== LOGIN PAGE ==================== -->
        <?php if ($page == 'login'): ?>
            <h1>Employee Database System</h1>
            
            <?php if (isset($login_error)): ?>
                <div class="alert alert-error"><?php echo $login_error; ?></div>
            <?php endif; ?>
            
            <form method="POST">
                <div class="form-group">
                    <label for="username">Username:</label>
                    <input type="text" id="username" name="username" required autofocus>
                </div>
                
                <div class="form-group">
                    <label for="password">Password:</label>
                    <input type="password" id="password" name="password" required>
                </div>
                
                <button type="submit" name="login">Login</button>
            </form>
            
            <p style="text-align: center; margin-top: 20px; color: #666;">
                <small>Demo: Use username 'employee' / password 'pass123' or 'manager' / password 'pass123'</small>
            </p>
        
        <!-- ==================== EMPLOYEE DASHBOARD ==================== -->
        <?php elseif ($page == 'employee_dashboard'): ?>
            <div class="header clearfix">
                <div>
                    <h2>Employee Dashboard</h2>
                    <div class="user-info">Logged in as: <strong><?php echo $_SESSION['username']; ?></strong></div>
                </div>
                <form method="POST" style="display: inline;">
                    <button type="submit" name="logout" class="secondary">Logout</button>
                </form>
            </div>
            
            <h3>Search Employee Information</h3>
            
            <?php if (!empty($search_error)): ?>
                <div class="alert alert-error"><?php echo $search_error; ?></div>
            <?php endif; ?>
            
            <form method="POST">
                <div class="form-group">
                    <label for="search_id">Enter ID Number:</label>
                    <input type="text" id="search_id" name="search_id" placeholder="e.g., EMP001" required>
                </div>
                <button type="submit" name="employee_search">Search</button>
            </form>
            
            <?php if ($employee_search_results): ?>
                <div class="info-card">
                    <h3>Employee Information</h3>
                    <div class="info-row">
                        <span class="info-label">Name:</span>
                        <span class="info-value"><?php echo htmlspecialchars($employee_search_results['name']); ?></span>
                    </div>
                    <div class="info-row">
                        <span class="info-label">ID Number:</span>
                        <span class="info-value"><?php echo htmlspecialchars($employee_search_results['id_number']); ?></span>
                    </div>
                    <div class="info-row">
                        <span class="info-label">Department:</span>
                        <span class="info-value"><?php echo htmlspecialchars($employee_search_results['department']); ?></span>
                    </div>
                    <div class="info-row">
                        <span class="info-label">Salary:</span>
                        <span class="info-value">$<?php echo number_format($employee_search_results['salary'], 2); ?></span>
                    </div>
                    <div class="info-row">
                        <span class="info-label">SSN:</span>
                        <span class="info-value"><?php echo htmlspecialchars($employee_search_results['ssn']); ?></span>
                    </div>
                </div>
            <?php endif; ?>
        
        <!-- ==================== MANAGER DASHBOARD ==================== -->
        <?php elseif ($page == 'manager_dashboard'): ?>
            <div class="header clearfix">
                <div>
                    <h2>Manager Dashboard</h2>
                    <div class="user-info">Logged in as: <strong><?php echo $_SESSION['username']; ?></strong> (Department: <?php echo $_SESSION['department']; ?>)</div>
                </div>
                <form method="POST" style="display: inline;">
                    <button type="submit" name="logout" class="secondary">Logout</button>
                </form>
            </div>
            
            <h3>Search Department Employees</h3>
            
            <?php if (!empty($dept_search_error)): ?>
                <div class="alert alert-error"><?php echo $dept_search_error; ?></div>
            <?php endif; ?>
            
            <form method="POST">
                <div class="form-group">
                    <label for="search_department">Select Department:</label>
                    <select id="search_department" name="search_department" required>
                        <option value="">-- Choose Department --</option>
                        <option value="HR">HR</option>
                        <option value="IT">IT</option>
                        <option value="Finance">Finance</option>
                        <option value="Sales">Sales</option>
                        <option value="Operations">Operations</option>
                    </select>
                </div>
                <button type="submit" name="department_search">Search</button>
            </form>
            
            <?php if ($department_employees): ?>
                <div style="margin-top: 30px;">
                    <h3>Employees in Department</h3>
                    <table class="results-table">
                        <thead>
                            <tr>
                                <th>Name</th>
                                <th>ID Number</th>
                                <th>Department</th>
                                <th>Salary</th>
                            </tr>
                        </thead*
