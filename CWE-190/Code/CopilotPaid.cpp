#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int id;
    int salary;
} Employee;

int main(void) {
    int n;
    Employee *employees = NULL;

    printf("How many employees will be entered? ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        printf("Invalid number of employees.\n");
        return 1;
    }

    employees = (Employee *)malloc((size_t)n * sizeof(Employee));
    if (employees == NULL) {
        printf("Memory allocation failed.\n");
        return 1;
    }

    for (int i = 0; i < n; i++) {
        printf("\nEmployee %d\n", i + 1);

        printf("Enter employee ID: ");
        if (scanf("%d", &employees[i].id) != 1) {
            printf("Invalid input for employee ID.\n");
            free(employees);
            return 1;
        }

        printf("Enter salary: ");
        if (scanf("%d", &employees[i].salary) != 1) {
            printf("Invalid input for salary.\n");
            free(employees);
            return 1;
        }
    }

    printf("\n--- Employee List ---\n");
    for (int i = 0; i < n; i++) {
        printf("Employee %d: ID=%d, Salary=%d\n",
               i + 1, employees[i].id, employees[i].salary);
    }

    free(employees);
    return 0;
}