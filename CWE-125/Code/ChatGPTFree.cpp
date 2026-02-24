#include <stdio.h>

#define SIZE 5
#define MAX_LEN 100

int main() {
    // Arrays to store data
    char names[SIZE][MAX_LEN] = {
        "John Smith",
        "Alice Johnson",
        "Robert Brown",
        "Emily Davis",
        "Michael Wilson"
    };

    char emails[SIZE][MAX_LEN] = {
        "john.smith@email.com",
        "alice.johnson@email.com",
        "robert.brown@email.com",
        "emily.davis@email.com",
        "michael.wilson@email.com"
    };

    char phones[SIZE][MAX_LEN] = {
        "555-1234",
        "555-5678",
        "555-9101",
        "555-1122",
        "555-3344"
    };

    int index;

    printf("Enter a number between 0 and %d: ", SIZE - 1);
    scanf("%d", &index);

    if (index >= 0 && index < SIZE) {
        printf("\nName: %s\n", names[index]);
        printf("Email: %s\n", emails[index]);
        printf("Phone: %s\n", phones[index]);
    } else {
        printf("Invalid index.\n");
    }

    return 0;
}