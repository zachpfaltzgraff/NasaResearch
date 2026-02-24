#include <stdio.h>
#include <stdlib.h>

/* Structure to hold employee data */
typedef struct {
    int id;
    int salary;
} Employee;

/* Function prototypes */
void inputEmployees(Employee *employees, int count);
void displayEmployees(const Employee *employees, int count);
void searchEmployee(const Employee *employees, int count);
void updateSalary(Employee *employees, int count);
void deleteEmployee(Employee **employees, int *count);

int main(void) {
    int n;
    Employee *employees = NULL;

    printf("========================================\n");
    printf("      Employee Management System        \n");
    printf("========================================\n");

    /* Ask how many employees */
    printf("Enter the number of employees: ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        fprintf(stderr, "Invalid number of employees.\n");
        return EXIT_FAILURE;
    }

    /* Dynamically allocate memory */
    employees = (Employee *)malloc(n * sizeof(Employee));
    if (employees == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        return EXIT_FAILURE;
    }

    /* Input employee details */
    inputEmployees(employees, n);

    /* Menu loop */
    int choice;
    do {
        printf("\n--- Main Menu ---\n");
        printf("1. Display all employees\n");
        printf("2. Search employee by ID\n");
        printf("3. Update employee salary\n");
        printf("4. Delete an employee\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                displayEmployees(employees, n);
                break;
            case 2:
                searchEmployee(employees, n);
                break;
            case 3:
                updateSalary(employees, n);
                break;
            case 4:
                deleteEmployee(&employees, &n);
                break;
            case 5:
                printf("Exiting. Goodbye!\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    } while (choice != 5);

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

        printf("  Enter Employee ID   : ");
        scanf("%d", &employees[i].id);

        printf("  Enter Salary ($)    : ");
        scanf("%d", &employees[i].salary);
    }
    printf("\nAll employee records saved successfully!\n");
}

/* Display all employees in a formatted table */
void displayEmployees(const Employee *employees, int count) {
    if (count == 0) {
        printf("No employee records found.\n");
        return;
    }

    printf("\n%-5s  %-12s  %-12s\n", "No.", "Employee ID", "Salary ($)");
    printf("-------------------------------\n");
    for (int i = 0; i < count; i++) {
        printf("%-5d  %-12d  %-12d\n", i + 1, employees[i].id, employees[i].salary);
    }
}

/* Search for an employee by ID */
void searchEmployee(const Employee *employees, int count) {
    int searchId;
    printf("Enter the Employee ID to search: ");
    scanf("%d", &searchId);

    for (int i = 0; i < count; i++) {
        if (employees[i].id == searchId) {
            printf("\nEmployee found:\n");
            printf("  Employee ID : %d\n", employees[i].id);
            printf("  Salary      : $%d\n", employees[i].salary);
            return;
        }
    }
    printf("Employee with ID %d not found.\n", searchId);
}

/* Update the salary of an employee by ID */
void updateSalary(Employee *employees, int count) {
    int searchId, newSalary;
    printf("Enter the Employee ID to update salary: ");
    scanf("%d", &searchId);

    for (int i = 0; i < count; i++) {
        if (employees[i].id == searchId) {
            printf("Current salary: $%d\n", employees[i].salary);
            printf("Enter new salary: $");
            scanf("%d", &newSalary);
            employees[i].salary = newSalary;
            printf("Salary updated successfully!\n");
            return;
        }
    }
    printf("Employee with ID %d not found.\n", searchId);
}

/* Delete an employee by ID using realloc */
void deleteEmployee(Employee **employees, int *count) {
    if (*count == 0) {
        printf("No employees to delete.\n");
        return;
    }

    int searchId;
    printf("Enter the Employee ID to delete: ");
    scanf("%d", &searchId);

    int index = -1;
    for (int i = 0; i < *count; i++) {
        if ((*employees)[i].id == searchId) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        printf("Employee with ID %d not found.\n", searchId);
        return;
    }

    /* Shift remaining employees left */
    for (int i = index; i < *count - 1; i++) {
        (*employees)[i] = (*employees)[i + 1];
    }

    /* Shrink the allocated memory */
    (*count)--;
    if (*count == 0) {
        free(*employees);
        *employees = NULL;
    } else {
        Employee *temp = (Employee *)realloc(*employees, (*count) * sizeof(Employee));
        if (temp != NULL) {
            *employees = temp;
        }
    }

    printf("Employee with ID %d deleted successfully.\n", searchId);
}