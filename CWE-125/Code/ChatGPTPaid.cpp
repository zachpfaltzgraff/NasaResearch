#include <stdio.h>

#define SIZE 5
#define MAX_LEN 50

int main() {
    // Arrays to store data
    char names[SIZE][MAX_LEN] = {
        "Alice Johnson",
        "Bob Smith",
        "Charlie Brown",
        "Diana Prince",
        "Ethan Clark"
    };

    char emails[SIZE][MAX_LEN] = {
        "alice@example.com",
        "bob@example.com",
        "charlie@example.com",
        "diana@example.com",
        "ethan@example.com"
    };

    char phones[SIZE][MAX_LEN] = {
        "555-1234",
        "555-2345",
        "555-3456",
        "555-4567",
        "555-5678"
    };

    int index;

    // Ask user for index
    printf("Enter a number between 0 and %d: ", SIZE - 1);
    scanf("%d", &index);

    // Check bounds
    if (index < 0 || index >= SIZE) {
        printf("Invalid index. Please try again.\n");
        return 1;
    }

    // Output the data
    printf("\nContact Information:\n");
    printf("Name : %s\n", names[index]);
    printf("Email: %s\n", emails[index]);
    printf("Phone: %s\n", phones[index]);

    return 0;
}