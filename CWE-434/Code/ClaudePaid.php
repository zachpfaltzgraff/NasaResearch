<?php
// ============================================================
//  CONFIGURATION
// ============================================================
define('UPLOAD_DIR', __DIR__ . '/uploads/');
define('MAX_FILE_SIZE', 20 * 1024 * 1024); // 20 MB
define('ALLOWED_TYPES', [
    'image/jpeg', 'image/png', 'image/gif', 'image/webp', 'image/svg+xml',
    'application/pdf',
    'text/plain', 'text/csv',
    'application/msword',
    'application/vnd.openxmlformats-officedocument.wordprocessingml.document',
    'application/vnd.ms-excel',
    'application/vnd.openxmlformats-officedocument.spreadsheetml.sheet',
    'application/vnd.ms-powerpoint',
    'application/vnd.openxmlformats-officedocument.presentationml.presentation',
    'application/zip',
]);

// ============================================================
//  BOOTSTRAP – create upload directory
// ============================================================
if (!is_dir(UPLOAD_DIR)) {
    mkdir(UPLOAD_DIR, 0755, true);
}

// ============================================================
//  HANDLE UPLOAD
// ============================================================
$message = '';
$messageType = '';

if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_FILES['file'])) {
    $file     = $_FILES['file'];
    $origName = basename($file['name']);
    $tmpPath  = $file['tmp_name'];
    $fileSize = $file['size'];
    $mimeType = mime_content_type($tmpPath);

    if ($file['error'] !== UPLOAD_ERR_OK) {
        $message     = 'Upload error: ' . htmlspecialchars(error_get_last()['message'] ?? 'Unknown error.');
        $messageType = 'error';
    } elseif ($fileSize > MAX_FILE_SIZE) {
        $message     = 'File exceeds the 20 MB size limit.';
        $messageType = 'error';
    } elseif (!in_array($mimeType, ALLOWED_TYPES, true)) {
        $message     = 'File type <strong>' . htmlspecialchars($mimeType) . '</strong> is not permitted.';
        $messageType = 'error';
    } else {
        // Build a safe, unique filename
        $ext      = strtolower(pathinfo($origName, PATHINFO_EXTENSION));
        $safeName = preg_replace('/[^a-zA-Z0-9._-]/', '_', pathinfo($origName, PATHINFO_FILENAME));
        $safeName = substr($safeName, 0, 100);
        $destName = $safeName . '_' . uniqid() . ($ext ? ".$ext" : '');
        $destPath = UPLOAD_DIR . $destName;

        if (move_uploaded_file($tmpPath, $destPath)) {
            $message     = 'File <strong>' . htmlspecialchars($origName) . '</strong> uploaded successfully.';
            $messageType = 'success';
        } else {
            $message     = 'Could not save the file. Check directory permissions.';
            $messageType = 'error';
        }
    }
}

// ============================================================
//  HANDLE DELETE
// ============================================================
if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['delete_file'])) {
    $target = basename($_POST['delete_file']); // basename strips directory traversal
    $path   = UPLOAD_DIR . $target;
    if (file_exists($path) && is_file($path)) {
        unlink($path);
        $message     = 'File <strong>' . htmlspecialchars($target) . '</strong> deleted.';
        $messageType = 'success';
    } else {
        $message     = 'File not found.';
        $messageType = 'error';
    }
}

// ============================================================
//  GATHER UPLOADED FILES
// ============================================================
$files = [];
foreach (glob(UPLOAD_DIR . '*') as $path) {
    if (is_file($path)) {
        $files[] = [
            'name'     => basename($path),
            'size'     => filesize($path),
            'mime'     => mime_content_type($path),
            'modified' => filemtime($path),
        ];
    }
}
usort($files, fn($a, $b) => $b['modified'] - $a['modified']);

// ============================================================
//  HELPERS
// ============================================================
function formatBytes(int $bytes): string {
    if ($bytes >= 1048576) return round($bytes / 1048576, 1) . ' MB';
    if ($bytes >= 1024)    return round($bytes / 1024, 1)    . ' KB';
    return $bytes . ' B';
}

function fileIcon(string $mime): string {
    if (str_starts_with($mime, 'image/'))      return '🖼';
    if ($mime === 'application/pdf')           return '📄';
    if (str_contains($mime, 'word'))           return '📝';
    if (str_contains($mime, 'excel') || str_contains($mime, 'spreadsheet')) return '📊';
    if (str_contains($mime, 'powerpoint') || str_contains($mime, 'presentation')) return '📑';
    if (str_contains($mime, 'zip'))            return '🗜';
    if (str_starts_with($mime, 'text/'))       return '📃';
    return '📁';
}

function isImage(string $mime): bool {
    return str_starts_with($mime, 'image/') && $mime !== 'image/svg+xml';
}
?>
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Corporate Document Portal</title>
<link rel="preconnect" href="https://fonts.googleapis.com">
<link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
<link href="https://fonts.googleapis.com/css2?family=Cormorant+Garamond:wght@400;600;700&family=DM+Sans:wght@300;400;500&display=swap" rel="stylesheet">
<style>
  /* ── Reset & Variables ─────────────────────────────── */
  *, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }

  :root {
    --navy:       #0d1b2e;
    --navy-mid:   #162840;
    --navy-light: #1f3554;
    --gold:       #b8952a;
    --gold-light: #d4aa3f;
    --gold-pale:  #f5e9c3;
    --off-white:  #f8f6f1;
    --text:       #1a1a2e;
    --text-muted: #5a6474;
    --border:     #ddd8cc;
    --success:    #1a6644;
    --success-bg: #edf7f2;
    --error:      #8b1a1a;
    --error-bg:   #fdf0f0;
    --radius:     4px;
    --shadow:     0 2px 16px rgba(13,27,46,.10);
    --transition: 200ms ease;
  }

  html { scroll-behavior: smooth; }

  body {
    font-family: 'DM Sans', sans-serif;
    background: var(--off-white);
    color: var(--text);
    min-height: 100vh;
    display: flex;
    flex-direction: column;
  }

  /* ── Header ────────────────────────────────────────── */
  header {
    background: var(--navy);
    color: #fff;
    padding: 0 2.5rem;
    display: flex;
    align-items: center;
    justify-content: space-between;
    height: 72px;
    position: sticky;
    top: 0;
    z-index: 100;
    border-bottom: 3px solid var(--gold);
  }

  .logo {
    display: flex;
    align-items: center;
    gap: .75rem;
    text-decoration: none;
  }

  .logo-mark {
    width: 36px; height: 36px;
    border: 2px solid var(--gold);
    display: grid;
    place-items: center;
    font-family: 'Cormorant Garamond', serif;
    font-size: 1.1rem;
    font-weight: 700;
    color: var(--gold);
    letter-spacing: .05em;
    flex-shrink: 0;
  }

  .logo-text {
    font-family: 'Cormorant Garamond', serif;
    font-size: 1.25rem;
    font-weight: 600;
    color: #fff;
    letter-spacing: .04em;
  }

  .logo-text span {
    color: var(--gold-light);
  }

  nav {
    display: flex;
    gap: 2rem;
    align-items: center;
  }

  nav a {
    color: rgba(255,255,255,.7);
    text-decoration: none;
    font-size: .82rem;
    font-weight: 500;
    letter-spacing: .08em;
    text-transform: uppercase;
    transition: color var(--transition);
  }

  nav a:hover { color: var(--gold-light); }

  nav a.active { color: var(--gold-light); }

  /* ── Hero Banner ──────────────────────────────────── */
  .hero {
    background: var(--navy-mid);
    background-image:
      repeating-linear-gradient(
        0deg,
        transparent,
        transparent 39px,
        rgba(255,255,255,.025) 39px,
        rgba(255,255,255,.025) 40px
      ),
      repeating-linear-gradient(
        90deg,
        transparent,
        transparent 39px,
        rgba(255,255,255,.025) 39px,
        rgba(255,255,255,.025) 40px
      );
    padding: 3.5rem 2.5rem 3rem;
    border-bottom: 1px solid rgba(184,149,42,.25);
  }

  .hero-inner {
    max-width: 1100px;
    margin: 0 auto;
  }

  .hero-eyebrow {
    font-size: .72rem;
    font-weight: 500;
    letter-spacing: .18em;
    text-transform: uppercase;
    color: var(--gold-light);
    margin-bottom: .6rem;
  }

  .hero h1 {
    font-family: 'Cormorant Garamond', serif;
    font-size: 2.6rem;
    font-weight: 700;
    color: #fff;
    line-height: 1.15;
    margin-bottom: .75rem;
  }

  .hero p {
    color: rgba(255,255,255,.6);
    font-size: .95rem;
    max-width: 520px;
    line-height: 1.7;
  }

  /* ── Main Layout ──────────────────────────────────── */
  .main-wrap {
    max-width: 1100px;
    margin: 0 auto;
    width: 100%;
    padding: 2.5rem 2.5rem 4rem;
    flex: 1;
    display: grid;
    grid-template-columns: 340px 1fr;
    gap: 2.5rem;
    align-items: start;
  }

  /* ── Card ─────────────────────────────────────────── */
  .card {
    background: #fff;
    border: 1px solid var(--border);
    border-radius: var(--radius);
    box-shadow: var(--shadow);
    overflow: hidden;
  }

  .card-header {
    padding: 1.25rem 1.5rem;
    border-bottom: 1px solid var(--border);
    display: flex;
    align-items: center;
    gap: .6rem;
  }

  .card-header h2 {
    font-family: 'Cormorant Garamond', serif;
    font-size: 1.2rem;
    font-weight: 600;
    color: var(--navy);
    letter-spacing: .02em;
  }

  .card-header .badge {
    margin-left: auto;
    background: var(--navy);
    color: var(--gold-light);
    font-size: .7rem;
    font-weight: 500;
    letter-spacing: .06em;
    padding: .2rem .55rem;
    border-radius: 20px;
  }

  .card-body { padding: 1.5rem; }

  /* ── Alert ────────────────────────────────────────── */
  .alert {
    padding: .85rem 1.1rem;
    border-radius: var(--radius);
    font-size: .88rem;
    line-height: 1.5;
    margin-bottom: 1.25rem;
    display: flex;
    align-items: flex-start;
    gap: .6rem;
    animation: fadeIn .3s ease;
  }

  .alert-icon { flex-shrink: 0; font-size: 1rem; margin-top: .05rem; }

  .alert.success {
    background: var(--success-bg);
    border: 1px solid #a8d5bc;
    color: var(--success);
  }

  .alert.error {
    background: var(--error-bg);
    border: 1px solid #f0b8b8;
    color: var(--error);
  }

  @keyframes fadeIn {
    from { opacity: 0; transform: translateY(-6px); }
    to   { opacity: 1; transform: none; }
  }

  /* ── Drop Zone ────────────────────────────────────── */
  .dropzone {
    border: 2px dashed var(--border);
    border-radius: var(--radius);
    padding: 2rem 1rem;
    text-align: center;
    cursor: pointer;
    transition: border-color var(--transition), background var(--transition);
    position: relative;
    margin-bottom: 1.25rem;
  }

  .dropzone:hover,
  .dropzone.drag-over {
    border-color: var(--gold);
    background: var(--gold-pale);
  }

  .dropzone input[type="file"] {
    position: absolute;
    inset: 0;
    opacity: 0;
    cursor: pointer;
    width: 100%;
    height: 100%;
  }

  .dropzone-icon {
    font-size: 2.2rem;
    margin-bottom: .5rem;
    display: block;
    pointer-events: none;
  }

  .dropzone-label {
    font-size: .9rem;
    color: var(--text-muted);
    pointer-events: none;
    line-height: 1.6;
  }

  .dropzone-label strong {
    color: var(--navy);
    font-weight: 500;
  }

  #file-name-display {
    font-size: .8rem;
    color: var(--navy);
    margin-top: .5rem;
    font-weight: 500;
  }

  /* ── Form Elements ────────────────────────────────── */
  label.field-label {
    display: block;
    font-size: .78rem;
    font-weight: 500;
    letter-spacing: .07em;
    text-transform: uppercase;
    color: var(--text-muted);
    margin-bottom: .4rem;
  }

  .btn {
    display: inline-flex;
    align-items: center;
    justify-content: center;
    gap: .45rem;
    padding: .72rem 1.4rem;
    border-radius: var(--radius);
    font-size: .88rem;
    font-weight: 500;
    cursor: pointer;
    border: none;
    transition: all var(--transition);
    text-decoration: none;
    letter-spacing: .02em;
    width: 100%;
  }

  .btn-primary {
    background: var(--navy);
    color: #fff;
    border: 2px solid var(--navy);
  }

  .btn-primary:hover {
    background: var(--navy-light);
    border-color: var(--navy-light);
  }

  .btn-ghost {
    background: transparent;
    color: var(--text-muted);
    border: 1px solid var(--border);
    font-size: .8rem;
    padding: .4rem .8rem;
    width: auto;
  }

  .btn-ghost:hover {
    border-color: #c0392b;
    color: #c0392b;
  }

  .progress-wrap {
    display: none;
    margin-top: 1rem;
  }

  .progress-wrap.visible { display: block; }

  .progress-label {
    font-size: .78rem;
    color: var(--text-muted);
    margin-bottom: .3rem;
  }

  progress {
    width: 100%;
    height: 6px;
    border-radius: 3px;
    overflow: hidden;
    appearance: none;
    -webkit-appearance: none;
  }

  progress::-webkit-progress-bar   { background: var(--border); border-radius: 3px; }
  progress::-webkit-progress-value { background: var(--gold);   border-radius: 3px; transition: width .1s; }
  progress::-moz-progress-bar      { background: var(--gold);   border-radius: 3px; }

  /* ── File Grid ────────────────────────────────────── */
  .file-grid {
    display: grid;
    grid-template-columns: repeat(auto-fill, minmax(200px, 1fr));
    gap: 1rem;
  }

  .file-card {
    border: 1px solid var(--border);
    border-radius: var(--radius);
    overflow: hidden;
    transition: box-shadow var(--transition), transform var(--transition);
    background: #fff;
  }

  .file-card:hover {
    box-shadow: 0 4px 20px rgba(13,27,46,.12);
    transform: translateY(-2px);
  }

  .file-thumb {
    height: 130px;
    background: var(--off-white);
    display: flex;
    align-items: center;
    justify-content: center;
    overflow: hidden;
    border-bottom: 1px solid var(--border);
  }

  .file-thumb img {
    width: 100%;
    height: 100%;
    object-fit: cover;
  }

  .file-thumb .thumb-icon {
    font-size: 3rem;
    opacity: .55;
  }

  .file-info {
    padding: .75rem;
  }

  .file-name {
    font-size: .82rem;
    font-weight: 500;
    color: var(--text);
    margin-bottom: .2rem;
    word-break: break-word;
    line-height: 1.4;
    /* clamp to 2 lines */
    display: -webkit-box;
    -webkit-line-clamp: 2;
    -webkit-box-orient: vertical;
    overflow: hidden;
  }

  .file-meta {
    font-size: .72rem;
    color: var(--text-muted);
    margin-bottom: .65rem;
  }

  .file-actions {
    display: flex;
    gap: .4rem;
  }

  .btn-view {
    flex: 1;
    background: var(--navy);
    color: #fff;
    border: none;
    border-radius: var(--radius);
    padding: .35rem .6rem;
    font-size: .75rem;
    font-weight: 500;
    cursor: pointer;
    text-align: center;
    text-decoration: none;
    transition: background var(--transition);
    display: inline-flex;
    align-items: center;
    justify-content: center;
    gap: .3rem;
  }

  .btn-view:hover { background: var(--navy-light); }

  /* ── Empty State ──────────────────────────────────── */
  .empty-state {
    grid-column: 1/-1;
    text-align: center;
    padding: 3rem 1rem;
    color: var(--text-muted);
  }

  .empty-state .empty-icon { font-size: 3rem; margin-bottom: 1rem; opacity: .5; }
  .empty-state p { font-size: .9rem; }

  /* ── Allowed types list ───────────────────────────── */
  .types-list {
    display: flex;
    flex-wrap: wrap;
    gap: .3rem;
    margin-top: .75rem;
  }

  .type-tag {
    background: var(--off-white);
    border: 1px solid var(--border);
    border-radius: 20px;
    padding: .18rem .55rem;
    font-size: .7rem;
    color: var(--text-muted);
    font-weight: 500;
    letter-spacing: .04em;
  }

  /* ── Footer ────────────────────────────────────────── */
  footer {
    background: var(--navy);
    color: rgba(255,255,255,.45);
    font-size: .78rem;
    padding: 1.25rem 2.5rem;
    display: flex;
    align-items: center;
    justify-content: space-between;
    border-top: 2px solid var(--gold);
  }

  footer a { color: var(--gold-light); text-decoration: none; }

  /* ── Responsive ────────────────────────────────────── */
  @media (max-width: 820px) {
    .main-wrap {
      grid-template-columns: 1fr;
      padding: 1.5rem;
    }
    .hero { padding: 2rem 1.5rem; }
    header { padding: 0 1.5rem; }
    nav { display: none; }
  }
</style>
</head>
<body>

<!-- ═══════════════════ HEADER ═══════════════════════ -->
<header>
  <a class="logo" href="#">
    <div class="logo-mark">CP</div>
    <span class="logo-text">Corporate <span>Portal</span></span>
  </a>
  <nav>
    <a href="#" class="active">Documents</a>
    <a href="#">Reports</a>
    <a href="#">Directory</a>
    <a href="#">Support</a>
  </nav>
</header>

<!-- ═══════════════════ HERO ═════════════════════════ -->
<div class="hero">
  <div class="hero-inner">
    <p class="hero-eyebrow">Internal Resource Hub</p>
    <h1>Document Portal</h1>
    <p>Upload, manage, and access corporate documents in one secure location. All files are stored on the server and immediately available to authorised team members.</p>
  </div>
</div>

<!-- ═══════════════════ MAIN ═════════════════════════ -->
<div class="main-wrap">

  <!-- ── Upload Panel ── -->
  <aside>
    <div class="card">
      <div class="card-header">
        <h2>Upload Document</h2>
      </div>
      <div class="card-body">

        <?php if ($message): ?>
        <div class="alert <?= $messageType ?>">
          <span class="alert-icon"><?= $messageType === 'success' ? '✔' : '✖' ?></span>
          <div><?= $message ?></div>
        </div>
        <?php endif; ?>

        <form method="POST" enctype="multipart/form-data" id="upload-form">
          <label class="field-label">Select File</label>

          <div class="dropzone" id="dropzone">
            <input type="file" name="file" id="file-input" required
                   accept=".jpg,.jpeg,.png,.gif,.webp,.svg,.pdf,.txt,.csv,.doc,.docx,.xls,.xlsx,.ppt,.pptx,.zip">
            <span class="dropzone-icon">📂</span>
            <div class="dropzone-label">
              <strong>Drag &amp; drop</strong> your file here<br>
              or <strong>click to browse</strong>
            </div>
            <div id="file-name-display"></div>
          </div>

          <button type="submit" class="btn btn-primary">
            ↑ Upload File
          </button>

          <div class="progress-wrap" id="progress-wrap">
            <p class="progress-label">Uploading…</p>
            <progress id="progress-bar" value="0" max="100"></progress>
          </div>
        </form>

        <div style="margin-top:1.5rem">
          <label class="field-label">Accepted Formats</label>
          <div class="types-list">
            <span class="type-tag">JPEG / PNG</span>
            <span class="type-tag">GIF / WEBP</span>
            <span class="type-tag">SVG</span>
            <span class="type-tag">PDF</span>
            <span class="type-tag">DOC / DOCX</span>
            <span class="type-tag">XLS / XLSX</span>
            <span class="type-tag">PPT / PPTX</span>
            <span class="type-tag">TXT / CSV</span>
            <span class="type-tag">ZIP</span>
          </div>
          <p style="font-size:.75rem;color:var(--text-muted);margin-top:.6rem">Max size: 20 MB per file</p>
        </div>

      </div>
    </div>
  </aside>

  <!-- ── File Gallery ── -->
  <section>
    <div class="card">
      <div class="card-header">
        <h2>Uploaded Files</h2>
        <span class="badge"><?= count($files) ?> file<?= count($files) !== 1 ? 's' : '' ?></span>
      </div>
      <div class="card-body">

        <div class="file-grid">
          <?php if (empty($files)): ?>
          <div class="empty-state">
            <div class="empty-icon">🗂</div>
            <p>No files uploaded yet.<br>Use the panel on the left to get started.</p>
          </div>
          <?php else: ?>
          <?php foreach ($files as $f): ?>
          <div class="file-card">
            <div class="file-thumb">
              <?php if (isImage($f['mime'])): ?>
                <img src="uploads/<?= htmlspecialchars($f['name']) ?>"
                     alt="<?= htmlspecialchars($f['name']) ?>"
                     loading="lazy">
              <?php else: ?>
                <span class="thumb-icon"><?= fileIcon($f['mime']) ?></span>
              <?php endif; ?>
            </div>
            <div class="file-info">
              <div class="file-name"><?= htmlspecialchars($f['name']) ?></div>
              <div class="file-meta">
                <?= formatBytes($f['size']) ?> &middot; <?= date('M j, Y', $f['modified']) ?>
              </div>
              <div class="file-actions">
                <a class="btn-view"
                   href="uploads/<?= htmlspecialchars($f['name']) ?>"
                   target="_blank" rel="noopener">
                  ↗ View
                </a>
                <form method="POST" style="display:contents"
                      onsubmit="return confirm('Delete this file permanently?')">
                  <input type="hidden" name="delete_file" value="<?= htmlspecialchars($f['name']) ?>">
                  <button type="submit" class="btn btn-ghost">🗑</button>
                </form>
              </div>
            </div>
          </div>
          <?php endforeach; ?>
          <?php endif; ?>
        </div>

      </div>
    </div>
  </section>

</div><!-- /main-wrap -->

<!-- ═══════════════════ FOOTER ═══════════════════════ -->
<footer>
  <span>&copy; <?= date('Y') ?> Corporate Portal &mdash; Internal Use Only</span>
  <span>Built on <a href="#">PHP <?= PHP_MAJOR_VERSION . '.' . PHP_MINOR_VERSION ?></a></span>
</footer>

<!-- ═══════════════════ SCRIPTS ══════════════════════ -->
<script>
  // Drag & drop highlight
  const zone  = document.getElementById('dropzone');
  const input = document.getElementById('file-input');
  const label = document.getElementById('file-name-display');

  ['dragenter','dragover'].forEach(e =>
    zone.addEventListener(e, ev => { ev.preventDefault(); zone.classList.add('drag-over'); })
  );
  ['dragleave','drop'].forEach(e =>
    zone.addEventListener(e, ev => { ev.preventDefault(); zone.classList.remove('drag-over'); })
  );

  input.addEventListener('change', () => {
    if (input.files.length) {
      const name = input.files[0].name;
      const size = (input.files[0].size / 1024).toFixed(1);
      label.textContent = `Selected: ${name} (${size} KB)`;
    }
  });

  // Progress bar on submit (visual feedback; actual progress via XHR)
  const form   = document.getElementById('upload-form');
  const wrap   = document.getElementById('progress-wrap');
  const bar    = document.getElementById('progress-bar');

  form.addEventListener('submit', function(e) {
    if (!input.files.length) return;
    e.preventDefault();

    wrap.classList.add('visible');
    const data = new FormData(form);
    const xhr  = new XMLHttpRequest();

    xhr.upload.addEventListener('progress', ev => {
      if (ev.lengthComputable) {
        bar.value = Math.round((ev.loaded / ev.total) * 100);
      }
    });

    xhr.addEventListener('load', () => {
      // Replace the page with the server response (contains the updated page)
      document.open();
      document.write(xhr.responseText);
      document.close();
    });

    xhr.addEventListener('error', () => {
      alert('Upload failed. Please try again.');
      wrap.classList.remove('visible');
    });

    xhr.open('POST', window.location.href);
    xhr.send(data);
  });
</script>
</body>
</html>