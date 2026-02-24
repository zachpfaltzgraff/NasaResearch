#include <stdio.h>
#include <stdlib.h>

/* Structure to represent an employee */
typedef struct {
    int id;
    int salary;
} Employee;

/* Function prototypes */
void inputEmployees(Employee *employees, int count);
void displayEmployees(const Employee *employees, int count);
void searchEmployee(const Employee *employees, int count);
void updateSalary(Employee *employees, int count);
void displayMenu(void);

int main(void) {
    int numEmployees;
    Employee *employees = NULL;

    printf("===========================================\n");
    printf("       EMPLOYEE MANAGEMENT SYSTEM          \n");
    printf("===========================================\n\n");

    /* Ask user for the number of employees */
    printf("How many employees will be entered? ");
    if (scanf("%d", &numEmployees) != 1 || numEmployees <= 0) {
        fprintf(stderr, "Error: Invalid number of employees.\n");
        return EXIT_FAILURE;
    }

    /* Dynamically allocate memory for the employees */
    employees = (Employee *)malloc(numEmployees * sizeof(Employee));
    if (employees == NULL) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return EXIT_FAILURE;
    }

    /* Input employee details */
    inputEmployees(employees, numEmployees);

    /* Menu-driven operations */
    int choice;
    do {
        displayMenu();
        printf("Enter your choice: ");
        if (scanf("%d", &choice) != 1) {
            fprintf(stderr, "Error: Invalid input.\n");
            break;
        }

        switch (choice) {
            case 1:
                displayEmployees(employees, numEmployees);
                break;
            case 2:
                searchEmployee(employees, numEmployees);
                break;
            case 3:
                updateSalary(employees, numEmployees);
                break;
            case 4:
                printf("\nExiting the system. Goodbye!\n");
                break;
            default:
                printf("\nInvalid choice. Please try again.\n");
        }
    } while (choice != 4);

    /* Free dynamically allocated memory */
    free(employees);
    employees = NULL;

    return EXIT_SUCCESS;
}

/* Input details for all employees */
void inputEmployees(Employee *employees, int count) {
    printf("\n--- Enter Employee Details ---\n");
    for (int i = 0; i < count; i++) {
        printf("\nEmployee %d:\n", i + 1);

        printf("  Enter Employee ID : ");
        scanf("%d", &employees[i].id);

        printf("  Enter Salary ($)  : ");
        scanf("%d", &employees[i].salary);
    }
    printf("\nAll employee records saved successfully!\n");
}

/* Display all employees */
void displayEmployees(const Employee *employees, int count) {
    printf("\n%-5s %-15s %-15s\n", "No.", "Employee ID", "Salary ($)");
    printf("--------------------------------------\n");
    for (int i = 0; i < count; i++) {
        printf("%-5d %-15d %-15d\n", i + 1, employees[i].id, employees[i].salary);
    }
    printf("--------------------------------------\n");
    printf("Total employees: %d\n", count);
}

/* Search for an employee by ID */
void searchEmployee(const Employee *employees, int count) {
    int searchId;
    printf("\nEnter Employee ID to search: ");
    scanf("%d", &searchId);

    int found = 0;
    for (int i = 0; i < count; i++) {
        if (employees[i].id == searchId) {
            printf("\nEmployee found:\n");
            printf("  Employee ID : %d\n", employees[i].id);
            printf("  Salary      : $%d\n", employees[i].salary);
            found = 1;
            break;
        }
    }

    if (!found) {
        printf("\nNo employee found with ID %d.\n", searchId);
    }
}

/* Update salary for an employee by ID */
void updateSalary(Employee *employees, int count) {
    int targetId, newSalary;
    printf("\nEnter Employee ID to update salary: ");
    scanf("%d", &targetId);

    int found = 0;
    for (int i = 0; i < count; i++) {
        if (employees[i].id == targetId) {
            printf("  Current Salary : $%d\n", employees[i].salary);
            printf("  Enter New Salary: ");
            scanf("%d", &newSalary);
            employees[i].salary = newSalary;
            printf("  Salary updated successfully!\n");
            found = 1;
            break;
        }
    }

    if (!found) {
        printf("\nNo employee found with ID %d.\n", targetId);
    }
}

/* Display main menu */
void displayMenu(void) {
    printf("\n===========================================\n");
    printf("                 MAIN MENU                 \n");
    printf("===========================================\n");
    printf("  1. Display All Employees\n");
    printf("  2. Search Employee by ID\n");
    printf("  3. Update Employee Salary\n");
    printf("  4. Exit\n");
    printf("===========================================\n");
}