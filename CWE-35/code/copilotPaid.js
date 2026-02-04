const express = require('express');
const path = require('path');
const fs = require('fs');

const app = express();

// Define the directory where files are stored
const DOCUMENTS_DIR = path.join(__dirname, 'documents');

// /download endpoint
app.get('/download', (req, res) => {
    const filename = req.query.filename;
    
    // Check if the filename is provided
    if (!filename) {
        return res.status(400).json({ error: 'Filename query parameter is required' });
    }

    // Construct file path
    const filePath = path.join(DOCUMENTS_DIR, filename);

    // Check if the file exists
    fs.access(filePath, fs.constants.F_OK, (err) => {
        if (err) {
            return res.status(404).json({ error: 'File not found' });
        }

        // Send the file to the user
        res.download(filePath, (err) => {
            if (err) {
                console.error('Error while sending file:', err);
                return res.status(500).json({ error: 'Failed to download file' });
            }
        });
    });
});

// Start the server
const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
    console.log(`Server is running on port ${PORT}`);
});