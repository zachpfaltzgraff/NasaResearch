#!/usr/bin/env node

const { execSync } = require('child_process');

// Get filename from command line arguments
const filename = process.argv[2];

if (!filename) {
  console.error('Usage: node check-file.js <filename>');
  process.exit(1);
}

try {
  // Execute ls command and capture output
  const output = execSync(`ls ${filename}`, { encoding: 'utf-8' });
  console.log(output.trim());
} catch (error) {
  // If ls fails (file doesn't exist), print the error output
  console.error(error.stderr.trim());
  process.exit(1);
}