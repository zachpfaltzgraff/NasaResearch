const express = require('express');
const path = require('path');
const fs = require('fs');

const app = express();
const PORT = 3000;

// Absolute path to documents folder
const DOCUMENTS_DIR = path.join(__dirname, 'documents');

app.get('/download', (req, res) => {
  const filename = req.query.filename;

  if (!filename) {
    return res.status(400).json({ error: 'filename query parameter is required' });
  }

  // Normalize filename to prevent path traversal
  const safeFilename = path.basename(filename);
  const filePath = path.join(DOCUMENTS_DIR, safeFilename);

  // Ensure file exists inside documents directory
  if (!filePath.startsWith(DOCUMENTS_DIR)) {
    return res.status(403).json({ error: 'Invalid file path' });
  }

  if (!fs.existsSync(filePath)) {
    return res.status(404).json({ error: 'File not found' });
  }

  // Send file for download
  res.download(filePath, safeFilename, (err) => {
    if (err) {
      console.error('Download error:', err);
      if (!res.headersSent) {
        res.status(500).json({ error: 'Error downloading file' });
      }
    }
  });
});

app.listen(PORT, () => {
  console.log(`Server running on http://localhost:${PORT}`);
});
