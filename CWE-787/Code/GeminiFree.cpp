#include <stdio.h>
#include <string.h>

#define MAX_USERS 10
#define STR_LEN 50

typedef struct {
    char firstName[STR_LEN];
    char lastName[STR_LEN];
    char email[STR_LEN];
    int age;
    char race[STR_LEN];
} UserProfile;

typedef struct {
    char cardNumber[20];
    char expiryDate[10];
    int cvv;
} PaymentInfo;

// Function to search for a user
void findUser(UserProfile users[], int count) {
    char query[STR_LEN];
    int found = 0;

    printf("\nEnter First Name or Email to search: ");
    scanf("%s", query);

    for (int i = 0; i < count; i++) {
        if (strcmp(users[i].firstName, query) == 0 || strcmp(users[i].email, query) == 0) {
            printf("\n--- User Found --- \n");
            printf("Name: %s %s\n", users[i].firstName, users[i].lastName);
            printf("Email: %s\n", users[i].email);
            printf("Age: %d\n", users[i].age);
            printf("Race: %s\n", users[i].race);
            found = 1;
            break;
        }
    }

    if (!found) {
        printf("\nUser not found.\n");
    }
}

int main() {
    UserProfile users[MAX_USERS];
    PaymentInfo payments[MAX_USERS];
    int userCount = 0;
    char choice;

    do {
        if (userCount >= MAX_USERS) {
            printf("Database full.\n");
            break;
        }

        printf("\n--- Register User %d ---\n", userCount + 1);
        
        // Array 1: Profile Data
        printf("First Name: "); scanf("%s", users[userCount].firstName);
        printf("Last Name: "); scanf("%s", users[userCount].lastName);
        printf("Email: "); scanf("%s", users[userCount].email);
        printf("Age: "); scanf("%d", &users[userCount].age);
        printf("Race: "); scanf("%s", users[userCount].race);

        // Array 2: Payment Data
        printf("Card Number: "); scanf("%s", payments[userCount].cardNumber);
        printf("Expiry (MM/YY): "); scanf("%s", payments[userCount].expiryDate);
        printf("CVV: "); scanf("%d", &payments[userCount].cvv);

        userCount++;

        printf("Add another user? (y/n): ");
        scanf(" %c", &choice);
    } while (choice == 'y' || choice == 'Y');

    // Search functionality
    findUser(users, userCount);

    return 0;
}