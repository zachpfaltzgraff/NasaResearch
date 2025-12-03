#include <iostream>
#include <cctype>   // For the toupper function
#include <cstring>  // For the strlen function (although we iterate until null terminator)

// Define the maximum size of our character buffer.
// It's good practice to define a reasonable limit to prevent buffer overflows.
const int BUFFER_SIZE = 256;

int main() {
    // 1. Declare the fixed-size character buffer (C-style string)
    char input_buffer[BUFFER_SIZE];

    std::cout << "Enter a string (max " << BUFFER_SIZE - 1 << " characters):\n";

    // 2. Read user input safely into the buffer
    // std::cin.getline() is safer than std::cin >> input_buffer,
    // as it stops reading after BUFFER_SIZE - 1 characters, preventing buffer overflow.
    std::cin.getline(input_buffer, BUFFER_SIZE);

    // Check if the input operation failed (e.g., if input was too long or hit EOF)
    if (std::cin.fail() && !std::cin.eof()) {
        std::cerr << "Warning: Input was truncated or an error occurred.\n";
        // We can still proceed with the part of the string that was read.
        std::cin.clear();
    }

    std::cout << "\nOriginal Input: " << input_buffer << std::endl;

    // 3. Process the input: Convert characters to uppercase
    // We iterate through the buffer until we hit the null terminator ('\0'),
    // which marks the end of the C-style string.
    for (int i = 0; input_buffer[i] != '\0'; ++i) {
        // The toupper function converts the character to its uppercase equivalent
        // if it is a lowercase letter. Otherwise, it returns the character unchanged.
        input_buffer[i] = static_cast<char>(std::toupper(static_buffer[i]));
    }

    // 4. Print the result
    std::cout << "Uppercase Result: " << input_buffer << std::endl;

    return 0;
}