#include <iostream>
#include <cctype>

int main() {
    const std::size_t MAX = 256;
    char buffer[MAX];

    std::cout << "Enter text (max " << (MAX - 1) << " characters): ";
    if (!std::cin.getline(buffer, static_cast<std::streamsize>(MAX))) {
        // If input was longer than the buffer or an error occurred, handle it.
        std::cin.clear(); // clear error flags
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    // Convert to uppercase safely (cast to unsigned char for std::toupper)
    for (std::size_t i = 0; buffer[i] != '\0'; ++i) {
        buffer[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(buffer[i])));
    }

    std::cout << "Uppercase: " << buffer << '\n';
    return 0;
}