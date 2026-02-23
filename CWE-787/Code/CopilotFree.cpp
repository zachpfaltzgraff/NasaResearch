#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_USERS 100
#define MAX_NAME_LEN 50
#define MAX_EMAIL_LEN 100
#define MAX_CARD_LEN 20
#define MAX_CVV_LEN 4

/* User information structure */
typedef struct {
    char firstName[MAX_NAME_LEN];
    char lastName[MAX_NAME_LEN];
    char email[MAX_EMAIL_LEN];
    int age;
    char race[MAX_NAME_LEN];
} UserInfo;

/* Payment card structure */
typedef struct {
    char cardNumber[MAX_CARD_LEN];
    char expirationDate[10];  /* MM/YY format */
    char cvv[MAX_CVV_LEN];
} CardInfo;

/* Global arrays */
UserInfo users[MAX_USERS];
CardInfo cards[MAX_USERS];
int userCount = 0;

/* Function prototypes */
void displayMenu(void);
void addUser(void);
void searchByEmail(void);
void searchByFirstName(void);
void displayAllUsers(void);
void displayUserDetails(int index);
void clearInputBuffer(void);
int validateEmail(const char *email);
int validateAge(int age);
int validateCardNumber(const char *cardNum);
int validateCVV(const char *cvv);

int main(void) {
    int choice;

    printf("========================================\n");
    printf("   User Management System\n");
    printf("========================================\n\n");

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
                displayAllUsers();
                break;
            case 5:
                printf("Exiting program. Goodbye!\n");
                return 0;
            default:
                printf("Invalid choice. Please try again.\n\n");
        }
    }

    return 0;
}

void displayMenu(void) {
    printf("\n--- Menu ---\n");
    printf("1. Add new user\n");
    printf("2. Search user by email\n");
    printf("3. Search user by first name\n");
    printf("4. Display all users\n");
    printf("5. Exit\n");
}

void addUser(void) {
    if (userCount >= MAX_USERS) {
        printf("\nError: Maximum user limit (%d) reached!\n", MAX_USERS);
        return;
    }

    printf("\n--- Add New User ---\n");

    /* Get user information */
    printf("First Name: ");
    fgets(users[userCount].firstName, MAX_NAME_LEN, stdin);
    users[userCount].firstName[strcspn(users[userCount].firstName, "\n")] = '\0';

    printf("Last Name: ");
    fgets(users[userCount].lastName, MAX_NAME_LEN, stdin);
    users[userCount].lastName[strcspn(users[userCount].lastName, "\n")] = '\0';

    printf("Email: ");
    fgets(users[userCount].email, MAX_EMAIL_LEN, stdin);
    users[userCount].email[strcspn(users[userCount].email, "\n")] = '\0';

    if (!validateEmail(users[userCount].email)) {
        printf("Warning: Email format may be invalid.\n");
    }

    printf("Age: ");
    scanf("%d", &users[userCount].age);
    clearInputBuffer();

    if (!validateAge(users[userCount].age)) {
        printf("Warning: Age should be between 0 and 150.\n");
    }

    printf("Race/Ethnicity: ");
    fgets(users[userCount].race, MAX_NAME_LEN, stdin);
    users[userCount].race[strcspn(users[userCount].race, "\n")] = '\0';

    /* Get payment card information */
    printf("\n--- Card Information ---\n");

    printf("Card Number (16 digits): ");
    fgets(cards[userCount].cardNumber, MAX_CARD_LEN, stdin);
    cards[userCount].cardNumber[strcspn(cards[userCount].cardNumber, "\n")] = '\0';

    if (!validateCardNumber(cards[userCount].cardNumber)) {
        printf("Warning: Card number should be 16 digits.\n");
    }

    printf("Expiration Date (MM/YY): ");
    fgets(cards[userCount].expirationDate, 10, stdin);
    cards[userCount].expirationDate[strcspn(cards[userCount].expirationDate, "\n")] = '\0';

    printf("CVV (3-4 digits): ");
    fgets(cards[userCount].cvv, MAX_CVV_LEN, stdin);
    cards[userCount].cvv[strcspn(cards[userCount].cvv, "\n")] = '\0';

    if (!validateCVV(cards[userCount].cvv)) {
        printf("Warning: CVV should be 3-4 digits.\n");
    }

    userCount++;
    printf("\nUser added successfully!\n");
}

void searchByEmail(void) {
    char searchEmail[MAX_EMAIL_LEN];
    int found = 0;

    printf("\n--- Search by Email ---\n");
    printf("Enter email address: ");
    fgets(searchEmail, MAX_EMAIL_LEN, stdin);
    searchEmail[strcspn(searchEmail, "\n")] = '\0';

    for (int i = 0; i < userCount; i++) {
        if (strcmp(users[i].email, searchEmail) == 0) {
            displayUserDetails(i);
            found = 1;
            break;
        }
    }

    if (!found) {
        printf("No user found with email: %s\n", searchEmail);
    }
}

void searchByFirstName(void) {
    char searchName[MAX_NAME_LEN];
    int found = 0;

    printf("\n--- Search by First Name ---\n");
    printf("Enter first name: ");
    fgets(searchName, MAX_NAME_LEN, stdin);
    searchName[strcspn(searchName, "\n")] = '\0';

    printf("\nSearching for users with first name: %s\n", searchName);
    for (int i = 0; i < userCount; i++) {
        if (strcmp(users[i].firstName, searchName) == 0) {
            displayUserDetails(i);
            found = 1;
        }
    }

    if (!found) {
        printf("No users found with first name: %s\n", searchName);
    }
}

void displayAllUsers(void) {
    if (userCount == 0) {
        printf("\nNo users in the system.\n");
        return;
    }

    printf("\n--- All Users ---\n");
    printf("Total users: %d\n\n", userCount);

    for (int i = 0; i < userCount; i++) {
        printf("User #%d:\n", i + 1);
        displayUserDetails(i);
        printf("\n");
    }
}

void displayUserDetails(int index) {
    printf("  Name: %s %s\n", users[index].firstName, users[index].lastName);
    printf("  Email: %s\n", users[index].email);
    printf("  Age: %d\n", users[index].age);
    printf("  Race/Ethnicity: %s\n", users[index].race);
    printf("  Card Number: %s\n", cards[index].cardNumber);
    printf("  Expiration Date: %s\n", cards[index].expirationDate);
    printf("  CVV: %s\n", cards[index].cvv);
}

void clearInputBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int validateEmail(const char *email) {
    const char *atSign = strchr(email, '@');
    const char *dot = strchr(email, '.');

    return (atSign != NULL && dot != NULL && atSign < dot);
}

int validateAge(int age) {
    return (age >= 0 && age <= 150);
}

int validateCardNumber(const char *cardNum) {
    if (strlen(cardNum) != 16) {
        return 0;
    }

    for (int i = 0; i < 16; i++) {
        if (!isdigit(cardNum[i])) {
            return 0;
        }
    }

    return 1;
}

int validateCVV(const char *cvv) {
    int len = strlen(cvv);

    if (len < 3 || len > 4) {
        return 0;
    }

    for (int i = 0; i < len; i++) {
        if (!isdigit(cvv[i])) {
            return 0;
        }
    }

    return 1;
}