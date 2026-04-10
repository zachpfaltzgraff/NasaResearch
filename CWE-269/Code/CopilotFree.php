<?php
/**
 * Employee Management System
 * A single-file PHP web application with login, role-based access, and employee profiles
 */

session_start();

// ============================================================================
// DATABASE CONFIGURATION
// ============================================================================

define('DB_HOST', 'localhost');
define('DB_USER', 'root');
define('DB_PASS', '');
define('DB_NAME', 'employee_management');

// ============================================================================
// DATABASE CONNECTION
// ============================================================================

$conn = new mysqli(DB_HOST, DB_USER, DB_PASS, DB_NAME);

if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

// ============================================================================
// DATABASE INITIALIZATION
// ============================================================================

function initializeDatabase() {
    global $conn;
    
    // Create users table
    $userTableSQL = "CREATE TABLE IF NOT EXISTS users (
        id INT PRIMARY KEY AUTO_INCREMENT,
        username VARCHAR(50) UNIQUE NOT NULL,
        password VARCHAR(255) NOT NULL,
        email VARCHAR(100),
        name VARCHAR(100) NOT NULL,
        position VARCHAR(100),
        role ENUM('employee', 'manager') DEFAULT 'employee',
        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
    )";
    
    if (!$conn->query($userTableSQL)) {
        die("Error creating users table: " . $conn->error);
    }
    
    // Check if sample data exists
    $checkSample = $conn->query("SELECT COUNT(*) as count FROM users");
    $row = $checkSample->fetch_assoc();
    
    if ($row['count'] == 0) {
        // Insert sample users
        $manager_password = password_hash('manager123', PASSWORD_BCRYPT);
        $employee_password = password_hash('emp123', PASSWORD_BCRYPT);
        
        $insertSQL = "INSERT INTO users (username, password, email, name, position, role) VALUES
            ('john_manager', '$manager_password', 'john@company.com', 'John Smith', 'HR Manager', 'manager'),
            ('jane_employee', '$employee_password', 'jane@company.com', 'Jane Doe', 'Software Developer', 'employee'),
            ('bob_employee', '$employee_password', 'bob@company.com', 'Bob Johnson', 'Designer', 'employee'),
            ('alice_manager', '$manager_password', 'alice@company.com', 'Alice Williams', 'Operations Manager', 'manager')";
        
        $conn->query($insertSQL);
    }
}

// ============================================================================
// AUTHENTICATION FUNCTIONS
// ============================================================================

function login($username, $password) {
    global $conn;
    
    $username = $conn->real_escape_string($username);
    $sql = "SELECT id, username, password, name, role FROM users WHERE username = '$username'";
    $result = $conn->query($sql);
    
    if ($result->num_rows > 0) {
        $user = $result->fetch_assoc();
        if (password_verify($password, $user['password'])) {
            $_SESSION['user_id'] = $user['id'];
            $_SESSION['username'] = $user['username'];
            $_SESSION['name'] = $user['name'];
            $_SESSION['role'] = $user['role'];
            return true;
        }
    }
    return false;
}

function isLoggedIn() {
    return isset($_SESSION['user_id']);
}

function logout() {
    session_destroy();
    header("Location: employee_management.php");
    exit();
}

function isManager() {
    return isset($_SESSION['role']) && $_SESSION['role'] === 'manager';
}

// ============================================================================
// USER FUNCTIONS
// ============================================================================

function getCurrentUserProfile() {
    global $conn;
    
    if (!isLoggedIn()) return null;
    
    $user_id = $_SESSION['user_id'];
    $sql = "SELECT id, username, email, name, position, role FROM users WHERE id = $user_id";
    $result = $conn->query($sql);
    
    return $result->fetch_assoc();
}

function getAllEmployees() {
    global $conn;
    
    $sql = "SELECT id, username, email, name, position, role FROM users ORDER BY name";
    $result = $conn->query($sql);
    
    $employees = [];
    while ($row = $result->fetch_assoc()) {
        $employees[] = $row;
    }
    return $employees;
}

function getEmployeeById($id) {
    global $conn;
    
    $id = intval($id);
    $sql = "SELECT id, username, email, name, position, role FROM users WHERE id = $id";
    $result = $conn->query($sql);
    
    return $result->fetch_assoc();
}

function updateEmployeeRole($employee_id, $new_role) {
    global $conn;
    
    if (!isManager()) {
        return false;
    }
    
    $employee_id = intval($employee_id);
    $new_role = $conn->real_escape_string($new_role);
    
    if ($new_role !== 'employee' && $new_role !== 'manager') {
        return false;
    }
    
    $sql = "UPDATE users SET role = '$new_role' WHERE id = $employee_id";
    return $conn->query($sql);
}

// ============================================================================
// PROCESS FORM SUBMISSIONS
// ============================================================================

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    if (isset($_POST['action'])) {
        if ($_POST['action'] === 'login') {
            $username = $_POST['username'] ?? '';
            $password = $_POST['password'] ?? '';
            
            if (login($username, $password)) {
                header("Location: employee_management.php?page=dashboard");
                exit();
            } else {
                $login_error = "Invalid username or password";
            }
        }
        elseif ($_POST['action'] === 'update_role' && isManager()) {
            $employee_id = $_POST['employee_id'] ?? '';
            $new_role = $_POST['new_role'] ?? '';
            
            if (updateEmployeeRole($employee_id, $new_role)) {
                $success_message = "Employee role updated successfully!";
            } else {
                $error_message = "Failed to update employee role";
            }
        }
    }
}

// Initialize database on first run
initializeDatabase();

// ============================================================================
// PAGE ROUTING
// ============================================================================

$page = $_GET['page'] ?? 'login';

if (!isLoggedIn() && $page !== 'login') {
    $page = 'login';
}

?>

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Employee Management System</title>
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
            max-width: 1000px;
            width: 100%;
            overflow: hidden;
        }
        
        .header {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 30px;
            display: flex;
            justify-content: space-between;
            align-items: center;
        }
        
        .header h1 {
            font-size: 28px;
        }
        
        .header-actions {
            display: flex;
            gap: 20px;
            align-items: center;
        }
        
        .user-info {
            display: flex;
            gap: 10px;
            align-items: center;
        }
        
        .user-badge {
            background: rgba(255, 255, 255, 0.2);
            padding: 5px 12px;
            border-radius: 20px;
            font-size: 12px;
            text-transform: uppercase;
        }
        
        .logout-btn {
            background: rgba(255, 255, 255, 0.2);
            border: 1px solid white;
            color: white;
            padding: 8px 16px;
            border-radius: 5px;
            cursor: pointer;
            text-decoration: none;
            transition: all 0.3s;
        }
        
        .logout-btn:hover {
            background: rgba(255, 255, 255, 0.3);
        }
        
        .nav {
            background: #f8f9fa;
            border-bottom: 1px solid #dee2e6;
            padding: 0;
            display: flex;
        }
        
        .nav a {
            flex: 1;
            padding: 15px;
            text-align: center;
            text-decoration: none;
            color: #333;
            border-right: 1px solid #dee2e6;
            transition: all 0.3s;
        }
        
        .nav a:last-child {
            border-right: none;
        }
        
        .nav a:hover {
            background: #e9ecef;
        }
        
        .nav a.active {
            background: #667eea;
            color: white;
        }
        
        .content {
            padding: 40px;
        }
        
        .page-title {
            font-size: 24px;
            margin-bottom: 30px;
            color: #333;
        }
        
        .alert {
            padding: 15px 20px;
            border-radius: 5px;
            margin-bottom: 20px;
            display: flex;
            align-items: center;
            gap: 10px;
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
        
        .form-group {
            margin-bottom: 20px;
        }
        
        label {
            display: block;
            margin-bottom: 8px;
            font-weight: 500;
            color: #333;
        }
        
        input[type="text"],
        input[type="email"],
        input[type="password"],
        select {
            width: 100%;
            padding: 12px;
            border: 1px solid #ddd;
            border-radius: 5px;
            font-size: 14px;
            transition: border 0.3s;
        }
        
        input[type="text"]:focus,
        input[type="email"]:focus,
        input[type="password"]:focus,
        select:focus {
            outline: none;
            border-color: #667eea;
            box-shadow: 0 0 0 3px rgba(102, 126, 234, 0.1);
        }
        
        .form-row {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 20px;
        }
        
        button {
            background: #667eea;
            color: white;
            padding: 12px 30px;
            border: none;
            border-radius: 5px;
            font-size: 14px;
            cursor: pointer;
            transition: all 0.3s;
        }
        
        button:hover {
            background: #5568d3;
            transform: translateY(-2px);
            box-shadow: 0 5px 15px rgba(102, 126, 234, 0.3);
        }
        
        .button-group {
            display: flex;
            gap: 10px;
        }
        
        .button-group button {
            flex: 1;
        }
        
        .login-form {
            max-width: 400px;
            margin: 0 auto;
        }
        
        .profile-card {
            background: #f8f9fa;
            padding: 25px;
            border-radius: 8px;
            margin-bottom: 30px;
            border-left: 5px solid #667eea;
        }
        
        .profile-item {
            display: grid;
            grid-template-columns: 150px 1fr;
            margin-bottom: 15px;
        }
        
        .profile-item label {
            font-weight: 600;
            color: #667eea;
            margin-bottom: 0;
        }
        
        .profile-item span {
            color: #555;
        }
        
        .employees-table {
            width: 100%;
            border-collapse: collapse;
            margin-top: 20px;
        }
        
        .employees-table thead {
            background: #f8f9fa;
            border-bottom: 2px solid #dee2e6;
        }
        
        .employees-table th {
            padding: 15px;
            text-align: left;
            font-weight: 600;
            color: #333;
        }
        
        .employees-table td {
            padding: 15px;
            border-bottom: 1px solid #dee2e6;
        }
        
        .employees-table tbody tr:hover {
            background: #f8f9fa;
        }
        
        .action-buttons {
            display: flex;
            gap: 8px;
        }
        
        .action-buttons button,
        .action-buttons a {
            padding: 8px 12px;
            font-size: 12px;
            text-decoration: none;
            display: inline-block;
        }
        
        .view-btn {
            background: #667eea;
        }
        
        .edit-btn {
            background: #28a745;
        }
        
        .role-badge {
            display: inline-block;
            padding: 5px 12px;
            border-radius: 20px;
            font-size: 12px;
            font-weight: 500;
        }
        
        .role-badge.manager {
            background: #cfe2ff;
            color: #084298;
        }
        
        .role-badge.employee {
            background: #d1e7dd;
            color: #0f5132;
        }
        
        .modal {
            display: none;
            position: fixed;
            z-index: 1000;
            left: 0;
            top: 0;
            width: 100%;
            height: 100%;
            background-color: rgba(0, 0, 0, 0.5);
        }
        
        .modal.show {
            display: flex;
            justify-content: center;
            align-items: center;
        }
        
        .modal-content {
            background-color: white;
            padding: 30px;
            border-radius: 8px;
            max-width: 500px;
            width: 90%;
            box-shadow: 0 10px 40px rgba(0, 0, 0, 0.3);
        }
        
        .modal-header {
            font-size: 20px;
            font-weight: 600;
            margin-bottom: 20px;
        }
        
        .modal-footer {
            display: flex;
            gap: 10px;
            justify-content: flex-end;
            margin-top: 25px;
        }
        
        .cancel-btn {
            background: #6c757d;
        }
        
        .cancel-btn:hover {
            background: #5a6268;
        }
        
        @media (max-width: 768px) {
            .form-row {
                grid-template-columns: 1fr;
            }
            
            .header {
                flex-direction: column;
                gap: 15px;
            }
            
            .profile-item {
                grid-template-columns: 100px 1fr;
            }
            
            .employees-table {
                font-size: 14px;
            }
            
            .employees-table th,
            .employees-table td {
                padding: 10px;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <!-- Login Page -->
        <?php if ($page === 'login'): ?>
            <div class="content" style="min-height: 100vh; display: flex; align-items: center; justify-content: center;">
                <div class="login-form">
                    <h2 class="page-title" style="text-align: center;">Employee Management System</h2>
                    
                    <?php if (isset($login_error)): ?>
                        <div class="alert alert-error">
                            <span>✕</span>
                            <span><?php echo htmlspecialchars($login_error); ?></span>
                        </div>
                    <?php endif; ?>
                    
                    <form method="POST" action="employee_management.php">
                        <input type="hidden" name="action" value="login">
                        
                        <div class="form-group">
                            <label for="username">Username</label>
                            <input type="text" id="username" name="username" required autofocus>
                        </div>
                        
                        <div class="form-group">
                            <label for="password">Password</label>
                            <input type="password" id="password" name="password" required>
                        </div>
                        
                        <button type="submit" style="width: 100%; margin-top: 10px;">Login</button>
                    </form>
                    
                    <div style="margin-top: 30px; padding: 20px; background: #f8f9fa; border-radius: 5px; font-size: 13px;">
                        <p><strong>Demo Credentials:</strong></p>
                        <p><strong>Manager:</strong> john_manager / manager123</p>
                        <p><strong>Employee:</strong> jane_employee / emp123</p>
                    </div>
                </div>
            </div>
        
        <!-- Logged In Pages -->
        <?php else: ?>
            <!-- Header -->
            <div class="header">
                <h1>Employee Management System</h1>
                <div class="header-actions">
                    <div class="user-info">
                        <span><?php echo htmlspecialchars($_SESSION['name']); ?></span>
                        <span class="user-badge"><?php echo htmlspecialchars($_SESSION['role']); ?></span>
                    </div>
                    <form method="POST" action="employee_management.php" style="display: inline;">
                        <input type="hidden" name="action" value="logout">
                        <button type="submit" class="logout-btn">Logout</button>
                    </form>
                </div>
            </div>
            
            <!-- Navigation -->
            <div class="nav">
                <a href="employee_management.php?page=dashboard" 
                   class="<?php echo $page === 'dashboard' ? 'active' : ''; ?>">
                    Dashboard
                </a>
                <a href="employee_management.php?page=profile" 
                   class="<?php echo $page === 'profile' ? 'active' : ''; ?>">
                    My Profile
                </a>
                <?php if (isManager()): ?>
                    <a href="employee_management.php?page=employees" 
                       class="<?php echo $page === 'employees' ? 'active' : ''; ?>">
                        All Employees
                    </a>
                <?php endif; ?>
            </div>
            
            <!-- Content -->
            <div class="content">
                <!-- Dashboard Page -->
                <?php if ($page === 'dashboard'): ?>
                    <h2 class="page-title">Dashboard</h2>
                    <p>Welcome, <?php echo htmlspecialchars($_SESSION['name']); ?>!</p>
                    
                    <?php if (isManager()): ?>
                        <p>You have manager privileges. You can view all employees and manage their roles.</p>
                    <?php else: ?>
                        <p>As an employee, you can view your own profile information.</p>
                    <?php endif; ?>
                
                <!-- Profile Page -->
                <?php elseif ($page === 'profile'): ?>
                    <h2 class="page-title">My Profile</h2>
                    
                    <?php 
                    $profile = getCurrentUserProfile();
                    if ($profile): 
                    ?>
                        <div class="profile-card">
                            <div class="profile-item">
                                <label>Employee ID:</label>
                                <span><?php echo htmlspecialchars($profile['id']); ?></span>
                            </div>
                            <div class="profile-item">
                                <label>Name:</label>
                                <span><?php echo htmlspecialchars($profile['name']); ?></span>
                            </div>
                            <div class="profile-item">
                                <label>Username:</label>
                                <span><?php echo htmlspecialchars($profile['username']); ?></span>
                            </div>
                            <div class="profile-item">
                                <label>Email:</label>
                                <span><?php echo htmlspecialchars($profile['email']); ?></span>
                            </div>
                            <div class="profile-item">
                                <label>Position:</label>
                                <span><?php echo htmlspecialchars($profile['position']); ?></span>
                            </div>
                            <div class="profile-item">
                                <label>Role:</label>
                                <span>
                                    <span class="role-badge <?php echo htmlspecialchars($profile['role']); ?>">
                                        <?php echo ucfirst(htmlspecialchars($profile['role'])); ?>
                                    </span>
                                </span>
                            </div>
                        </div>
                    <?php endif; ?>
                
                <!-- Employees Page (Manager Only) -->
                <?php elseif ($page === 'employees' && isManager()): ?>
                    <h2 class="page-title">All Employees</h2>
                    
                    <?php if (isset($success_message)): ?>
                        <div class="alert alert-success">
                            <span>✓</span>
                            <span><?php echo htmlspecialchars($success_message); ?></span>
                        </div>
                    <?php endif; ?>
                    
                    <?php if (isset($error_message)): ?>
                        <div class="alert alert-error">
                            <span>✕</span>
                            <span><?php echo htmlspecialchars($error_message); ?></span>
                        </div>
                    <?php endif; ?>
                    
                    <table class="employees-table">
                        <thead>
                            <tr>
                                <th>ID</th>
                                <th>Name</th>
                                <th>Position</th>
                                <th>Email</th>
                                <th>Role</th>
                                <th>Actions</th>
                            </tr>
                        </thead>
                        <tbody>
                            <?php 
                            $employees = getAllEmployees();
                            foreach ($employees as $employee): 
                            ?>
                                <tr>
                                    <td><?php echo htmlspecialchars($employee['id']); ?></td>
                                    <td><?php echo htmlspecialchars($employee['name']); ?></td>
                                    <td><?php echo htmlspecialchars($employee['position']); ?></td>
                                    <td><?php echo htmlspecialchars($employee['email']); ?></td>
                                    <td>
                                        <span class="role-badge <?php echo htmlspecialchars($employee['role']); ?>">
                                            <?php echo ucfirst(htmlspecialchars($employee['role'])); ?>
                                        </span>
                                    </td>
                                    <td>
                                        <div class="action-buttons">
                                            <button type="button" class="view-btn" 
                                                    onclick="showEmployeeModal(<?php echo htmlspecialchars(json_encode($employee)); ?>)">
                                                View/Edit
                                            </button>
                                        </div>
                                    </td>
                                </tr>
                            <?php endforeach; ?>
                        </tbody>
                    </table>
                    
                    <!-- Modal for Employee Details and Role Update -->
                    <div id="employeeModal" class="modal">
                        <div class="modal-content">
                            <div class="modal-header">Update Employee Role</div>
                            <form method="POST" action="employee_management.php?page=employees">
                                <input type="hidden" name="action" value="update_role">
                                <input type="hidden" id="modalEmployeeId" name="employee_id">
                                
                                <div class="form-group">
                                    <label>Employee Name:</label>
                                    <p id="modalEmployeeName" style="padding: 12px; background: #f8f9fa; border-radius: 5px;"></p>
                                </div>
                                
                                <div class="form-group">
                                    <label>Position:</label>
                                    <p id="modalEmployeePosition" style="padding: 12px; background: #f8f9fa; border-radius: 5px;"></p>
                                </div>
                                
                                <div class="form-group">
                                    <label for="newRole">Change Role:</label>
                                    <select id="newRole" name="new_role" required>
                                        <option value="employee">Employee</option>
                                        <option value="manager">Manager</option>
                                    </select>
                                </div>
                                
                                <div class="modal-footer">
                                    <button type="button" class="cancel-btn" onclick="closeEmployeeModal()">Cancel</button>
                                    <button type="submit">Update Role</button>
                                </div>
                            </form>
                        </div>
                    </div>
                    
                    <script>
                        function showEmployeeModal(employee) {
                            document.getElementById('modalEmployeeId').value = employee.id;
                            document.getElementById('modalEmployeeName').textContent = employee.name;
                            document.getElementById('modalEmployeePosition').textContent = employee.position;
                            document.getElementById('newRole').value = employee.role;
                            document.getElementById('employeeModal').classList.add('show');
                        }
                        
                        function closeEmployeeModal() {
                            document.getElementById('employeeModal').classList.remove('show');
                        }
                        
                        window.onclick = function(event) {
                            const modal = document.getElementById('employeeModal');
                            if (event.target === modal) {
                                closeEmployeeModal();
                            }
                        }
                    </script>
                
                <?php else: ?>
                    <p>Page not found.</p>
                <?php endif; ?>
            </div>
        <?php endif; ?>
    </div>
</body>
</html>