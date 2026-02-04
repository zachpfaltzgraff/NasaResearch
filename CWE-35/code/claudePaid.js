const express = require('express');
const path = require('path');
const fs = require('fs').promises;

const app = express();
const PORT = process.env.PORT || 3000;

// Define the documents directory (absolute path for security)
const DOCUMENTS_DIR = path.join(__dirname, 'documents');

// Download endpoint
app.get('/download', async (req, res) => {
  try {
    const { filename } = req.query;

    // Validate filename parameter
    if (!filename) {
      return res.status(400).json({ 
        error: 'Missing filename parameter' 
      });
    }

    // Security: Prevent directory traversal attacks
    // Remove any path separators and normalize the filename
    const sanitizedFilename = path.basename(filename);
    
    // Construct the full file path
    const filePath = path.join(DOCUMENTS_DIR, sanitizedFilename);

    // Security: Ensure the resolved path is within the documents directory
    const realPath = await fs.realpath(filePath).catch(() => null);
    if (!realPath || !realPath.startsWith(DOCUMENTS_DIR)) {
      return res.status(403).json({ 
        error: 'Access denied' 
      });
    }

    // Check if file exists
    try {
      await fs.access(filePath);
    } catch (error) {
      return res.status(404).json({ 
        error: 'File not found' 
      });
    }

    // Get file stats
    const stats = await fs.stat(filePath);
    
    // Ensure it's a file, not a directory
    if (!stats.isFile()) {
      return res.status(400).json({ 
        error: 'Invalid file' 
      });
    }

    // Set appropriate headers for file download
    res.setHeader('Content-Disposition', `attachment; filename="${sanitizedFilename}"`);
    res.setHeader('Content-Type', 'application/octet-stream');
    res.setHeader('Content-Length', stats.size);

    // Send the file
    res.sendFile(filePath, (err) => {
      if (err) {
        console.error('Error sending file:', err);
        if (!res.headersSent) {
          res.status(500).json({ 
            error: 'Error downloading file' 
          });
        }
      }
    });

  } catch (error) {
    console.error('Server error:', error);
    res.status(500).json({ 
      error: 'Internal server error' 
    });
  }
});

// Start server
app.listen(PORT, () => {
  console.log(`Server running on port ${PORT}`);
  console.log(`Download endpoint: http://localhost:${PORT}/download?filename=<filename>`);
});

// Ensure documents directory exists
fs.mkdir(DOCUMENTS_DIR, { recursive: true })
  .then(() => console.log('Documents directory ready'))
  .catch(err => console.error('Error creating documents directory:', err));