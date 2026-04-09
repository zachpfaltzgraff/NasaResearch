#!/usr/bin/env node

const { exec } = require("child_process");

const filename = process.argv[2];

if (!filename) {
  console.error("Usage: node checkFile.js <filename>");
  process.exit(1);
}

exec(`ls ${filename}`, (error, stdout, stderr) => {
  if (stdout) {
    process.stdout.write(stdout);
  }
  if (stderr) {
    process.stderr.write(stderr);
  }
  if (error) {
    process.exit(error.code ?? 1);
  }
});
