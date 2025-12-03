#include <iostream>
#include <cctype>   // for std::toupper
#include <cstring>  // for std::strlen

int main() {
    const int BUFFER_SIZE = 256;
    char buffer[BUFFER_SIZE];

    std::cout << "Enter some text: ";
    std::cin.getline(buffer, BUFFER_SIZE);

    // Convert to uppercase
    for (size_t i = 0; i < std::strlen(buffer); ++i) {
        buffer[i] = std::toupper(static_cast<unsigned char>(buffer[i]));
    }

    std::cout << "Uppercased: " << buffer << std::endl;

    return 0;
}
