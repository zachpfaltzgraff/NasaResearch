<?php
// File Upload Manager - Corporate Site Integration
// Single-file PHP application for handling file uploads and display

// Configuration
define('UPLOAD_DIR', 'uploads/');
define('MAX_FILE_SIZE', 5 * 1024 * 1024); // 5MB
define('ALLOWED_EXTENSIONS', ['pdf', 'doc', 'docx', 'xls', 'xlsx', 'jpg', 'jpeg', 'png', 'gif', 'txt', 'zip']);

// Create uploads directory if it doesn't exist
if (!is_dir(UPLOAD_DIR)) {
    mkdir(UPLOAD_DIR, 0755, true);
}

// Handle file upload
$upload_message = '';
$upload_error = '';

if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_FILES['file_upload'])) {
    $file = $_FILES['file_upload'];
    
    // Validate file
    if ($file['error'] !== UPLOAD_ERR_OK) {
        $upload_error = 'File upload error: ' . $file['error'];
    } elseif ($file['size'] > MAX_FILE_SIZE) {
        $upload_error = 'File size exceeds maximum limit of ' . (MAX_FILE_SIZE / 1024 / 1024) . 'MB';
    } else {
        // Get file extension
        $file_ext = strtolower(pathinfo($file['name'], PATHINFO_EXTENSION));
        
        // Validate extension
        if (!in_array($file_ext, ALLOWED_EXTENSIONS)) {
            $upload_error = 'File type not allowed. Allowed types: ' . implode(', ', ALLOWED_EXTENSIONS);
        } else {
            // Sanitize filename
            $original_name = basename($file['name']);
            $unique_name = time() . '_' . preg_replace('/[^a-zA-Z0-9._-]/', '_', $original_name);
            $upload_path = UPLOAD_DIR . $unique_name;
            
            // Move uploaded file
            if (move_uploaded_file($file['tmp_name'], $upload_path)) {
                $upload_message = 'File uploaded successfully: ' . htmlspecialchars($original_name);
            } else {
                $upload_error = 'Failed to move uploaded file';
            }
        }
    }
}

// Get list of uploaded files
$uploaded_files = [];
if (is_dir(UPLOAD_DIR)) {
    $files = scandir(UPLOAD_DIR);
    $uploaded_files = array_filter($files, function($file) {
        return $file !== '.' && $file !== '..' && is_file(UPLOAD_DIR . $file);
    });
    // Sort by modification time (newest first)
    usort($uploaded_files, function($a, $b) {
        return filemtime(UPLOAD_DIR . $b) - filemtime(UPLOAD_DIR . $a);
    });
}

// Handle file deletion (if authorized)
if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['delete_file'])) {
    $file_to_delete = basename($_POST['delete_file']);
    $delete_path = UPLOAD_DIR . $file_to_delete;
    
    if (file_exists($delete_path) && is_file($delete_path)) {
        if (unlink($delete_path)) {
            $upload_message = 'File deleted successfully';
        } else {
            $upload_error = 'Failed to delete file';
        }
    }
}

// Get file icon based on extension
function getFileIcon($filename) {
    $ext = strtolower(pathinfo($filename, PATHINFO_EXTENSION));
    $icons = [
        'pdf' => '📄',
        'doc' => '📝',
        'docx' => '📝',
        'xls' => '📊',
        'xlsx' => '📊',
        'jpg' => '🖼️',
        'jpeg' => '🖼️',
        'png' => '🖼️',
        'gif' => '🖼️',
        'txt' => '📄',
        'zip' => '📦'
    ];
    return $icons[$ext] ?? '📁';
}

// Get human-readable file size
function getFileSize($bytes) {
    $units = ['B', 'KB', 'MB', 'GB'];
    $bytes = max($bytes, 0);
    $pow = floor(($bytes ? log($bytes) : 0) / log(1024));
    $pow = min($pow, count($units) - 1);
    $bytes /= (1 << (10 * $pow));
    return round($bytes, 2) . ' ' . $units[$pow];
}

// Get file display type
function getFileDisplayType($filename) {
    $ext = strtolower(pathinfo($filename, PATHINFO_EXTENSION));
    if (in_array($ext, ['jpg', 'jpeg', 'png', 'gif'])) {
        return 'image';
    }
    return 'file';
}
?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>File Upload Manager - Corporate Portal</title>
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
            padding: 20px;
        }

        .container {
            max-width: 1000px;
            margin: 0 auto;
            background: white;
            border-radius: 10px;
            box-shadow: 0 10px 40px rgba(0, 0, 0, 0.2);
            overflow: hidden;
        }

        header {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 40px 20px;
            text-align: center;
        }

        header h1 {
            font-size: 2.5em;
            margin-bottom: 10px;
        }

        header p {
            font-size: 1.1em;
            opacity: 0.9;
        }

        .content {
            padding: 40px;
        }

        .section {
            margin-bottom: 40px;
        }

        .section-title {
            font-size: 1.5em;
            color: #333;
            margin-bottom: 20px;
            border-bottom: 2px solid #667eea;
            padding-bottom: 10px;
        }

        .upload-form {
            background: #f8f9fa;
            padding: 25px;
            border-radius: 8px;
            border: 2px dashed #667eea;
        }

        .form-group {
            margin-bottom: 20px;
        }

        .form-group label {
            display: block;
            margin-bottom: 8px;
            color: #333;
            font-weight: 600;
        }

        .form-group input[type="file"] {
            display: block;
            width: 100%;
            padding: 12px;
            border: 1px solid #ddd;
            border-radius: 5px;
            cursor: pointer;
            background: white;
        }

        .form-group input[type="file"]:hover {
            border-color: #667eea;
            background: #f0f0f0;
        }

        .file-info {
            color: #666;
            font-size: 0.9em;
            margin-top: 8px;
        }

        .btn {
            display: inline-block;
            padding: 12px 30px;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            font-size: 1em;
            font-weight: 600;
            transition: all 0.3s ease;
            text-decoration: none;
        }

        .btn-primary {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
        }

        .btn-primary:hover {
            transform: translateY(-2px);
            box-shadow: 0 5px 15px rgba(102, 126, 234, 0.4);
        }

        .btn-danger {
            background: #e74c3c;
            color: white;
            padding: 8px 15px;
            font-size: 0.9em;
        }

        .btn-danger:hover {
            background: #c0392b;
        }

        .message {
            padding: 15px;
            border-radius: 5px;
            margin-bottom: 20px;
            font-weight: 500;
        }

        .message.success {
            background: #d4edda;
            color: #155724;
            border: 1px solid #c3e6cb;
        }

        .message.error {
            background: #f8d7da;
            color: #721c24;
            border: 1px solid #f5c6cb;
        }

        .files-grid {
            display: grid;
            grid-template-columns: repeat(auto-fill, minmax(250px, 1fr));
            gap: 20px;
            margin-top: 20px;
        }

        .file-card {
            background: white;
            border: 1px solid #ddd;
            border-radius: 8px;
            padding: 20px;
            text-align: center;
            transition: all 0.3s ease;
            box-shadow: 0 2px 8px rgba(0, 0, 0, 0.1);
        }

        .file-card:hover {
            transform: translateY(-5px);
            box-shadow: 0 5px 20px rgba(0, 0, 0, 0.15);
            border-color: #667eea;
        }

        .file-icon {
            font-size: 3em;
            margin-bottom: 10px;
        }

        .file-name {
            word-break: break-word;
            color: #333;
            font-weight: 600;
            margin-bottom: 8px;
            font-size: 0.95em;
        }

        .file-meta {
            color: #666;
            font-size: 0.85em;
            margin-bottom: 15px;
        }

        .file-actions {
            display: flex;
            gap: 10px;
            justify-content: center;
        }

        .file-actions form {
            margin: 0;
        }

        .preview-image {
            width: 100%;
            height: 150px;
            object-fit: cover;
            border-radius: 5px;
            margin-bottom: 10px;
        }

        .empty-state {
            text-align: center;
            padding: 40px;
            color: #666;
        }

        .empty-state-icon {
            font-size: 4em;
            margin-bottom: 20px;
            opacity: 0.5;
        }

        footer {
            background: #f8f9fa;
            padding: 20px;
            text-align: center;
            color: #666;
            border-top: 1px solid #ddd;
        }

        @media (max-width: 768px) {
            header h1 {
                font-size: 1.8em;
            }

            .content {
                padding: 20px;
            }

            .files-grid {
                grid-template-columns: 1fr;
            }

            .file-actions {
                flex-direction: column;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <header>
            <h1>📁 File Upload Manager</h1>
            <p>Corporate Document Management Portal</p>
        </header>

        <div class="content">
            <!-- Upload Section -->
            <div class="section">
                <h2 class="section-title">Upload New File</h2>

                <?php if (!empty($upload_message)): ?>
                    <div class="message success"><?php echo htmlspecialchars($upload_message); ?></div>
                <?php endif; ?>

                <?php if (!empty($upload_error)): ?>
                    <div class="message error"><?php echo htmlspecialchars($upload_error); ?></div>
                <?php endif; ?>

                <form method="POST" enctype="multipart/form-data" class="upload-form">
                    <div class="form-group">
                        <label for="file_upload">Select File to Upload:</label>
                        <input type="file" id="file_upload" name="file_upload" required>
                        <div class="file-info">
                            Maximum file size: <?php echo (MAX_FILE_SIZE / 1024 / 1024); ?>MB<br>
                            Allowed formats: <?php echo implode(', ', ALLOWED_EXTENSIONS); ?>
                        </div>
                    </div>
                    <button type="submit" class="btn btn-primary">Upload File</button>
                </form>
            </div>

            <!-- Files Display Section -->
            <div class="section">
                <h2 class="section-title">Uploaded Files (<?php echo count($uploaded_files); ?>)</h2>

                <?php if (empty($uploaded_files)): ?>
                    <div class="empty-state">
                        <div class="empty-state-icon">📂</div>
                        <p>No files uploaded yet. Upload your first file above.</p>
                    </div>
                <?php else: ?>
                    <div class="files-grid">
                        <?php foreach ($uploaded_files as $file): ?>
                            <?php
                            $file_path = UPLOAD_DIR . $file;
                            $file_size = filesize($file_path);
                            $file_date = date('M d, Y H:i', filemtime($file_path));
                            $display_type = getFileDisplayType($file);
                            $original_name = preg_replace('/^\d+_/', '', $file);
                            ?>
                            <div class="file-card">
                                <?php if ($display_type === 'image'): ?>
                                    <img src="<?php echo htmlspecialchars($file_path); ?>" alt="<?php echo htmlspecialchars($original_name); ?>" class="preview-image">
                                <?php else: ?>
                                    <div class="file-icon"><?php echo getFileIcon($file); ?></div>
                                <?php endif; ?>
                                <div class="file-name" title="<?php echo htmlspecialchars($original_name); ?>">
                                    <?php echo htmlspecialchars($original_name); ?>
                                </div>
                                <div class="file-meta">
                                    <div><?php echo getFileSize($file_size); ?></div>
                                    <div><?php echo $file_date; ?></div>
                                </div>
                                <div class="file-actions">
                                    <a href="<?php echo htmlspecialchars($file_path); ?>" download="<?php echo htmlspecialchars($original_name); ?>" class="btn btn-primary" style="flex: 1;">Download</a>
                                    <form method="POST" style="flex: 1;">
                                        <input type="hidden" name="delete_file" value="<?php echo htmlspecialchars($file); ?>">
                                        <button type="submit" class="btn btn-danger" style="width: 100%; margin: 0;">Delete</button>
                                    </form>
                                </div>
                            </div>
                        <?php endforeach; ?>
                    </div>
                <?php endif; ?>
            </div>
        </div>

        <footer>
            <p>&copy; <?php echo date('Y'); ?> Corporate File Management System. All rights reserved.</p>
        </footer>
    </div>
</body>
</html>