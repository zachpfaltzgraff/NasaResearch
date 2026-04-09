<?php
/**
 * Single-file upload + display app
 * - Saves uploads to ./uploads/
 * - Displays the latest uploaded file on the main page
 * - Basic security controls (allowlist extensions, MIME check, size limit, randomized filename)
 *
 * Deployment:
 *   1) Put this file on your server (e.g., index.php)
 *   2) Create ./uploads/ and ensure it is writable by the web server user
 *      (e.g., chmod 0755 uploads; chown www-data:www-data uploads)
 */

declare(strict_types=1);

error_reporting(E_ALL);
ini_set('display_errors', '0'); // don't leak info on corporate site; log instead
ini_set('log_errors', '1');

$UPLOAD_DIR = __DIR__ . DIRECTORY_SEPARATOR . 'uploads';
$STATE_FILE = $UPLOAD_DIR . DIRECTORY_SEPARATOR . '.latest.json';

$MAX_BYTES = 10 * 1024 * 1024; // 10 MB

// Allowlist by extension (adjust to your needs)
$ALLOWED_EXT = [
  'jpg','jpeg','png','gif','webp',
  'pdf',
  'txt','csv',
  'mp4','webm',
  'mp3','wav',
];

// Some safe, commonly-used MIME types mapped to extensions.
// (MIME sniffing is not perfect, but better than trusting user input.)
$ALLOWED_MIME = [
  'image/jpeg' => ['jpg','jpeg'],
  'image/png'  => ['png'],
  'image/gif'  => ['gif'],
  'image/webp' => ['webp'],
  'application/pdf' => ['pdf'],
  'text/plain' => ['txt','csv'],
  'text/csv'   => ['csv'],
  'video/mp4'  => ['mp4'],
  'video/webm' => ['webm'],
  'audio/mpeg' => ['mp3'],
  'audio/wav'  => ['wav'],
];

// Ensure uploads dir exists
if (!is_dir($UPLOAD_DIR)) {
  // Try to create it. In many corporate environments, you may prefer pre-creating it.
  @mkdir($UPLOAD_DIR, 0755, true);
}

function h(string $s): string {
  return htmlspecialchars($s, ENT_QUOTES | ENT_SUBSTITUTE, 'UTF-8');
}

function read_latest(string $stateFile): ?array {
  if (!is_file($stateFile)) return null;
  $raw = @file_get_contents($stateFile);
  if ($raw === false) return null;
  $data = json_decode($raw, true);
  if (!is_array($data)) return null;
  // expected keys: stored_name, original_name, mime, uploaded_at
  return $data;
}

function write_latest(string $stateFile, array $data): void {
  @file_put_contents($stateFile, json_encode($data, JSON_PRETTY_PRINT | JSON_UNESCAPED_SLASHES), LOCK_EX);
}

function ext_of(string $filename): string {
  $ext = strtolower(pathinfo($filename, PATHINFO_EXTENSION));
  return $ext ?: '';
}

function is_ext_allowed(string $ext, array $allowed): bool {
  return in_array($ext, $allowed, true);
}

function detect_mime(string $path): string {
  if (!function_exists('finfo_open')) return 'application/octet-stream';
  $finfo = finfo_open(FILEINFO_MIME_TYPE);
  if (!$finfo) return 'application/octet-stream';
  $mime = finfo_file($finfo, $path) ?: 'application/octet-stream';
  finfo_close($finfo);
  return $mime;
}

function random_name(string $ext): string {
  // 32 hex chars + extension
  $bytes = random_bytes(16);
  return bin2hex($bytes) . ($ext ? ('.' . $ext) : '');
}

function is_mime_allowed(string $mime, string $ext, array $allowedMime): bool {
  if (!isset($allowedMime[$mime])) return false;
  return in_array($ext, $allowedMime[$mime], true);
}

$status = null;
$error  = null;

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
  if (!is_dir($UPLOAD_DIR) || !is_writable($UPLOAD_DIR)) {
    $error = "Upload directory is missing or not writable: " . $UPLOAD_DIR;
  } elseif (!isset($_FILES['upload']) || !is_array($_FILES['upload'])) {
    $error = "No file received.";
  } else {
    $f = $_FILES['upload'];

    if (($f['error'] ?? UPLOAD_ERR_NO_FILE) !== UPLOAD_ERR_OK) {
      $code = (int)($f['error'] ?? -1);
      $error = match ($code) {
        UPLOAD_ERR_INI_SIZE, UPLOAD_ERR_FORM_SIZE => "File is too large.",
        UPLOAD_ERR_PARTIAL => "File upload was incomplete.",
        UPLOAD_ERR_NO_FILE => "No file selected.",
        default => "Upload failed (error code: $code).",
      };
    } else {
      $tmpPath = (string)($f['tmp_name'] ?? '');
      $origName = (string)($f['name'] ?? 'uploaded_file');
      $size = (int)($f['size'] ?? 0);

      if ($size <= 0) {
        $error = "Empty upload.";
      } elseif ($size > $MAX_BYTES) {
        $error = "File exceeds the maximum allowed size (" . (int)($MAX_BYTES/1024/1024) . " MB).";
      } elseif (!is_uploaded_file($tmpPath)) {
        $error = "Possible file upload attack detected.";
      } else {
        $ext = ext_of($origName);

        if (!is_ext_allowed($ext, $ALLOWED_EXT)) {
          $error = "File type not allowed (extension: " . h($ext ?: 'none') . ").";
        } else {
          $mime = detect_mime($tmpPath);

          if (!is_mime_allowed($mime, $ext, $ALLOWED_MIME)) {
            $error = "File content type not allowed (detected MIME: " . h($mime) . ").";
          } else {
            $stored = random_name($ext);
            $dest = $UPLOAD_DIR . DIRECTORY_SEPARATOR . $stored;

            // Move file into uploads directory
            if (!@move_uploaded_file($tmpPath, $dest)) {
              $error = "Could not save uploaded file.";
            } else {
              // Tighten permissions
              @chmod($dest, 0644);

              // Store state of latest upload
              $latest = [
                'stored_name'   => $stored,
                'original_name' => $origName,
                'mime'          => $mime,
                'uploaded_at'   => gmdate('c'),
              ];
              write_latest($STATE_FILE, $latest);

              $status = "Upload successful.";
            }
          }
        }
      }
    }
  }
}

$latest = read_latest($STATE_FILE);
$latestUrl = null;

if ($latest && isset($latest['stored_name'])) {
  // Build a relative URL; assumes this file is served from a web root and uploads/ is web-accessible
  $latestUrl = 'uploads/' . rawurlencode((string)$latest['stored_name']);
}

function render_preview(?array $latest, ?string $url): string {
  if (!$latest || !$url) {
    return "<p>No file has been uploaded yet.</p>";
  }

  $mime = (string)($latest['mime'] ?? 'application/octet-stream');
  $orig = (string)($latest['original_name'] ?? 'file');
  $when = (string)($latest['uploaded_at'] ?? '');

  $meta = "<p><strong>Latest upload:</strong> " . h($orig) . "<br><small>MIME: " . h($mime) .
          ($when ? " · Uploaded (UTC): " . h($when) : "") . "</small></p>";

  // Inline preview by type
  if (str_starts_with($mime, 'image/')) {
    return $meta . '<img src="' . h($url) . '" alt="' . h($orig) . '" style="max-width:100%;height:auto;border:1px solid #ddd;padding:6px;border-radius:6px;">';
  }

  if ($mime === 'application/pdf') {
    return $meta . '<iframe src="' . h($url) . '" style="width:100%;height:70vh;border:1px solid #ddd;border-radius:6px;"></iframe>';
  }

  if (str_starts_with($mime, 'text/')) {
    // Safely display small-ish text files; otherwise provide link
    $path = __DIR__ . DIRECTORY_SEPARATOR . 'uploads' . DIRECTORY_SEPARATOR . (string)$latest['stored_name'];
    $size = is_file($path) ? filesize($path) : 0;
    if ($size !== false && $size <= 200 * 1024) { // 200KB
      $content = @file_get_contents($path);
      if ($content === false) {
        return $meta . '<p><a href="' . h($url) . '" download>Download file</a></p>';
      }
      return $meta . '<pre style="white-space:pre-wrap;word-break:break-word;background:#0b1020;color:#e6e6e6;padding:12px;border-radius:6px;border:1px solid #222;">' . h($content) . '</pre>';
    }
    return $meta . '<p>Text file is large; <a href="' . h($url) . '" download>download it</a>.</p>';
  }

  if (str_starts_with($mime, 'video/')) {
    return $meta . '<video controls style="max-width:100%;border:1px solid #ddd;border-radius:6px;"><source src="' . h($url) . '" type="' . h($mime) . '"></video>';
  }

  if (str_starts_with($mime, 'audio/')) {
    return $meta . '<audio controls style="width:100%;"><source src="' . h($url) . '" type="' . h($mime) . '"></audio>';
  }

  return $meta . '<p><a href="' . h($url) . '" download>Download file</a></p>';
}

?><!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Upload & Display</title>
  <style>
    body { font-family: system-ui, -apple-system, Segoe UI, Roboto, Arial, sans-serif; margin: 24px; color: #111; }
    .wrap { max-width: 980px; margin: 0 auto; }
    .card { border: 1px solid #e5e7eb; border-radius: 10px; padding: 16px; background: #fff; }
    .row { display: grid; grid-template-columns: 1fr; gap: 16px; }
    .status { padding: 10px 12px; border-radius: 8px; margin-bottom: 12px; }
    .ok { background: #ecfdf5; border: 1px solid #10b98133; color: #065f46; }
    .err { background: #fef2f2; border: 1px solid #ef444433; color: #7f1d1d; }
    label { display:block; font-weight: 600; margin-bottom: 8px; }
    input[type=file] { width: 100%; }
    button { padding: 10px 14px; border: 1px solid #111; background: #111; color: #fff; border-radius: 8px; cursor: pointer; }
    button:hover { opacity: 0.9; }
    small.muted { color: #6b7280; }
    .footer { margin-top: 18px; color: #6b7280; font-size: 12px; }
    code { background: #f3f4f6; padding: 2px 6px; border-radius: 6px; }
  </style>
</head>
<body>
  <div class="wrap">
    <h1>Upload & Display</h1>

    <?php if ($status): ?>
      <div class="status ok"><?= h($status) ?></div>
    <?php endif; ?>
    <?php if ($error): ?>
      <div class="status err"><?= h($error) ?></div>
    <?php endif; ?>

    <div class="row">
      <div class="card">
        <form method="post" enctype="multipart/form-data">
          <label for="upload">Upload a file</label>
          <input id="upload" name="upload" type="file" required>
          <p><small class="muted">
            Max size: <?= (int)($MAX_BYTES/1024/1024) ?> MB.
            Allowed: <?= h(implode(', ', $ALLOWED_EXT)) ?>.
          </small></p>
          <button type="submit">Upload</button>
        </form>
      </div>

      <div class="card">
        <h2>Preview</h2>
        <?= render_preview($latest, $latestUrl) ?>
      </div>
    </div>

    <div class="footer">
      Files are stored under <code>./uploads/</code>. Ensure the directory exists and is writable.
    </div>
  </div>
</body>
</html>