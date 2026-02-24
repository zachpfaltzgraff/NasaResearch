#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int employee_id;
    int salary;
} Employee;

int main() {
    int num_employees;
    
    // Ask user for number of employees
    printf("Enter the number of employees: ");
    scanf("%d", &num_employees);
    
    // Validate input
    if (num_employees <= 0) {
        printf("Error: Number of employees must be greater than 0.\n");
        return 1;
    }
    
    // Dynamically allocate memory for employees
    Employee *employees = (Employee *)malloc(num_employees * sizeof(Employee));
    
    // Check if memory allocation was successful
    if (employees == NULL) {
        printf("Error: Memory allocation failed.\n");
        return 1;
    }
    
    // Input employee data
    printf("\n--- Enter Employee Information ---\n");
    for (int i = 0; i < num_employees; i++) {
        printf("\nEmployee %d:\n", i + 1);
        printf("  Enter Employee ID: ");
        scanf("%d", &employees[i].employee_id);
        printf("  Enter Salary: ");
        scanf("%d", &employees[i].salary);
    }
    
    // Display all employees
    printf("\n--- Employee Records ---\n");
    printf("%-12s %-10s\n", "Employee ID", "Salary");
    printf("%-12s %-10s\n", "-----------", "------");
    for (int i = 0; i < num_employees; i++) {
        printf("%-12d %-10d\n", employees[i].employee_id, employees[i].salary);
    }
    
    // Calculate and display total and average salary
    int total_salary = 0;
    for (int i = 0; i < num_employees; i++) {
        total_salary += employees[i].salary;
    }
    int average_salary = total_salary / num_employees;
    
    printf("\n--- Salary Summary ---\n");
    printf("Total Salary: %d\n", total_salary);
    printf("Average Salary: %d\n", average_salary);
    
    // Free dynamically allocated memory
    free(employees);
    employees = NULL;
    
    printf("\nMemory freed successfully.\n");
    
    return 0;
}