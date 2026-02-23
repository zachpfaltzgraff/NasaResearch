#include <stdio.h>
#include <string.h>

#define MAX_USERS 100
#define STR_LEN 100

// -------- USER INFO (Array 1) --------
typedef struct {
    char firstName[STR_LEN];
    char lastName[STR_LEN];
    char email[STR_LEN];
    int age;
    char race[STR_LEN];
} UserInfo;

// -------- CARD INFO (Array 2) --------
typedef struct {
    char cardNumber[STR_LEN];
    char expDate[STR_LEN];
    char cvv[STR_LEN];
} CardInfo;

UserInfo users[MAX_USERS];
CardInfo cards[MAX_USERS];
int userCount = 0;

// Remove newline from fgets
void trimNewline(char *str) {
    str[strcspn(str, "\n")] = 0;
}

// -------- ADD USER --------
void addUser() {
    if (userCount >= MAX_USERS) {
        printf("Database full!\n");
        return;
    }

    printf("Enter First Name: ");
    fgets(users[userCount].firstName, STR_LEN, stdin);
    trimNewline(users[userCount].firstName);

    printf("Enter Last Name: ");
    fgets(users[userCount].lastName, STR_LEN, stdin);
    trimNewline(users[userCount].lastName);

    printf("Enter Email: ");
    fgets(users[userCount].email, STR_LEN, stdin);
    trimNewline(users[userCount].email);

    printf("Enter Age: ");
    scanf("%d", &users[userCount].age);
    getchar(); // clear newline

    printf("Enter Race: ");
    fgets(users[userCount].race, STR_LEN, stdin);
    trimNewline(users[userCount].race);

    printf("Enter Card Number: ");
    fgets(cards[userCount].cardNumber, STR_LEN, stdin);
    trimNewline(cards[userCount].cardNumber);

    printf("Enter Expiration Date (MM/YY): ");
    fgets(cards[userCount].expDate, STR_LEN, stdin);
    trimNewline(cards[userCount].expDate);

    printf("Enter CVV: ");
    fgets(cards[userCount].cvv, STR_LEN, stdin);
    trimNewline(cards[userCount].cvv);

    userCount++;
    printf("User added successfully!\n\n");
}

// -------- SEARCH BY EMAIL --------
void searchByEmail() {
    char email[STR_LEN];
    printf("Enter Email to search: ");
    fgets(email, STR_LEN, stdin);
    trimNewline(email);

    for (int i = 0; i < userCount; i++) {
        if (strcmp(users[i].email, email) == 0) {
            printf("\nUser Found:\n");
            printf("Name: %s %s\n", users[i].firstName, users[i].lastName);
            printf("Email: %s\n", users[i].email);
            printf("Age: %d\n", users[i].age);
            printf("Race: %s\n", users[i].race);
            printf("Card: %s | Exp: %s | CVV: %s\n\n",
                   cards[i].cardNumber,
                   cards[i].expDate,
                   cards[i].cvv);
            return;
        }
    }
    printf("User not found.\n\n");
}

// -------- SEARCH BY FIRST NAME --------
void searchByFirstName() {
    char name[STR_LEN];
    printf("Enter First Name to search: ");
    fgets(name, STR_LEN, stdin);
    trimNewline(name);

    int found = 0;
    for (int i = 0; i < userCount; i++) {
        if (strcmp(users[i].firstName, name) == 0) {
            printf("\nUser Found:\n");
            printf("Name: %s %s\n", users[i].firstName, users[i].lastName);
            printf("Email: %s\n", users[i].email);
            printf("Age: %d\n", users[i].age);
            printf("Race: %s\n", users[i].race);
            printf("Card: %s | Exp: %s | CVV: %s\n\n",
                   cards[i].cardNumber,
                   cards[i].expDate,
                   cards[i].cvv);
            found = 1;
        }
    }

    if (!found)
        printf("No users found with that first name.\n\n");
}

// -------- MAIN MENU --------
int main() {
    int choice;

    while (1) {
        printf("===== USER DATABASE =====\n");
        printf("1. Add User\n");
        printf("2. Search by Email\n");
        printf("3. Search by First Name\n");
        printf("4. Exit\n");
        printf("Choose: ");
        scanf("%d", &choice);
        getchar(); // clear newline

        switch (choice) {
            case 1: addUser(); break;
            case 2: searchByEmail(); break;
            case 3: searchByFirstName(); break;
            case 4: return 0;
            default: printf("Invalid choice\n\n");
        }
    }
}
