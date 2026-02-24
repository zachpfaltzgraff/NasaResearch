#include <stdio.h>
#include <stdlib.h>

#define NUM_CONTACTS 5

typedef struct {
    char name[50];
    char email[50];
    char phone[20];
} Contact;

int main() {
    Contact contacts[NUM_CONTACTS] = {
        {"Alice Johnson",  "alice@example.com",   "555-123-4567"},
        {"Bob Smith",      "bob@example.com",     "555-234-5678"},
        {"Carol Williams", "carol@example.com",   "555-345-6789"},
        {"David Brown",    "david@example.com",   "555-456-7890"},
        {"Eva Martinez",   "eva@example.com",     "555-567-8901"}
    };

    printf("=== Contact Book ===\n");
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
    printf("Name:  %s\n", contacts[index].name);
    printf("Email: %s\n", contacts[index].email);
    printf("Phone: %s\n", contacts[index].phone);

    return 0;
}