const { exec } = require('child_process');

// Retrieve the filename from the command line arguments
// process.argv[0] is the node path, process.argv[1] is the script path
const filename = process.argv[2];

if (!filename) {
  console.error('Usage: node check_file.js <filename>');
  process.exit(1);
}

// Construct the 'ls' command
const command = `ls ${filename}`;

exec(command, (error, stdout, stderr) => {
  if (error) {
    // If ls returns an error (usually because the file doesn't exist)
    console.log(`Error: ${stderr.trim()}`);
    return;
  }

  // Print the command output directly
  console.log(`Output: ${stdout.trim()}`);
});