#include <stdio.h>
#include <string.h>

#define MAX_CONTACTS 10
#define MAX_NAME_LEN 50
#define MAX_EMAIL_LEN 50
#define MAX_PHONE_LEN 15

// Structure to hold contact information
typedef struct {
    char name[MAX_NAME_LEN];
    char email[MAX_EMAIL_LEN];
    char phone[MAX_PHONE_LEN];
} Contact;

int main() {
    // Initialize contact array with sample data
    Contact contacts[MAX_CONTACTS] = {
        {"John Smith", "john.smith@email.com", "555-0101"},
        {"Sarah Johnson", "sarah.j@email.com", "555-0102"},
        {"Michael Brown", "m.brown@email.com", "555-0103"},
        {"Emily Davis", "emily.d@email.com", "555-0104"},
        {"David Wilson", "d.wilson@email.com", "555-0105"},
        {"Lisa Anderson", "l.anderson@email.com", "555-0106"},
        {"James Taylor", "j.taylor@email.com", "555-0107"},
        {"Jennifer Thomas", "jen.thomas@email.com", "555-0108"},
        {"Robert Jackson", "r.jackson@email.com", "555-0109"},
        {"Mary White", "mary.white@email.com", "555-0110"}
    };

    int choice;
    int index;

    printf("========== Contact List Manager ==========\n");
    printf("You have %d contacts in the database.\n\n", MAX_CONTACTS);

    // Display all contacts with their indices
    printf("Available Contacts:\n");
    printf("-------------------------------------------\n");
    for (int i = 0; i < MAX_CONTACTS; i++) {
        printf("[%d] %s\n", i, contacts[i].name);
    }
    printf("-------------------------------------------\n\n");

    // Get user input
    printf("Enter the contact number (0-%d) to view details: ", MAX_CONTACTS - 1);
    scanf("%d", &index);

    // Validate input
    if (index < 0 || index >= MAX_CONTACTS) {
        printf("Error: Invalid contact number! Please enter a number between 0 and %d.\n", MAX_CONTACTS - 1);
        return 1;
    }

    // Display the selected contact's information
    printf("\n========== Contact Information ==========\n");
    printf("Name:  %s\n", contacts[index].name);
    printf("Email: %s\n", contacts[index].email);
    printf("Phone: %s\n", contacts[index].phone);
    printf("=========================================\n");

    return 0;
}