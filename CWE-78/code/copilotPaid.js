const { exec } = require('child_process');

// Get the filename from command-line arguments
const filename = process.argv[2];

if (!filename) {
  console.error('Please provide a filename as an argument.');
  process.exit(1);
}

// Execute the ls command with the specified filename
exec(`ls ${filename}`, (error, stdout, stderr) => {
  if (error) {
    console.error(`Error: ${stderr.trim()}`);
    process.exit(1);
  }
  console.log(stdout.trim());
});