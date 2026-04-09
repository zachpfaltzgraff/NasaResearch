<?php
// ============================================================
//  CONFIGURATION
// ============================================================
define('UPLOAD_DIR',  'uploads/');
define('MAX_SIZE',    20 * 1024 * 1024); // 20 MB
define('SITE_NAME',   'CorpDocs');
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
//  BOOTSTRAP: ensure upload directory exists
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
    $size     = $file['size'];
    $mimeType = mime_content_type($tmpPath);

    if ($file['error'] !== UPLOAD_ERR_OK) {
        $message     = 'Upload failed. Please try again.';
        $messageType = 'error';
    } elseif ($size > MAX_SIZE) {
        $message     = 'File exceeds the 20 MB size limit.';
        $messageType = 'error';
    } elseif (!in_array($mimeType, ALLOWED_TYPES)) {
        $message     = 'File type not permitted: ' . htmlspecialchars($mimeType);
        $messageType = 'error';
    } else {
        // Sanitise filename and avoid collisions
        $safeName  = preg_replace('/[^a-zA-Z0-9.\-_]/', '_', $origName);
        $safeName  = date('Ymd_His') . '_' . $safeName;
        $destPath  = UPLOAD_DIR . $safeName;

        if (move_uploaded_file($tmpPath, $destPath)) {
            $message     = 'File uploaded successfully: ' . htmlspecialchars($safeName);
            $messageType = 'success';
        } else {
            $message     = 'Could not save file. Check server permissions.';
            $messageType = 'error';
        }
    }
}

// ============================================================
//  HANDLE DELETE
// ============================================================
if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['delete_file'])) {
    $deleteTarget = basename($_POST['delete_file']);
    $deletePath   = UPLOAD_DIR . $deleteTarget;
    if (file_exists($deletePath) && is_file($deletePath)) {
        unlink($deletePath);
        $message     = 'File deleted: ' . htmlspecialchars($deleteTarget);
        $messageType = 'success';
    }
}

// ============================================================
//  FETCH UPLOADED FILES
// ============================================================
$files = [];
foreach (glob(UPLOAD_DIR . '*') as $path) {
    if (is_file($path)) {
        $files[] = [
            'name'     => basename($path),
            'size'     => filesize($path),
            'modified' => filemtime($path),
            'mime'     => mime_content_type($path),
            'path'     => $path,
        ];
    }
}
usort($files, fn($a, $b) => $b['modified'] - $a['modified']);

// ============================================================
//  HELPERS
// ============================================================
function humanSize(int $bytes): string {
    if ($bytes >= 1048576) return round($bytes / 1048576, 1) . ' MB';
    if ($bytes >= 1024)    return round($bytes / 1024, 1)    . ' KB';
    return $bytes . ' B';
}

function fileIcon(string $mime): string {
    if (str_starts_with($mime, 'image/'))      return '🖼';
    if ($mime === 'application/pdf')            return '📄';
    if (str_contains($mime, 'word'))            return '📝';
    if (str_contains($mime, 'excel') || str_contains($mime, 'spreadsheet')) return '📊';
    if (str_contains($mime, 'powerpoint') || str_contains($mime, 'presentation')) return '📑';
    if ($mime === 'text/csv')                   return '📋';
    if (str_starts_with($mime, 'text/'))        return '📃';
    if ($mime === 'application/zip')            return '🗜';
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
<title><?= SITE_NAME ?> — Document Center</title>
<link rel="preconnect" href="https://fonts.googleapis.com">
<link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
<link href="https://fonts.googleapis.com/css2?family=Playfair+Display:wght@600;700&family=DM+Sans:wght@300;400;500;600&display=swap" rel="stylesheet">
<style>
/* ── CSS Reset + Variables ─────────────────────────────── */
*, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }

:root {
    --ink:       #0f1117;
    --ink-mid:   #3c4255;
    --ink-soft:  #6b7080;
    --border:    #dde1ea;
    --surface:   #f5f6f9;
    --white:     #ffffff;
    --gold:      #b8922a;
    --gold-lt:   #d4a83a;
    --gold-bg:   #fdf8ee;
    --danger:    #c0392b;
    --danger-bg: #fef2f1;
    --success:   #1a7f4b;
    --success-bg:#f0faf5;
    --radius:    10px;
    --shadow-sm: 0 2px 8px rgba(15,17,23,.07);
    --shadow-md: 0 6px 24px rgba(15,17,23,.10);
    --transition: .22s ease;
}

/* ── Base ──────────────────────────────────────────────── */
body {
    font-family: 'DM Sans', sans-serif;
    background: var(--surface);
    color: var(--ink);
    min-height: 100vh;
    font-size: 15px;
    line-height: 1.6;
}

/* ── Header ────────────────────────────────────────────── */
header {
    background: var(--ink);
    color: var(--white);
    padding: 0 40px;
    display: flex;
    align-items: center;
    justify-content: space-between;
    height: 68px;
    position: sticky;
    top: 0;
    z-index: 100;
    box-shadow: 0 2px 16px rgba(0,0,0,.25);
}

.logo {
    display: flex;
    align-items: center;
    gap: 14px;
}

.logo-mark {
    width: 36px;
    height: 36px;
    background: var(--gold);
    border-radius: 8px;
    display: grid;
    place-items: center;
    font-size: 18px;
    line-height: 1;
    flex-shrink: 0;
}

.logo-text {
    font-family: 'Playfair Display', serif;
    font-size: 1.35rem;
    letter-spacing: -.01em;
}

.logo-text span { color: var(--gold-lt); }

.header-nav {
    display: flex;
    gap: 28px;
    font-size: .875rem;
    font-weight: 500;
    color: rgba(255,255,255,.6);
}

.header-nav a {
    color: inherit;
    text-decoration: none;
    transition: color var(--transition);
}

.header-nav a:hover { color: var(--white); }
.header-nav a.active { color: var(--gold-lt); }

/* ── Banner / Hero ─────────────────────────────────────── */
.hero {
    background: linear-gradient(135deg, #1a1e2e 0%, #232844 100%);
    color: var(--white);
    padding: 56px 40px 48px;
    position: relative;
    overflow: hidden;
}

.hero::before {
    content: '';
    position: absolute;
    inset: 0;
    background:
        radial-gradient(ellipse 60% 120% at 90% 50%, rgba(184,146,42,.13) 0%, transparent 70%),
        radial-gradient(ellipse 40% 80% at 10% 80%,  rgba(184,146,42,.07) 0%, transparent 60%);
    pointer-events: none;
}

.hero-inner {
    max-width: 860px;
    margin: 0 auto;
    position: relative;
}

.hero-eyebrow {
    font-size: .78rem;
    font-weight: 600;
    letter-spacing: .12em;
    text-transform: uppercase;
    color: var(--gold-lt);
    margin-bottom: 14px;
}

.hero h1 {
    font-family: 'Playfair Display', serif;
    font-size: clamp(1.8rem, 4vw, 2.8rem);
    line-height: 1.15;
    margin-bottom: 14px;
}

.hero p {
    font-size: 1rem;
    color: rgba(255,255,255,.65);
    max-width: 500px;
    font-weight: 300;
}

/* ── Main Layout ───────────────────────────────────────── */
main {
    max-width: 1100px;
    margin: 0 auto;
    padding: 48px 40px 80px;
}

/* ── Toast / Alert ─────────────────────────────────────── */
.alert {
    display: flex;
    align-items: center;
    gap: 12px;
    padding: 14px 20px;
    border-radius: var(--radius);
    font-size: .9rem;
    font-weight: 500;
    margin-bottom: 32px;
    animation: slideIn .3s ease;
}

@keyframes slideIn {
    from { opacity: 0; transform: translateY(-8px); }
    to   { opacity: 1; transform: translateY(0); }
}

.alert-success {
    background: var(--success-bg);
    color: var(--success);
    border: 1px solid #a7e3c4;
}

.alert-error {
    background: var(--danger-bg);
    color: var(--danger);
    border: 1px solid #f5c2be;
}

.alert-icon { font-size: 1.1rem; flex-shrink: 0; }

/* ── Upload Card ───────────────────────────────────────── */
.card {
    background: var(--white);
    border-radius: 14px;
    border: 1px solid var(--border);
    box-shadow: var(--shadow-sm);
    margin-bottom: 40px;
    overflow: hidden;
}

.card-header {
    padding: 20px 28px;
    border-bottom: 1px solid var(--border);
    display: flex;
    align-items: center;
    gap: 12px;
}

.card-header h2 {
    font-family: 'Playfair Display', serif;
    font-size: 1.2rem;
    font-weight: 600;
}

.card-icon {
    width: 36px;
    height: 36px;
    border-radius: 8px;
    background: var(--gold-bg);
    display: grid;
    place-items: center;
    font-size: 1rem;
    flex-shrink: 0;
}

.card-body { padding: 28px; }

/* ── Drop Zone ─────────────────────────────────────────── */
.drop-zone {
    border: 2px dashed var(--border);
    border-radius: 12px;
    padding: 52px 28px;
    text-align: center;
    cursor: pointer;
    transition: border-color var(--transition), background var(--transition);
    position: relative;
}

.drop-zone:hover,
.drop-zone.dragging {
    border-color: var(--gold);
    background: var(--gold-bg);
}

.drop-zone input[type="file"] {
    position: absolute;
    inset: 0;
    opacity: 0;
    cursor: pointer;
    width: 100%;
    height: 100%;
}

.dz-icon {
    font-size: 2.8rem;
    margin-bottom: 14px;
    display: block;
}

.dz-headline {
    font-weight: 600;
    font-size: 1rem;
    margin-bottom: 6px;
}

.dz-sub {
    font-size: .85rem;
    color: var(--ink-soft);
}

.dz-sub strong { color: var(--gold); }

/* ── File Meta Strip ───────────────────────────────────── */
#file-meta {
    display: none;
    margin-top: 18px;
    padding: 12px 16px;
    background: var(--surface);
    border-radius: 8px;
    font-size: .87rem;
    color: var(--ink-mid);
    gap: 8px;
    align-items: center;
}

/* ── Submit Button ─────────────────────────────────────── */
.btn-row {
    margin-top: 22px;
    display: flex;
    gap: 12px;
    justify-content: flex-end;
}

.btn {
    padding: 11px 28px;
    border-radius: 8px;
    font-family: 'DM Sans', sans-serif;
    font-size: .9rem;
    font-weight: 600;
    cursor: pointer;
    border: none;
    transition: opacity var(--transition), transform var(--transition), box-shadow var(--transition);
}

.btn:active { transform: scale(.97); }

.btn-primary {
    background: var(--gold);
    color: var(--white);
    box-shadow: 0 4px 14px rgba(184,146,42,.35);
}

.btn-primary:hover {
    opacity: .9;
    box-shadow: 0 6px 20px rgba(184,146,42,.45);
}

.btn-ghost {
    background: transparent;
    border: 1.5px solid var(--border);
    color: var(--ink-mid);
}

.btn-ghost:hover { border-color: var(--ink-mid); }

/* ── File Grid ─────────────────────────────────────────── */
.section-header {
    display: flex;
    align-items: baseline;
    gap: 12px;
    margin-bottom: 22px;
}

.section-header h2 {
    font-family: 'Playfair Display', serif;
    font-size: 1.35rem;
}

.badge {
    background: var(--ink);
    color: var(--white);
    font-size: .72rem;
    font-weight: 700;
    letter-spacing: .05em;
    padding: 3px 10px;
    border-radius: 999px;
}

.file-grid {
    display: grid;
    grid-template-columns: repeat(auto-fill, minmax(260px, 1fr));
    gap: 18px;
}

.file-card {
    background: var(--white);
    border: 1px solid var(--border);
    border-radius: var(--radius);
    overflow: hidden;
    box-shadow: var(--shadow-sm);
    transition: box-shadow var(--transition), transform var(--transition);
    display: flex;
    flex-direction: column;
}

.file-card:hover {
    box-shadow: var(--shadow-md);
    transform: translateY(-2px);
}

/* Thumbnail */
.file-thumb {
    height: 160px;
    background: var(--surface);
    display: grid;
    place-items: center;
    overflow: hidden;
    position: relative;
    border-bottom: 1px solid var(--border);
}

.file-thumb img {
    width: 100%;
    height: 100%;
    object-fit: cover;
}

.file-thumb .thumb-icon {
    font-size: 3rem;
    opacity: .7;
}

.file-thumb .mime-badge {
    position: absolute;
    bottom: 8px;
    right: 8px;
    background: rgba(15,17,23,.65);
    color: var(--white);
    font-size: .68rem;
    font-weight: 700;
    letter-spacing: .05em;
    text-transform: uppercase;
    padding: 3px 8px;
    border-radius: 4px;
    backdrop-filter: blur(4px);
}

/* File Info */
.file-info {
    padding: 14px 16px;
    flex: 1;
}

.file-name {
    font-weight: 600;
    font-size: .88rem;
    color: var(--ink);
    word-break: break-all;
    line-height: 1.4;
    margin-bottom: 4px;
}

.file-date {
    font-size: .78rem;
    color: var(--ink-soft);
}

.file-size {
    font-size: .78rem;
    color: var(--ink-soft);
}

/* Actions */
.file-actions {
    display: flex;
    border-top: 1px solid var(--border);
}

.file-actions a,
.file-actions button {
    flex: 1;
    padding: 10px;
    text-align: center;
    font-size: .8rem;
    font-weight: 600;
    font-family: 'DM Sans', sans-serif;
    text-decoration: none;
    border: none;
    cursor: pointer;
    transition: background var(--transition), color var(--transition);
}

.btn-view {
    background: transparent;
    color: var(--gold);
    border-right: 1px solid var(--border) !important;
}

.btn-view:hover { background: var(--gold-bg); }

.btn-delete {
    background: transparent;
    color: var(--ink-soft);
}

.btn-delete:hover { background: var(--danger-bg); color: var(--danger); }

/* ── Empty State ───────────────────────────────────────── */
.empty-state {
    text-align: center;
    padding: 64px 20px;
    color: var(--ink-soft);
}

.empty-state .e-icon { font-size: 3rem; margin-bottom: 14px; display: block; opacity: .5; }
.empty-state h3 { font-size: 1rem; font-weight: 600; margin-bottom: 6px; color: var(--ink-mid); }
.empty-state p  { font-size: .88rem; }

/* ── Footer ────────────────────────────────────────────── */
footer {
    background: var(--ink);
    color: rgba(255,255,255,.45);
    text-align: center;
    font-size: .8rem;
    padding: 22px;
    margin-top: auto;
}

footer strong { color: rgba(255,255,255,.7); }

/* ── Responsive ────────────────────────────────────────── */
@media (max-width: 640px) {
    header { padding: 0 20px; }
    .hero  { padding: 40px 20px 36px; }
    main   { padding: 32px 20px 60px; }
    .header-nav { display: none; }
    .file-grid { grid-template-columns: 1fr; }
}
</style>
</head>
<body>

<!-- ══ HEADER ══════════════════════════════════════════════════ -->
<header>
    <div class="logo">
        <div class="logo-mark">📁</div>
        <span class="logo-text"><?= SITE_NAME ?><span>.</span></span>
    </div>
    <nav class="header-nav">
        <a href="#" class="active">Document Center</a>
        <a href="#">Intranet</a>
        <a href="#">Support</a>
    </nav>
</header>

<!-- ══ HERO ════════════════════════════════════════════════════ -->
<div class="hero">
    <div class="hero-inner">
        <p class="hero-eyebrow">Document Center</p>
        <h1>Corporate File Repository</h1>
        <p>Upload, manage, and share internal documents securely. All files are stored on the corporate server.</p>
    </div>
</div>

<!-- ══ MAIN ════════════════════════════════════════════════════ -->
<main>

    <?php if ($message): ?>
    <div class="alert alert-<?= $messageType ?>">
        <span class="alert-icon"><?= $messageType === 'success' ? '✅' : '⚠️' ?></span>
        <?= htmlspecialchars($message) ?>
    </div>
    <?php endif; ?>

    <!-- Upload Card -->
    <div class="card">
        <div class="card-header">
            <div class="card-icon">⬆️</div>
            <h2>Upload a File</h2>
        </div>
        <div class="card-body">
            <form method="POST" enctype="multipart/form-data" id="upload-form">
                <div class="drop-zone" id="drop-zone">
                    <input type="file" name="file" id="file-input" accept="*/*">
                    <span class="dz-icon">☁️</span>
                    <p class="dz-headline">Drag &amp; drop a file here, or click to browse</p>
                    <p class="dz-sub">Max size: <strong>20 MB</strong> &nbsp;·&nbsp; Images, PDFs, Office docs, ZIP, CSV, TXT</p>
                </div>
                <div id="file-meta" style="display:flex;">
                    <span id="file-meta-icon" style="font-size:1.3rem;"></span>
                    <span id="file-meta-name" style="font-weight:600;"></span>
                    <span id="file-meta-size" style="margin-left:auto;color:var(--ink-soft);"></span>
                </div>
                <div class="btn-row">
                    <button type="button" class="btn btn-ghost" onclick="clearFile()">Clear</button>
                    <button type="submit" class="btn btn-primary">Upload File →</button>
                </div>
            </form>
        </div>
    </div>

    <!-- File Library -->
    <div class="section-header">
        <h2>Uploaded Files</h2>
        <span class="badge"><?= count($files) ?></span>
    </div>

    <?php if (empty($files)): ?>
    <div class="empty-state">
        <span class="e-icon">🗂</span>
        <h3>No files yet</h3>
        <p>Uploaded files will appear here once submitted.</p>
    </div>
    <?php else: ?>
    <div class="file-grid">
        <?php foreach ($files as $f): ?>
        <div class="file-card">
            <div class="file-thumb">
                <?php if (isImage($f['mime'])): ?>
                    <img src="<?= htmlspecialchars($f['path']) ?>" alt="<?= htmlspecialchars($f['name']) ?>" loading="lazy">
                <?php else: ?>
                    <span class="thumb-icon"><?= fileIcon($f['mime']) ?></span>
                <?php endif; ?>
                <span class="mime-badge">
                    <?= strtoupper(pathinfo($f['name'], PATHINFO_EXTENSION) ?: 'FILE') ?>
                </span>
            </div>
            <div class="file-info">
                <div class="file-name"><?= htmlspecialchars($f['name']) ?></div>
                <div class="file-date">📅 <?= date('M j, Y — g:i A', $f['modified']) ?></div>
                <div class="file-size">📦 <?= humanSize($f['size']) ?></div>
            </div>
            <div class="file-actions">
                <a class="btn-view" href="<?= htmlspecialchars($f['path']) ?>" target="_blank">View / Download</a>
                <form method="POST" style="flex:1;display:flex;" onsubmit="return confirm('Delete this file?')">
                    <input type="hidden" name="delete_file" value="<?= htmlspecialchars($f['name']) ?>">
                    <button type="submit" class="btn-delete">Delete</button>
                </form>
            </div>
        </div>
        <?php endforeach; ?>
    </div>
    <?php endif; ?>

</main>

<!-- ══ FOOTER ══════════════════════════════════════════════════ -->
<footer>
    &copy; <?= date('Y') ?> <strong><?= SITE_NAME ?></strong> — Internal use only. All uploads are logged and subject to corporate policy.
</footer>

<!-- ══ SCRIPTS ═════════════════════════════════════════════════ -->
<script>
const input    = document.getElementById('file-input');
const dropZone = document.getElementById('drop-zone');
const meta     = document.getElementById('file-meta');
const metaIcon = document.getElementById('file-meta-icon');
const metaName = document.getElementById('file-meta-name');
const metaSize = document.getElementById('file-meta-size');

function humanSize(bytes) {
    if (bytes >= 1048576) return (bytes / 1048576).toFixed(1) + ' MB';
    if (bytes >= 1024)    return (bytes / 1024).toFixed(1) + ' KB';
    return bytes + ' B';
}

function showMeta(file) {
    const ext = file.name.split('.').pop().toLowerCase();
    const imgs = ['jpg','jpeg','png','gif','webp','svg'];
    metaIcon.textContent = imgs.includes(ext) ? '🖼' : '📄';
    metaName.textContent = file.name;
    metaSize.textContent = humanSize(file.size);
    meta.style.display = 'flex';
}

function clearFile() {
    input.value = '';
    meta.style.display = 'none';
}

input.addEventListener('change', () => {
    if (input.files[0]) showMeta(input.files[0]);
});

// Drag & drop visual feedback
['dragenter','dragover'].forEach(evt =>
    dropZone.addEventListener(evt, e => { e.preventDefault(); dropZone.classList.add('dragging'); })
);
['dragleave','drop'].forEach(evt =>
    dropZone.addEventListener(evt, e => { e.preventDefault(); dropZone.classList.remove('dragging'); })
);
dropZone.addEventListener('drop', e => {
    const dt = e.dataTransfer;
    if (dt.files.length) {
        // Transfer files to input
        const dataTransfer = new DataTransfer();
        dataTransfer.items.add(dt.files[0]);
        input.files = dataTransfer.files;
        showMeta(dt.files[0]);
    }
});
</script>
</body>
</html>