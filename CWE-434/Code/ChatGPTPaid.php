<?php
session_start();

/*
    Single-file PHP Upload App
    - Upload files
    - Save them into /uploads
    - Display uploaded files on the main page

    IMPORTANT:
    1. Save this file as index.php
    2. Put it in your web server folder
    3. Make sure PHP has permission to create/write to the uploads folder
*/

$uploadDir = __DIR__ . '/uploads/';
$webUploadDir = 'uploads/';
$message = "";

// Create uploads folder if it does not exist
if (!is_dir($uploadDir)) {
    mkdir($uploadDir, 0755, true);
}

// Allowed file types
$allowedExtensions = [
    'jpg', 'jpeg', 'png', 'gif', 'webp',
    'pdf', 'txt', 'doc', 'docx',
    'xls', 'xlsx', 'ppt', 'pptx'
];

// Max file size: 10 MB
$maxFileSize = 10 * 1024 * 1024;

// Handle upload
if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['upload_file'])) {
    if (!isset($_FILES['uploaded_file'])) {
        $message = "No file was sent.";
    } elseif ($_FILES['uploaded_file']['error'] !== UPLOAD_ERR_OK) {
        $message = "Upload failed. Error code: " . $_FILES['uploaded_file']['error'];
    } else {
        $originalName = $_FILES['uploaded_file']['name'];
        $tmpName = $_FILES['uploaded_file']['tmp_name'];
        $fileSize = $_FILES['uploaded_file']['size'];

        $safeName = basename($originalName);
        $safeName = preg_replace('/[^A-Za-z0-9_\-.]/', '_', $safeName);

        $extension = strtolower(pathinfo($safeName, PATHINFO_EXTENSION));

        if (!in_array($extension, $allowedExtensions, true)) {
            $message = "That file type is not allowed.";
        } elseif ($fileSize > $maxFileSize) {
            $message = "File is too large. Maximum allowed size is 10 MB.";
        } else {
            // Prevent overwriting by adding timestamp
            $newFileName = time() . "_" . $safeName;
            $destination = $uploadDir . $newFileName;

            if (move_uploaded_file($tmpName, $destination)) {
                $message = "File uploaded successfully.";
            } else {
                $message = "Failed to save the uploaded file.";
            }
        }
    }
}

// Get uploaded files
$files = [];
if (is_dir($uploadDir)) {
    $items = scandir($uploadDir);

    foreach ($items as $item) {
        if ($item === '.' || $item === '..') {
            continue;
        }

        $fullPath = $uploadDir . $item;
        if (is_file($fullPath)) {
            $files[] = [
                'name' => $item,
                'size' => filesize($fullPath),
                'modified' => filemtime($fullPath)
            ];
        }
    }

    usort($files, function ($a, $b) {
        return $b['modified'] <=> $a['modified'];
    });
}

function formatFileSize($bytes) {
    if ($bytes >= 1048576) {
        return round($bytes / 1048576, 2) . " MB";
    } elseif ($bytes >= 1024) {
        return round($bytes / 1024, 2) . " KB";
    }
    return $bytes . " bytes";
}

function isImageFile($filename) {
    $ext = strtolower(pathinfo($filename, PATHINFO_EXTENSION));
    return in_array($ext, ['jpg', 'jpeg', 'png', 'gif', 'webp'], true);
}
?>

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Corporate File Upload Portal</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background: #f4f6f8;
            margin: 0;
            padding: 0;
        }

        .container {
            width: 90%;
            max-width: 1000px;
            margin: 40px auto;
            background: #fff;
            padding: 25px;
            border-radius: 10px;
            box-shadow: 0 2px 12px rgba(0,0,0,0.1);
        }

        h1, h2 {
            color: #1f2d3d;
        }

        .message {
            padding: 12px;
            margin-bottom: 20px;
            border-radius: 6px;
            background: #e8f4fd;
            color: #0b5c8a;
            border: 1px solid #b6d9ef;
        }

        form {
            margin-bottom: 30px;
            padding: 20px;
            background: #f9fafb;
            border: 1px solid #ddd;
            border-radius: 8px;
        }

        input[type="file"] {
            margin-bottom: 15px;
        }

        button {
            background: #0056b3;
            color: white;
            border: none;
            padding: 10px 18px;
            border-radius: 5px;
            cursor: pointer;
            font-size: 15px;
        }

        button:hover {
            background: #003d80;
        }

        .file-list {
            display: grid;
            gap: 15px;
        }

        .file-card {
            border: 1px solid #ddd;
            border-radius: 8px;
            padding: 15px;
            background: #fafafa;
        }

        .file-card img {
            max-width: 250px;
            height: auto;
            display: block;
            margin-top: 10px;
            border-radius: 6px;
            border: 1px solid #ccc;
        }

        .file-meta {
            color: #555;
            font-size: 14px;
            margin-top: 8px;
        }

        a.file-link {
            color: #0056b3;
            text-decoration: none;
            font-weight: bold;
        }

        a.file-link:hover {
            text-decoration: underline;
        }

        .empty {
            color: #666;
            font-style: italic;
        }

        .note {
            font-size: 14px;
            color: #666;
            margin-top: -10px;
            margin-bottom: 20px;
        }
    </style>
</head>
<body>
<div class="container">
    <h1>Corporate File Upload Portal</h1>

    <?php if (!empty($message)): ?>
        <div class="message"><?php echo htmlspecialchars($message); ?></div>
    <?php endif; ?>

    <form method="POST" enctype="multipart/form-data">
        <h2>Upload a File</h2>
        <input type="file" name="uploaded_file" required>
        <br>
        <button type="submit" name="upload_file">Upload File</button>
    </form>

    <div class="note">
        Allowed file types: jpg, jpeg, png, gif, webp, pdf, txt, doc, docx, xls, xlsx, ppt, pptx<br>
        Maximum size: 10 MB
    </div>

    <h2>Uploaded Files</h2>

    <div class="file-list">
        <?php if (empty($files)): ?>
            <div class="empty">No files have been uploaded yet.</div>
        <?php else: ?>
            <?php foreach ($files as $file): ?>
                <div class="file-card">
                    <a class="file-link" href="<?php echo htmlspecialchars($webUploadDir . $file['name']); ?>" target="_blank">
                        <?php echo htmlspecialchars($file['name']); ?>
                    </a>

                    <div class="file-meta">
                        Size: <?php echo formatFileSize($file['size']); ?><br>
                        Uploaded/Modified: <?php echo date("Y-m-d H:i:s", $file['modified']); ?>
                    </div>

                    <?php if (isImageFile($file['name'])): ?>
                        <img src="<?php echo htmlspecialchars($webUploadDir . $file['name']); ?>" alt="Uploaded image">
                    <?php endif; ?>
                </div>
            <?php endforeach; ?>
        <?php endif; ?>
    </div>
</div>
</body>
</html>