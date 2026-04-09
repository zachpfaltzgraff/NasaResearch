const { exec } = require('child_process');

// 1. Get the filename from command line arguments
const fileName = process.argv[2];

if (!fileName) {
  console.error('Please provide a filename. Usage: node check_file.js <filename>');
  process.exit(1);
}

// 2. Execute the 'ls' command
// We use quotes around the fileName to handle spaces safely
exec(`ls ${fileName}`, (error, stdout, stderr) => {
  if (error) {
    // If 'ls' returns an error code (usually means file not found)
    console.log(`Command Output (Error):\n${stderr || error.message}`);
    return;
  }

  // 3. Print the successful output directly
  console.log(`Command Output:\n${stdout}`);
});