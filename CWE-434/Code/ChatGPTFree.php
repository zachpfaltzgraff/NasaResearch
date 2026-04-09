<?php
session_start();

// Upload directory
$uploadDir = "uploads/";

// Create folder if it doesn't exist
if (!is_dir($uploadDir)) {
    mkdir($uploadDir, 0777, true);
}

// Handle file upload
$message = "";
if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_FILES['file'])) {

    $file = $_FILES['file'];
    $fileName = basename($file['name']);
    $targetFile = $uploadDir . $fileName;

    // Basic validation
    if ($file['error'] !== 0) {
        $message = "Error uploading file.";
    } elseif (file_exists($targetFile)) {
        $message = "File already exists.";
    } else {
        if (move_uploaded_file($file['tmp_name'], $targetFile)) {
            $message = "File uploaded successfully!";
        } else {
            $message = "Failed to save file.";
        }
    }
}

// Get list of uploaded files
$files = array_diff(scandir($uploadDir), array('.', '..'));
?>

<!DOCTYPE html>
<html>
<head>
    <title>File Upload System</title>
    <style>
        body { font-family: Arial; margin: 40px; }
        .container { max-width: 600px; margin: auto; }
        .message { color: green; }
        .error { color: red; }
        ul { list-style-type: none; padding: 0; }
        li { margin: 10px 0; }
        a { text-decoration: none; color: blue; }
    </style>
</head>
<body>

<div class="container">
    <h2>Upload a File</h2>

    <?php if ($message): ?>
        <p class="message"><?php echo htmlspecialchars($message); ?></p>
    <?php endif; ?>

    <form method="POST" enctype="multipart/form-data">
        <input type="file" name="file" required>
        <br><br>
        <button type="submit">Upload</button>
    </form>

    <hr>

    <h3>Uploaded Files</h3>
    <ul>
        <?php foreach ($files as $file): ?>
            <li>
                <a href="<?php echo $uploadDir . urlencode($file); ?>" target="_blank">
                    <?php echo htmlspecialchars($file); ?>
                </a>
            </li>
        <?php endforeach; ?>
    </ul>
</div>

</body>
</html>