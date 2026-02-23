#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

#define MAX_USERS 100
#define MAX_STRING 100

// Structure for personal information (Array 1)
typedef struct {
    char firstName[MAX_STRING];
    char lastName[MAX_STRING];
    char email[MAX_STRING];
    int age;
    char race[MAX_STRING];
    int isActive; // Flag to check if this slot is occupied
} PersonalInfo;

// Structure for payment information (Array 2)
typedef struct {
    char cardNumber[20];
    char expirationDate[10];
    char cvv[5];
    int isActive; // Flag to check if this slot is occupied
} PaymentInfo;

// Global arrays
PersonalInfo personalArray[MAX_USERS];
PaymentInfo paymentArray[MAX_USERS];
int userCount = 0;

// Function prototypes
void initializeArrays();
void addUser();
void searchByEmail();
void searchByFirstName();
void displayUser(int index);
void displayMenu();
void clearInputBuffer();

int main() {
    int choice;
    
    initializeArrays();
    
    while (1) {
        displayMenu();
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
                printf("\nExiting program. Goodbye!\n");
                exit(0);
            default:
                printf("\nInvalid choice. Please try again.\n");
        }
    }
    
    return 0;
}

void initializeArrays() {
    for (int i = 0; i < MAX_USERS; i++) {
        personalArray[i].isActive = 0;
        paymentArray[i].isActive = 0;
    }
}

void displayMenu() {
    printf("\n========================================\n");
    printf("        USER MANAGEMENT SYSTEM\n");
    printf("========================================\n");
    printf("1. Add New User\n");
    printf("2. Search User by Email\n");
    printf("3. Search User by First Name\n");
    printf("4. Exit\n");
    printf("========================================\n");
}

void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void addUser() {
    if (userCount >= MAX_USERS) {
        printf("\nError: User database is full. Cannot add more users.\n");
        return;
    }
    
    printf("\n--- Adding New User ---\n");
    
    // Get personal information
    printf("Enter First Name: ");
    fgets(personalArray[userCount].firstName, MAX_STRING, stdin);
    personalArray[userCount].firstName[strcspn(personalArray[userCount].firstName, "\n")] = 0;
    
    printf("Enter Last Name: ");
    fgets(personalArray[userCount].lastName, MAX_STRING, stdin);
    personalArray[userCount].lastName[strcspn(personalArray[userCount].lastName, "\n")] = 0;
    
    printf("Enter Email: ");
    fgets(personalArray[userCount].email, MAX_STRING, stdin);
    personalArray[userCount].email[strcspn(personalArray[userCount].email, "\n")] = 0;
    
    printf("Enter Age: ");
    scanf("%d", &personalArray[userCount].age);
    clearInputBuffer();
    
    printf("Enter Race: ");
    fgets(personalArray[userCount].race, MAX_STRING, stdin);
    personalArray[userCount].race[strcspn(personalArray[userCount].race, "\n")] = 0;
    
    personalArray[userCount].isActive = 1;
    
    // Get payment information
    printf("\n--- Payment Information ---\n");
    printf("Enter Card Number: ");
    fgets(paymentArray[userCount].cardNumber, 20, stdin);
    paymentArray[userCount].cardNumber[strcspn(paymentArray[userCount].cardNumber, "\n")] = 0;
    
    printf("Enter Expiration Date (MM/YY): ");
    fgets(paymentArray[userCount].expirationDate, 10, stdin);
    paymentArray[userCount].expirationDate[strcspn(paymentArray[userCount].expirationDate, "\n")] = 0;
    
    printf("Enter CVV: ");
    fgets(paymentArray[userCount].cvv, 5, stdin);
    paymentArray[userCount].cvv[strcspn(paymentArray[userCount].cvv, "\n")] = 0;
    
    paymentArray[userCount].isActive = 1;
    
    userCount++;
    printf("\nUser added successfully! Total users: %d\n", userCount);
}

void searchByEmail() {
    char searchEmail[MAX_STRING];
    int found = 0;
    
    printf("\nEnter email to search: ");
    fgets(searchEmail, MAX_STRING, stdin);
    searchEmail[strcspn(searchEmail, "\n")] = 0;
    
    for (int i = 0; i < userCount; i++) {
        if (personalArray[i].isActive && 
            strcasecmp(personalArray[i].email, searchEmail) == 0) {
            displayUser(i);
            found = 1;
            break;
        }
    }
    
    if (!found) {
        printf("\nNo user found with email: %s\n", searchEmail);
    }
}

void searchByFirstName() {
    char searchName[MAX_STRING];
    int found = 0;
    
    printf("\nEnter first name to search: ");
    fgets(searchName, MAX_STRING, stdin);
    searchName[strcspn(searchName, "\n")] = 0;
    
    printf("\n--- Search Results ---\n");
    for (int i = 0; i < userCount; i++) {
        if (personalArray[i].isActive && 
            strcasecmp(personalArray[i].firstName, searchName) == 0) {
            displayUser(i);
            found = 1;
            printf("------------------------\n");
        }
    }
    
    if (!found) {
        printf("No users found with first name: %s\n", searchName);
    }
}

void displayUser(int index) {
    printf("\n========== USER INFORMATION ==========\n");
    printf("Personal Information:\n");
    printf("  Name: %s %s\n", personalArray[index].firstName, 
           personalArray[index].lastName);
    printf("  Email: %s\n", personalArray[index].email);
    printf("  Age: %d\n", personalArray[index].age);
    printf("  Race: %s\n", personalArray[index].race);
    
    printf("\nPayment Information:\n");
    printf("  Card Number: %s\n", paymentArray[index].cardNumber);
    printf("  Expiration Date: %s\n", paymentArray[index].expirationDate);
    printf("  CVV: %s\n", paymentArray[index].cvv);
    printf("======================================\n");
}