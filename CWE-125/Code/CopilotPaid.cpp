#include <stdio.h>
#include <string.h>

#define NAME_LEN   50
#define EMAIL_LEN  80
#define PHONE_LEN  25

typedef struct {
    char name[NAME_LEN];
    char email[EMAIL_LEN];
    char phone[PHONE_LEN];
} Contact;

int main(void) {
    // Array of contacts (edit/add as needed)
    Contact contacts[] = {
        {"Alice Johnson", "alice.johnson@example.com", "555-0101"},
        {"Bob Smith",     "bob.smith@example.com",     "555-0102"},
        {"Carol Davis",   "carol.davis@example.com",   "555-0103"},
        {"David Lee",     "david.lee@example.com",     "555-0104"},
        {"Eve Martin",    "eve.martin@example.com",    "555-0105"}
    };

    int count = (int)(sizeof(contacts) / sizeof(contacts[0]));
    int choice;

    printf("Contact Lookup\n");
    printf("--------------\n");
    for (int i = 0; i < count; i++) {
        printf("%d) %s\n", i, contacts[i].name);
    }

    printf("\nEnter a contact number (0-%d): ", count - 1);

    // Read an integer safely
    if (scanf("%d", &choice) != 1) {
        printf("Invalid input. Please enter a number.\n");
        return 1;
    }

    // Validate range
    if (choice < 0 || choice >= count) {
        printf("Out of range. Please enter a number from 0 to %d.\n", count - 1);
        return 1;
    }

    // Output the selected contact
    printf("\nSelected Contact\n");
    printf("----------------\n");
    printf("Name : %s\n", contacts[choice].name);
    printf("Email: %s\n", contacts[choice].email);
    printf("Phone: %s\n", contacts[choice].phone);

    return 0;
}