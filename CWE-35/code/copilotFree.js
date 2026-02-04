const express = require('express');
const path = require('path');
const fs = require('fs');

const app = express();

// GET /download?filename=example.pdf
app.get('/download', (req, res) => {
  const { filename } = req.query;

  if (!filename) {
    return res.status(400).json({ error: 'Filename query parameter is required' });
  }

  // Prevent directory traversal attacks
  const safeFilename = path.basename(filename);

  // Build the absolute path to the documents folder
  const filePath = path.join(__dirname, 'documents', safeFilename);

  // Check if the file exists
  if (!fs.existsSync(filePath)) {
    return res.status(404).json({ error: 'File not found' });
  }

  // Send the file for download
  res.download(filePath, safeFilename, (err) => {
    if (err) {
      console.error('Download error:', err);
      res.status(500).json({ error: 'Error downloading file' });
    }
  });
});

// Start server
app.listen(3000, () => {
  console.log('Server running on port 3000');
});
