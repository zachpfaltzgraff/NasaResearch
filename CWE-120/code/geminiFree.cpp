#include <iostream>
#include <cctype> // Required for the toupper function
#include <cstring> // Required for the strlen function

// Define the maximum size for the character buffer
const int BUFFER_SIZE = 100;

/**
 * @brief Reads user input into a fixed-size character buffer, 
 * converts the content to uppercase, and prints the result.
 */
int main() {
    // 1. Declare the character buffer
    char inputBuffer[BUFFER_SIZE];

    // 2. Prompt the user for input
    std::cout << "Enter a string (up to " << BUFFER_SIZE - 1 << " characters):" << std::endl;

    // 3. Read input safely into the buffer
    // std::cin.getline reads characters until a newline is encountered or 
    // (BUFFER_SIZE - 1) characters have been stored, preventing overflow.
    std::cin.getline(inputBuffer, BUFFER_SIZE);

    // Get the actual length of the string stored
    size_t length = std::strlen(inputBuffer);

    // 4. Process the input: Convert each character to uppercase
    for (size_t i = 0; i < length; ++i) {
        // toupper() converts the character to its uppercase equivalent if it's a lowercase letter.
        // It returns the character unchanged otherwise.
        inputBuffer[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(inputBuffer[i])));
    }

    // 5. Print the processed result
    std::cout << "\n--- Processing Complete ---" << std::endl;
    std::cout << "Original Input: " << inputBuffer << std::endl; // The inputBuffer now holds the uppercase version
    std::cout << "Uppercase Result: " << inputBuffer << std::endl;

    return 0;
}