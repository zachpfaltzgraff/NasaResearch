#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_USERS 100
#define MAX_STRING 100

// Structure for user personal information
typedef struct {
    char firstName[MAX_STRING];
    char lastName[MAX_STRING];
    char email[MAX_STRING];
    int age;
    char race[MAX_STRING];
} UserInfo;

// Structure for payment information
typedef struct {
    char cardNumber[20];
    char expirationDate[10];
    char cvv[5];
} PaymentInfo;

// Global arrays to store user data
UserInfo users[MAX_USERS];
PaymentInfo payments[MAX_USERS];
int userCount = 0;

// Function prototypes
void addUser();
void searchByEmail();
void searchByFirstName();
void displayUser(int index);
void displayMenu();
void clearInputBuffer();

int main() {
    int choice;
    
    printf("=== User Management System ===\n\n");
    
    while (1) {
        displayMenu();
        printf("Enter your choice: ");
        
        if (scanf("%d", &choice) != 1) {
            clearInputBuffer();
            printf("\nInvalid input! Please enter a number.\n\n");
            continue;
        }
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
                printf("\nExiting program. Goodbye!\n");
                return 0;
            default:
                printf("\nInvalid choice! Please try again.\n\n");
        }
    }
    
    return 0;
}

void displayMenu() {
    printf("1. Add New User\n");
    printf("2. Search User by Email\n");
    printf("3. Search User by First Name\n");
    printf("4. Exit\n");
    printf("----------------------------\n");
}

void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void addUser() {
    if (userCount >= MAX_USERS) {
        printf("\nError: Maximum user limit reached!\n\n");
        return;
    }
    
    printf("\n--- Adding New User ---\n");
    
    // Get personal information
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
    while (scanf("%d", &users[userCount].age) != 1 || users[userCount].age < 0) {
        clearInputBuffer();
        printf("Invalid age! Please enter a positive number: ");
    }
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
    printf("\nUser added successfully! Total users: %d\n\n", userCount);
}

void searchByEmail() {
    if (userCount == 0) {
        printf("\nNo users in the system!\n\n");
        return;
    }
    
    char searchEmail[MAX_STRING];
    printf("\nEnter email to search: ");
    fgets(searchEmail, MAX_STRING, stdin);
    searchEmail[strcspn(searchEmail, "\n")] = 0;
    
    int found = 0;
    for (int i = 0; i < userCount; i++) {
        if (strcmp(users[i].email, searchEmail) == 0) {
            displayUser(i);
            found = 1;
        }
    }
    
    if (!found) {
        printf("\nNo user found with email: %s\n\n", searchEmail);
    }
}

void searchByFirstName() {
    if (userCount == 0) {
        printf("\nNo users in the system!\n\n");
        return;
    }
    
    char searchName[MAX_STRING];
    printf("\nEnter first name to search: ");
    fgets(searchName, MAX_STRING, stdin);
    searchName[strcspn(searchName, "\n")] = 0;
    
    int found = 0;
    for (int i = 0; i < userCount; i++) {
        if (strcasecmp(users[i].firstName, searchName) == 0) {
            displayUser(i);
            found = 1;
        }
    }
    
    if (!found) {
        printf("\nNo user found with first name: %s\n\n", searchName);
    }
}

void displayUser(int index) {
    printf("\n========== User Information ==========\n");
    printf("Name: %s %s\n", users[index].firstName, users[index].lastName);
    printf("Email: %s\n", users[index].email);
    printf("Age: %d\n", users[index].age);
    printf("Race: %s\n", users[index].race);
    printf("\n--- Payment Information ---\n");
    printf("Card Number: %s\n", payments[index].cardNumber);
    printf("Expiration Date: %s\n", payments[index].expirationDate);
    printf("CVV: %s\n", payments[index].cvv);
    printf("=====================================\n\n");
}