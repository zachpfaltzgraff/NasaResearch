<?php
// ============================================================================
// Single-File PHP Web Application
// Features: User Login, Company Info Viewer, Action Logging, Dynamic Pages
// ============================================================================

session_start();

// Configuration
define('LOG_FILE', __DIR__ . '/user_actions.log');
define('SESSION_TIMEOUT', 3600); // 1 hour

// Initialize user session if needed
if (!isset($_SESSION['user'])) {
    $_SESSION['user'] = null;
    $_SESSION['login_time'] = null;
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * Log user actions to file
 */
function logAction($action, $details = '') {
    $timestamp = date('Y-m-d H:i:s');
    $user = $_SESSION['user'] ?? 'GUEST';
    $ip = $_SERVER['REMOTE_ADDR'] ?? 'UNKNOWN';
    $logEntry = "[{$timestamp}] User: {$user} | IP: {$ip} | Action: {$action}";
    
    if (!empty($details)) {
        $logEntry .= " | Details: {$details}";
    }
    
    $logEntry .= PHP_EOL;
    file_put_contents(LOG_FILE, $logEntry, FILE_APPEND | LOCK_EX);
}

/**
 * Check if user session is valid
 */
function isSessionValid() {
    if (!isset($_SESSION['user']) || $_SESSION['user'] === null) {
        return false;
    }
    
    if (isset($_SESSION['login_time']) && (time() - $_SESSION['login_time']) > SESSION_TIMEOUT) {
        session_destroy();
        return false;
    }
    
    return true;
}

/**
 * Get current page
 */
function getCurrentPage() {
    return $_GET['page'] ?? 'dashboard';
}

/**
 * Validate login credentials
 */
function validateCredentials($username, $password) {
    // Demo users (in production, use a database)
    $validUsers = [
        'admin' => password_hash('admin123', PASSWORD_BCRYPT),
        'user' => password_hash('user123', PASSWORD_BCRYPT),
        'manager' => password_hash('manager123', PASSWORD_BCRYPT)
    ];
    
    if (isset($validUsers[$username]) && password_verify($password, $validUsers[$username])) {
        return true;
    }
    
    return false;
}

/**
 * Get sample company data
 */
function getCompanyData() {
    return [
        [
            'id' => 1,
            'name' => 'Acme Corporation',
            'industry' => 'Technology',
            'employees' => 250,
            'founded' => 2010,
            'website' => 'www.acme.com',
            'revenue' => '$50M'
        ],
        [
            'id' => 2,
            'name' => 'Global Industries Inc',
            'industry' => 'Manufacturing',
            'employees' => 500,
            'founded' => 1995,
            'website' => 'www.global.com',
            'revenue' => '$200M'
        ],
        [
            'id' => 3,
            'name' => 'TechStart Solutions',
            'industry' => 'Software',
            'employees' => 75,
            'founded' => 2018,
            'website' => 'www.techstart.com',
            'revenue' => '$15M'
        ],
        [
            'id' => 4,
            'name' => 'Financial Plus',
            'industry' => 'Finance',
            'employees' => 350,
            'founded' => 2005,
            'website' => 'www.finplus.com',
            'revenue' => '$120M'
        ]
    ];
}

// ============================================================================
// REQUEST HANDLERS
// ============================================================================

// Handle login
if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['action']) && $_POST['action'] === 'login') {
    $username = $_POST['username'] ?? '';
    $password = $_POST['password'] ?? '';
    
    if (validateCredentials($username, $password)) {
        $_SESSION['user'] = $username;
        $_SESSION['login_time'] = time();
        logAction('LOGIN', "User logged in successfully");
        header('Location: ?page=dashboard');
        exit;
    } else {
        $loginError = "Invalid username or password";
        logAction('LOGIN_FAILED', "Failed login attempt for username: {$username}");
    }
}

// Handle logout
if (isset($_GET['action']) && $_GET['action'] === 'logout' && isSessionValid()) {
    logAction('LOGOUT', "User logged out");
    session_destroy();
    header('Location: ?page=login');
    exit;
}

// Handle company info view
if (isset($_GET['view_company']) && isSessionValid()) {
    $companyId = (int)$_GET['view_company'];
    logAction('VIEW_COMPANY', "Viewed company ID: {$companyId}");
}

// ============================================================================
// PAGE RENDERING
// ============================================================================

?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Company Information Portal</title>
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
            max-width: 900px;
        }
        
        header {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 20px;
            border-radius: 10px 10px 0 0;
            display: flex;
            justify-content: space-between;
            align-items: center;
        }
        
        header h1 {
            font-size: 24px;
        }
        
        .user-info {
            display: flex;
            align-items: center;
            gap: 15px;
        }
        
        .logout-btn {
            background: #ff6b6b;
            color: white;
            border: none;
            padding: 8px 16px;
            border-radius: 5px;
            cursor: pointer;
            text-decoration: none;
            font-size: 14px;
        }
        
        .logout-btn:hover {
            background: #ff5252;
        }
        
        nav {
            background: #f8f9fa;
            padding: 0;
            border-bottom: 1px solid #e0e0e0;
            display: flex;
        }
        
        nav a {
            flex: 1;
            padding: 15px 20px;
            text-decoration: none;
            color: #333;
            border-right: 1px solid #e0e0e0;
            transition: background 0.3s;
            text-align: center;
        }
        
        nav a:last-child {
            border-right: none;
        }
        
        nav a:hover {
            background: #e8e8e8;
        }
        
        nav a.active {
            background: #667eea;
            color: white;
        }
        
        .content {
            padding: 40px;
            min-height: 400px;
        }
        
        .login-form {
            max-width: 400px;
            margin: 0 auto;
        }
        
        .form-group {
            margin-bottom: 20px;
        }
        
        .form-group label {
            display: block;
            margin-bottom: 8px;
            font-weight: 600;
            color: #333;
        }
        
        .form-group input {
            width: 100%;
            padding: 12px;
            border: 1px solid #ddd;
            border-radius: 5px;
            font-size: 14px;
        }
        
        .form-group input:focus {
            outline: none;
            border-color: #667eea;
            box-shadow: 0 0 0 3px rgba(102, 126, 234, 0.1);
        }
        
        .form-group button {
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
        }
        
        .form-group button:hover {
            background: #5568d3;
        }
        
        .alert {
            padding: 15px;
            margin-bottom: 20px;
            border-radius: 5px;
            border-left: 4px solid;
        }
        
        .alert-error {
            background: #ffe0e0;
            color: #c00;
            border-left-color: #c00;
        }
        
        .alert-success {
            background: #e0ffe0;
            color: #0a0;
            border-left-color: #0a0;
        }
        
        .companies-grid {
            display: grid;
            grid-template-columns: repeat(auto-fill, minmax(280px, 1fr));
            gap: 20px;
        }
        
        .company-card {
            border: 1px solid #ddd;
            border-radius: 8px;
            padding: 20px;
            transition: all 0.3s;
            cursor: pointer;
        }
        
        .company-card:hover {
            box-shadow: 0 5px 20px rgba(0, 0, 0, 0.1);
            transform: translateY(-2px);
        }
        
        .company-card h3 {
            color: #667eea;
            margin-bottom: 10px;
            font-size: 18px;
        }
        
        .company-card p {
            color: #666;
            margin-bottom: 8px;
            font-size: 14px;
        }
        
        .company-card .info {
            display: flex;
            justify-content: space-between;
            font-size: 13px;
            color: #888;
            margin-top: 12px;
            padding-top: 12px;
            border-top: 1px solid #eee;
        }
        
        .company-detail {
            background: white;
            padding: 30px;
            border: 1px solid #ddd;
            border-radius: 8px;
        }
        
        .company-detail h2 {
            color: #667eea;
            margin-bottom: 20px;
            font-size: 28px;
        }
        
        .detail-row {
            display: flex;
            padding: 12px 0;
            border-bottom: 1px solid #eee;
        }
        
        .detail-row:last-child {
            border-bottom: none;
        }
        
        .detail-label {
            font-weight: 600;
            color: #333;
            width: 150px;
        }
        
        .detail-value {
            color: #666;
        }
        
        .back-btn {
            display: inline-block;
            margin-top: 20px;
            padding: 10px 20px;
            background: #667eea;
            color: white;
            text-decoration: none;
            border-radius: 5px;
            transition: background 0.3s;
        }
        
        .back-btn:hover {
            background: #5568d3;
        }
        
        .welcome-message {
            background: #f0f4ff;
            border-left: 4px solid #667eea;
            padding: 20px;
            margin-bottom: 20px;
            border-radius: 5px;
        }
        
        .welcome-message h2 {
            color: #667eea;
            margin-bottom: 10px;
        }
        
        .footer {
            text-align: center;
            padding: 20px;
            color: #999;
            font-size: 12px;
            border-top: 1px solid #eee;
        }
    </style>
</head>
<body>
    <div class="container">
        <?php
        // LOGIN PAGE
        if (!isSessionValid()) {
            ?>
            <div class="content" style="max-width: 500px; margin: 0 auto; min-height: auto;">
                <h1 style="text-align: center; margin-bottom: 30px; color: #667eea;">Company Portal Login</h1>
                
                <?php if (isset($loginError)): ?>
                    <div class="alert alert-error"><?php echo htmlspecialchars($loginError); ?></div>
                <?php endif; ?>
                
                <form method="POST" class="login-form">
                    <input type="hidden" name="action" value="login">
                    
                    <div class="form-group">
                        <label for="username">Username</label>
                        <input type="text" id="username" name="username" required autofocus>
                    </div>
                    
                    <div class="form-group">
                        <label for="password">Password</label>
                        <input type="password" id="password" name="password" required>
                    </div>
                    
                    <div class="form-group">
                        <button type="submit">Login</button>
                    </div>
                </form>
                
                <div style="margin-top: 30px; padding: 20px; background: #f5f5f5; border-radius: 5px;">
                    <h4 style="color: #333; margin-bottom: 10px;">Demo Credentials:</h4>
                    <p><strong>Username:</strong> admin, user, or manager</p>
                    <p><strong>Password:</strong> admin123, user123, or manager123</p>
                </div>
            </div>
            <?php
        } else {
            // AUTHENTICATED PAGES
            $page = getCurrentPage();
            ?>
            <header>
                <h1>Company Information Portal</h1>
                <div class="user-info">
                    <span>Welcome, <strong><?php echo htmlspecialchars($_SESSION['user']); ?></strong></span>
                    <a href="?action=logout" class="logout-btn">Logout</a>
                </div>
            </header>
            
            <nav>
                <a href="?page=dashboard" class="<?php echo $page === 'dashboard' ? 'active' : ''; ?>">Dashboard</a>
                <a href="?page=companies" class="<?php echo $page === 'companies' ? 'active' : ''; ?>">Companies</a>
                <a href="?page=logs" class="<?php echo $page === 'logs' ? 'active' : ''; ?>">Activity Logs</a>
                <a href="?page=profile" class="<?php echo $page === 'profile' ? 'active' : ''; ?>">Profile</a>
            </nav>
            
            <div class="content">
                <?php
                // PAGE: DASHBOARD
                if ($page === 'dashboard') {
                    logAction('VIEW_PAGE', 'Dashboard');
                    ?>
                    <div class="welcome-message">
                        <h2>Welcome to the Company Portal</h2>
                        <p>You are logged in as <strong><?php echo htmlspecialchars($_SESSION['user']); ?></strong>. 
                        Use the navigation menu to browse companies, view activity logs, and manage your profile.</p>
                    </div>
                    
                    <h2 style="margin-bottom: 20px;">Quick Stats</h2>
                    <div class="companies-grid">
                        <div class="company-card">
                            <h3>📊 Total Companies</h3>
                            <p style="font-size: 28px; color: #667eea; font-weight: bold;">4</p>
                        </div>
                        <div class="company-card">
                            <h3>👥 Total Employees</h3>
                            <p style="font-size: 28px; color: #667eea; font-weight: bold;">1,175+</p>
                        </div>
                        <div class="company-card">
                            <h3>💼 Industries Covered</h3>
                            <p style="font-size: 28px; color: #667eea; font-weight: bold;">4</p>
                        </div>
                    </div>
                    <?php
                }
                
                // PAGE: COMPANIES
                elseif ($page === 'companies') {
                    logAction('VIEW_PAGE', 'Companies List');
                    
                    // Check if viewing single company
                    if (isset($_GET['view_company'])) {
                        $companyId = (int)$_GET['view_company'];
                        $companies = getCompanyData();
                        $company = null;
                        
                        foreach ($companies as $c) {
                            if ($c['id'] === $companyId) {
                                $company = $c;
                                break;
                            }
                        }
                        
                        if ($company) {
                            ?>
                            <div class="company-detail">
                                <h2><?php echo htmlspecialchars($company['name']); ?></h2>
                                
                                <div class="detail-row">
                                    <span class="detail-label">Industry:</span>
                                    <span class="detail-value"><?php echo htmlspecialchars($company['industry']); ?></span>
                                </div>
                                
                                <div class="detail-row">
                                    <span class="detail-label">Founded:</span>
                                    <span class="detail-value"><?php echo htmlspecialchars($company['founded']); ?></span>
                                </div>
                                
                                <div class="detail-row">
                                    <span class="detail-label">Employees:</span>
                                    <span class="detail-value"><?php echo htmlspecialchars($company['employees']); ?></span>
                                </div>
                                
                                <div class="detail-row">
                                    <span class="detail-label">Annual Revenue:</span>
                                    <span class="detail-value"><?php echo htmlspecialchars($company['revenue']); ?></span>
                                </div>
                                
                                <div class="detail-row">
                                    <span class="detail-label">Website:</span>
                                    <span class="detail-value"><a href="https://<?php echo htmlspecialchars($company['website']); ?>" target="_blank"><?php echo htmlspecialchars($company['website']); ?></a></span>
                                </div>
                                
                                <a href="?page=companies" class="back-btn">← Back to Companies</a>
                            </div>
                            <?php
                        } else {
                            echo '<div class="alert alert-error">Company not found.</div>';
                        }
                    } else {
                        // List all companies
                        $companies = getCompanyData();
                        ?>
                        <h2 style="margin-bottom: 20px;">Company Directory</h2>
                        <div class="companies-grid">
                            <?php foreach ($companies as $company): ?>
                                <a href="?page=companies&view_company=<?php echo $company['id']; ?>" style="text-decoration: none; color: inherit;">
                                    <div class="company-card">
                                        <h3><?php echo htmlspecialchars($company['name']); ?></h3>
                                        <p><strong><?php echo htmlspecialchars($company['industry']); ?></strong></p>
                                        <p><?php echo htmlspecialchars($company['employees']); ?> employees</p>
                                        <div class="info">
                                            <span>Founded: <?php echo htmlspecialchars($company['founded']); ?></span>
                                            <span><?php echo htmlspecialchars($company['revenue']); ?></span>
                                        </div>
                                    </div>
                                </a>
                            <?php endforeach; ?>
                        </div>
                        <?php
                    }
                }
                
                // PAGE: ACTIVITY LOGS
                elseif ($page === 'logs') {
                    logAction('VIEW_PAGE', 'Activity Logs');
                    ?>
                    <h2 style="margin-bottom: 20px;">User Activity Logs</h2>
                    
                    <?php if (file_exists(LOG_FILE)): ?>
                        <div style="background: #f5f5f5; padding: 20px; border-radius: 5px; font-family: monospace; max-height: 500px; overflow-y: auto; border: 1px solid #ddd;">
                            <?php
                            $logs = file_get_contents(LOG_FILE);
                            $logs = htmlspecialchars($logs);
                            echo nl2br($logs);
                            ?>
                        </div>
                        <p style="margin-top: 15px; color: #999; font-size: 12px;">
                            📝 Log file location: <?php echo htmlspecialchars(LOG_FILE); ?>
                        </p>
                    <?php else: ?>
                        <div class="alert alert-error">No activity logs found yet.</div>
                    <?php endif; ?>
                    <?php
                }
                
                // PAGE: PROFILE
                elseif ($page === 'profile') {
                    logAction('VIEW_PAGE', 'User Profile');
                    ?>
                    <h2 style="margin-bottom: 20px;">User Profile</h2>
                    
                    <div style="background: #f5f5f5; padding: 30px; border-radius: 8px; max-width: 500px;">
                        <div class="detail-row">
                            <span class="detail-label">Username:</span>
                            <span class="detail-value"><?php echo htmlspecialchars($_SESSION['user']); ?></span>
                        </div>
                        
                        <div class="detail-row">
                            <span class="detail-label">Login Time:</span>
                            <span class="detail-value"><?php echo date('Y-m-d H:i:s', $_SESSION['login_time']); ?></span>
                        </div>
                        
                        <div class="detail-row">
                            <span class="detail-label">Session Duration:</span>
                            <span class="detail-value"><?php echo round((time() - $_SESSION['login_time']) / 60); ?> minutes</span>
                        </div>
                        
                        <div class="detail-row">
                            <span class="detail-label">IP Address:</span>
                            <span class="detail-value"><?php echo htmlspecialchars($_SERVER['REMOTE_ADDR']); ?></span>
                        </div>
                        
                        <div class="detail-row">
                            <span class="detail-label">User Agent:</span>
                            <span class="detail-value" style="font-size: 12px;"><?php echo htmlspecialchars(substr($_SERVER['HTTP_USER_AGENT'], 0, 50)); ?>...</span>
                        </div>
                    </div>
                    <?php
                }
                
                // DEFAULT*
