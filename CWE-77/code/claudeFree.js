#!/usr/bin/env node

const { exec } = require('child_process');

// Get filename from command line arguments
const filename = process.argv[2];

if (!filename) {
  console.error('Usage: node script.js <filename>');
  process.exit(1);
}

// Execute ls command
exec(`ls ${filename}`, (error, stdout, stderr) => {
  if (error) {
    // If ls fails, the file doesn't exist or there's an error
    console.error(stderr);
    process.exit(1);
  }
  
  // Print the command output
  console.log(stdout);
});