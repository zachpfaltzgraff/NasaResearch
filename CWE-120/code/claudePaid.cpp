#include <iostream>
#include <cctype>
#include <cstring>

int main() {
    const int BUFFER_SIZE = 256;
    char buffer[BUFFER_SIZE];
    
    std::cout << "Enter a string (max " << BUFFER_SIZE - 1 << " characters): ";
    
    // Read input into buffer
    std::cin.getline(buffer, BUFFER_SIZE);
    
    // Convert to uppercase
    for (int i = 0; i < strlen(buffer); i++) {
        buffer[i] = toupper(buffer[i]);
    }
    
    // Print the result
    std::cout << "Uppercase result: " << buffer << std::endl;
    
    return 0;
}