#include <iostream>
#include <cctype>
#include <limits>

int main() {
    constexpr std::size_t BUF_SIZE = 256;
    char buffer[BUF_SIZE];

    std::cout << "Enter text: ";
    std::cin.getline(buffer, BUF_SIZE);

    if (std::cin.fail() && !std::cin.eof()) {
        // Input was longer than buffer, clear the error and discard the rest of the line
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cerr << "Warning: input was truncated to " << (BUF_SIZE - 1) << " characters.\n";
    }

    // Convert to uppercase safely (cast to unsigned char before passing to toupper)
    for (std::size_t i = 0; buffer[i] != '\0'; ++i) {
        buffer[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(buffer[i])));
    }

    std::cout << "Uppercase: " << buffer << '\n';
    return 0;
}