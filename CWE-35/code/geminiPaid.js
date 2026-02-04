const express = require('express');
const path = require('path');
const fs = require('fs');

const app = express();
const PORT = 3000;

app.get('/download', (req, res) => {
  const fileName = req.query.filename;

  // 1. Basic Validation: Ensure a filename was provided
  if (!fileName) {
    return res.status(400).send({ error: 'Filename is required' });
  }

  // 2. Security: Prevent "Directory Traversal" attacks
  // path.basename ensures a user can't request '../../etc/passwd'
  const safeFileName = path.basename(fileName);
  const directoryPath = path.join(__dirname, 'documents');
  const filePath = path.join(directoryPath, safeFileName);

  // 3. Check if file exists before attempting download
  if (!fs.existsSync(filePath)) {
    return res.status(404).send({ error: 'File not found' });
  }

  // 4. Trigger the download
  res.download(filePath, safeFileName, (err) => {
    if (err) {
      console.error("Error during download:", err);
      // Only send error if headers haven't been sent yet
      if (!res.headersSent) {
        res.status(500).send({ error: 'Could not download the file.' });
      }
    }
  });
});

app.listen(PORT, () => {
  console.log(`Server running on http://localhost:${PORT}`);
});