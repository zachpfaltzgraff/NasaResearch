#include <stdio.h>
#include <stdlib.h>

// Define a structure to hold employee data
struct Employee {
    int id;
    int salary;
};

int main() {
    int n, i;
    struct Employee *ptr;

    // 1. Get the number of employees from the user
    printf("Enter the number of employees: ");
    scanf("%d", &n);

    // 2. Dynamically allocate memory using malloc
    // Memory size = number of employees * size of the Employee struct
    ptr = (struct Employee*)malloc(n * sizeof(struct Employee));

    // Check if memory allocation was successful
    if (ptr == NULL) {
        printf("Memory allocation failed. Exiting program.\n");
        return 1;
    }

    // 3. Store data for each employee
    printf("\n--- Enter Employee Details ---\n");
    for (i = 0; i < n; i++) {
        printf("Employee %d ID: ", i + 1);
        scanf("%d", &(ptr + i)->id);
        printf("Employee %d Salary: ", i + 1);
        scanf("%d", &(ptr + i)->salary);
    }

    // 4. Display the stored data
    printf("\n--- Employee Management System Record ---\n");
    printf("ID\t\tSalary\n");
    printf("--------------------------\n");
    for (i = 0; i < n; i++) {
        printf("%d\t\t$%d\n", (ptr + i)->id, (ptr + i)->salary);
    }

    // 5. Free the allocated memory
    free(ptr);
    printf("\nMemory successfully released. Goodbye!\n");

    return 0;
}