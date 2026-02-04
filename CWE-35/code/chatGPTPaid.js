// routes/download.js
const express = require("express");
const path = require("path");
const fs = require("fs");

const router = express.Router();

const DOCUMENTS_DIR = path.resolve(__dirname, "..", "documents");

// Optional: restrict what a "filename" can look like (recommended).
// This allows letters, numbers, spaces, underscores, dashes, dots, and parentheses.
const SAFE_NAME = /^[\w\-. ()]+$/;

router.get("/download", (req, res) => {
  const filename = req.query.filename;

  if (typeof filename !== "string" || filename.trim() === "") {
    return res.status(400).json({ error: "Missing filename query parameter." });
  }

  // Basic filename validation (helps avoid odd edge cases).
  if (!SAFE_NAME.test(filename)) {
    return res.status(400).json({ error: "Invalid filename." });
  }

  // Resolve the requested file path against the documents directory.
  // This is the key step to prevent ../ traversal.
  const filePath = path.resolve(DOCUMENTS_DIR, filename);

  // Ensure the resolved path is still within DOCUMENTS_DIR.
  // (Prevents things like filename = "../../secrets.txt")
  if (!filePath.startsWith(DOCUMENTS_DIR + path.sep)) {
    return res.status(400).json({ error: "Invalid filename path." });
  }

  // Check file exists and is a regular file
  let stat;
  try {
    stat = fs.statSync(filePath);
  } catch {
    return res.status(404).json({ error: "File not found." });
  }

  if (!stat.isFile()) {
    return res.status(404).json({ error: "File not found." });
  }

  // res.download sets appropriate headers and prompts a download in browsers.
  return res.download(filePath, filename, (err) => {
    if (err) {
      // Avoid leaking server paths/details
      if (!res.headersSent) res.status(500).json({ error: "Download failed." });
    }
  });
});

module.exports = router;
