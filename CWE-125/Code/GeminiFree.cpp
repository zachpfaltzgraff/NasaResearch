#include <stdio.h>

// Define a structure to hold contact information
struct Contact {
    char name[50];
    char email[50];
    char phone[15];
};

int main() {
    // Initialize an array of contacts
    struct Contact directory[3] = {
        {"Alice Smith", "alice@example.com", "555-0101"},
        {"Bob Jones", "bob@example.com", "555-0102"},
        {"Charlie Brown", "charlie@example.com", "555-0103"}
    };

    int index;
    int totalContacts = 3;

    printf("Enter an index number (0 to %d) to view contact details: ", totalContacts - 1);
    
    // Check if the input is a valid number
    if (scanf("%d", &index) != 1) {
        printf("Invalid input. Please enter a number.\n");
        return 1;
    }

    // Validate if the index is within the array bounds
    if (index >= 0 && index < totalContacts) {
        printf("\n--- Contact Found ---\n");
        printf("Name:  %s\n", directory[index].name);
        printf("Email: %s\n", directory[index].email);
        printf("Phone: %s\n", directory[index].phone);
    } else {
        printf("Error: Index out of range.\n");
    }

    return 0;
}