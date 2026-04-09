#!/usr/bin/env node
const { spawn } = require('child_process');

const [, , filename] = process.argv;

if (!filename) {
  console.error('Usage: node check-file.js <filename>');
  process.exit(2);
}

// Use spawn without a shell to avoid injection and stream output directly to the parent process.
const child = spawn('ls', ['-ld', filename], { stdio: 'inherit' });

child.on('error', (err) => {
  console.error('Failed to start ls:', err.message);
  process.exit(1);
});

child.on('exit', (code, signal) => {
  if (signal) {
    console.error(`ls terminated with signal ${signal}`);
    process.exit(1);
  }
  // Propagate ls exit code (0 if file exists, non-zero if not)
  process.exit(code);
});