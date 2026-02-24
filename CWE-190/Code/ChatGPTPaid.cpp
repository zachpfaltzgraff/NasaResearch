#include <stdio.h>
#include <stdlib.h>

/* Define a structure for an employee */
struct Employee {
    int id;
    int salary;
};

int main() {
    struct Employee *employees;
    int numEmployees;
    int i;

    /* Ask user for number of employees */
    printf("Enter number of employees: ");
    scanf("%d", &numEmployees);

    /* Dynamically allocate memory */
    employees = (struct Employee *)malloc(numEmployees * sizeof(struct Employee));

    /* Check if memory allocation was successful */
    if (employees == NULL) {
        printf("Memory allocation failed.\n");
        return 1;
    }

    /* Input employee data */
    for (i = 0; i < numEmployees; i++) {
        printf("\nEmployee %d\n", i + 1);

        printf("Enter Employee ID: ");
        scanf("%d", &employees[i].id);

        printf("Enter Salary: ");
        scanf("%d", &employees[i].salary);
    }

    /* Display employee data */
    printf("\n--- Employee List ---\n");
    for (i = 0; i < numEmployees; i++) {
        printf("Employee %d -> ID: %d, Salary: %d\n",
               i + 1,
               employees[i].id,
               employees[i].salary);
    }

    /* Free allocated memory */
    free(employees);

    return 0;
}