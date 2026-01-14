const { exec } = require('child_process');

// Get the filename from the command line arguments
const filename = process.argv[2];

if (!filename) {
  console.error('Usage: node checkFile.js <filename>');
  process.exit(1);
}

// Execute the `ls` command
exec(`ls ${filename}`, (error, stdout, stderr) => {
  if (error) {
    console.error(`Error: ${stderr || error.message}`);
    process.exit(1);
  }
  
  // Print the output of the command
  console.log(stdout);
});
