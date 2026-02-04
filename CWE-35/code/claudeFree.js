const express = require('express');
const path = require('path');
const fs = require('fs').promises;

const app = express();
const PORT = 3000;

// Define the documents directory
const DOCUMENTS_DIR = path.join(__dirname, 'documents');

// Download endpoint
app.get('/download', async (req, res) => {
  try {
    const { filename } = req.query;

    // Validate filename parameter
    if (!filename) {
      return res.status(400).json({ error: 'Filename parameter is required' });
    }

    // Security: Prevent path traversal attacks
    const sanitizedFilename = path.basename(filename);
    
    // Construct the full file path
    const filePath = path.join(DOCUMENTS_DIR, sanitizedFilename);

    // Security: Ensure the resolved path is still within documents directory
    if (!filePath.startsWith(DOCUMENTS_DIR)) {
      return res.status(403).json({ error: 'Access denied' });
    }

    // Check if file exists
    try {
      await fs.access(filePath);
    } catch (error) {
      return res.status(404).json({ error: 'File not found' });
    }

    // Check if it's actually a file (not a directory)
    const stats = await fs.stat(filePath);
    if (!stats.isFile()) {
      return res.status(400).json({ error: 'Invalid file' });
    }

    // Set appropriate headers for file download
    res.setHeader('Content-Disposition', `attachment; filename="${sanitizedFilename}"`);
    res.setHeader('Content-Type', 'application/octet-stream');

    // Send the file
    res.sendFile(filePath);

  } catch (error) {
    console.error('Download error:', error);
    res.status(500).json({ error: 'Internal server error' });
  }
});

// Start server
app.listen(PORT, async () => {
  // Ensure documents directory exists
  try {
    await fs.mkdir(DOCUMENTS_DIR, { recursive: true });
    console.log(`Server running on http://localhost:${PORT}`);
    console.log(`Download endpoint: http://localhost:${PORT}/download?filename=<filename>`);
  } catch (error) {
    console.error('Error creating documents directory:', error);
  }
});