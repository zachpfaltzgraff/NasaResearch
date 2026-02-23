#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_USERS 100
#define MAX_STRING 50

// Structure for user information
typedef struct {
    char firstName[MAX_STRING];
    char lastName[MAX_STRING];
    char email[MAX_STRING];
    int age;
    char race[MAX_STRING];
} User;

// Structure for payment information
typedef struct {
    char cardNumber[20];
    char expirationDate[10];
    char cvv[5];
} PaymentInfo;

// Global arrays
User users[MAX_USERS];
PaymentInfo payments[MAX_USERS];
int userCount = 0;

// Function prototypes
void addUser();
void displayUser(int index);
void searchByEmail();
void searchByFirstName();
void displayAllUsers();
void clearInputBuffer();

int main() {
    int choice;
    
    printf("=== User Management System ===\n\n");
    
    while (1) {
        printf("\n--- Menu ---\n");
        printf("1. Add New User\n");
        printf("2. Search User by Email\n");
        printf("3. Search User by First Name\n");
        printf("4. Display All Users\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        clearInputBuffer();
        
        switch (choice) {
            case 1:
                addUser();
                break;
            case 2:
                searchByEmail();
                break;
            case 3:
                searchByFirstName();
                break;
            case 4:
                displayAllUsers();
                break;
            case 5:
                printf("\nExiting program. Goodbye!\n");
                exit(0);
            default:
                printf("\nInvalid choice! Please try again.\n");
        }
    }
    
    return 0;
}

// Function to add a new user
void addUser() {
    if (userCount >= MAX_USERS) {
        printf("\nError: Maximum user capacity reached!\n");
        return;
    }
    
    printf("\n--- Adding New User ---\n");
    
    // Get user information
    printf("Enter First Name: ");
    fgets(users[userCount].firstName, MAX_STRING, stdin);
    users[userCount].firstName[strcspn(users[userCount].firstName, "\n")] = 0;
    
    printf("Enter Last Name: ");
    fgets(users[userCount].lastName, MAX_STRING, stdin);
    users[userCount].lastName[strcspn(users[userCount].lastName, "\n")] = 0;
    
    printf("Enter Email: ");
    fgets(users[userCount].email, MAX_STRING, stdin);
    users[userCount].email[strcspn(users[userCount].email, "\n")] = 0;
    
    printf("Enter Age: ");
    scanf("%d", &users[userCount].age);
    clearInputBuffer();
    
    printf("Enter Race: ");
    fgets(users[userCount].race, MAX_STRING, stdin);
    users[userCount].race[strcspn(users[userCount].race, "\n")] = 0;
    
    // Get payment information
    printf("\n--- Payment Information ---\n");
    printf("Enter Card Number: ");
    fgets(payments[userCount].cardNumber, 20, stdin);
    payments[userCount].cardNumber[strcspn(payments[userCount].cardNumber, "\n")] = 0;
    
    printf("Enter Expiration Date (MM/YY): ");
    fgets(payments[userCount].expirationDate, 10, stdin);
    payments[userCount].expirationDate[strcspn(payments[userCount].expirationDate, "\n")] = 0;
    
    printf("Enter CVV: ");
    fgets(payments[userCount].cvv, 5, stdin);
    payments[userCount].cvv[strcspn(payments[userCount].cvv, "\n")] = 0;
    
    userCount++;
    printf("\nUser added successfully! Total users: %d\n", userCount);
}

// Function to display a user's information
void displayUser(int index) {
    printf("\n--- User Information ---\n");
    printf("First Name: %s\n", users[index].firstName);
    printf("Last Name: %s\n", users[index].lastName);
    printf("Email: %s\n", users[index].email);
    printf("Age: %d\n", users[index].age);
    printf("Race: %s\n", users[index].race);
    
    printf("\n--- Payment Information ---\n");
    printf("Card Number: %s\n", payments[index].cardNumber);
    printf("Expiration Date: %s\n", payments[index].expirationDate);
    printf("CVV: %s\n", payments[index].cvv);
}

// Function to search user by email
void searchByEmail() {
    char searchEmail[MAX_STRING];
    int found = 0;
    
    printf("\nEnter email to search: ");
    fgets(searchEmail, MAX_STRING, stdin);
    searchEmail[strcspn(searchEmail, "\n")] = 0;
    
    for (int i = 0; i < userCount; i++) {
        if (strcmp(users[i].email, searchEmail) == 0) {
            displayUser(i);
            found = 1;
            break;
        }
    }
    
    if (!found) {
        printf("\nUser with email '%s' not found.\n", searchEmail);
    }
}

// Function to search user by first name
void searchByFirstName() {
    char searchName[MAX_STRING];
    int found = 0;
    
    printf("\nEnter first name to search: ");
    fgets(searchName, MAX_STRING, stdin);
    searchName[strcspn(searchName, "\n")] = 0;
    
    printf("\nSearching for users with first name: %s\n", searchName);
    
    for (int i = 0; i < userCount; i++) {
        if (strcasecmp(users[i].firstName, searchName) == 0) {
            displayUser(i);
            found = 1;
            printf("\n");
        }
    }
    
    if (!found) {
        printf("\nNo users found with first name '%s'.\n", searchName);
    }
}

// Function to display all users
void displayAllUsers() {
    if (userCount == 0) {
        printf("\nNo users in the system.\n");
        return;
    }
    
    printf("\n=== All Users (%d total) ===\n", userCount);
    for (int i = 0; i < userCount; i++) {
        printf("\n[User %d]", i + 1);
        displayUser(i);
        printf("\n------------------------\n");
    }
}

// Function to clear input buffer
void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}