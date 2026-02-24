#include <stdio.h>

#define NUM_CONTACTS 5

int main() {
    const char *names[NUM_CONTACTS] = {
        "Alice Johnson",
        "Bob Smith",
        "Carol White",
        "David Brown",
        "Eve Davis"
    };

    const char *emails[NUM_CONTACTS] = {
        "alice.johnson@email.com",
        "bob.smith@email.com",
        "carol.white@email.com",
        "david.brown@email.com",
        "eve.davis@email.com"
    };

    const char *phones[NUM_CONTACTS] = {
        "555-101-2020",
        "555-202-3030",
        "555-303-4040",
        "555-404-5050",
        "555-505-6060"
    };

    printf("=== Contact Directory ===\n");
    printf("There are %d contacts available (0 to %d).\n\n", NUM_CONTACTS, NUM_CONTACTS - 1);

    int index;
    printf("Enter a contact number: ");
    if (scanf("%d", &index) != 1) {
        printf("Invalid input. Please enter a number.\n");
        return 1;
    }

    if (index < 0 || index >= NUM_CONTACTS) {
        printf("Error: Index %d is out of range. Please enter a number between 0 and %d.\n",
               index, NUM_CONTACTS - 1);
        return 1;
    }

    printf("\n--- Contact #%d ---\n", index);
    printf("Name:  %s\n", names[index]);
    printf("Email: %s\n", emails[index]);
    printf("Phone: %s\n", phones[index]);

    return 0;
}