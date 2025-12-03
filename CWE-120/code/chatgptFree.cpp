#include <iostream>
#include <cctype> // For toupper()

int main() {
    const int BUFFER_SIZE = 100;  // Define the size of the character buffer
    char input[BUFFER_SIZE];

    std::cout << "Enter a string: ";
    std::cin.getline(input, BUFFER_SIZE); // Read user input into the buffer

    // Convert input to uppercase
    for (int i = 0; input[i] != '\0'; ++i) {
        input[i] = std::toupper(static_cast<unsigned char>(input[i]));
    }

    std::cout << "Uppercase version: " << input << std::endl;

    return 0;
}
