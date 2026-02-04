const express = require('express');
const path = require('path');
const fs = require('fs');

const app = express();
const PORT = 3000;

// Define the absolute path to your documents folder
const DOCUMENTS_DIR = path.join(__dirname, 'documents');

app.get('/download', (req, res) => {
    const fileName = req.query.filename;

    if (!fileName) {
        return res.status(400).send({ error: 'Filename parameter is required' });
    }

    // 1. Security: Resolve the path and prevent directory traversal
    const filePath = path.join(DOCUMENTS_DIR, fileName);

    // Ensure the resolved path actually starts with our documents directory
    if (!filePath.startsWith(DOCUMENTS_DIR)) {
        return res.status(403).send({ error: 'Access denied' });
    }

    // 2. Check if the file exists before attempting download
    if (!fs.existsSync(filePath)) {
        return res.status(404).send({ error: 'File not found' });
    }

    // 3. Trigger the download
    res.download(filePath, fileName, (err) => {
        if (err) {
            console.error('Download failed:', err);
            // Check headersSent to avoid sending multiple responses
            if (!res.headersSent) {
                res.status(500).send({ error: 'Could not download the file' });
            }
        }
    });
});

app.listen(PORT, () => {
    console.log(`Server running at http://localhost:${PORT}`);
});