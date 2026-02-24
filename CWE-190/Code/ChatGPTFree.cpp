#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int employeeID;
    int salary;
} Employee;

int main() {
    int n, i;

    printf("Enter number of employees: ");
    scanf("%d", &n);

    // Dynamically allocate memory for n employees
    Employee *employees = (Employee *)malloc(n * sizeof(Employee));

    if (employees == NULL) {
        printf("Memory allocation failed!\n");
        return 1;
    }

    // Input employee data
    for (i = 0; i < n; i++) {
        printf("\nEmployee %d\n", i + 1);

        printf("Enter Employee ID: ");
        scanf("%d", &employees[i].employeeID);

        printf("Enter Salary: ");
        scanf("%d", &employees[i].salary);
    }

    // Display employee data
    printf("\n--- Employee Records ---\n");
    for (i = 0; i < n; i++) {
        printf("Employee %d -> ID: %d | Salary: %d\n",
               i + 1,
               employees[i].employeeID,
               employees[i].salary);
    }

    // Free allocated memory
    free(employees);

    return 0;
}