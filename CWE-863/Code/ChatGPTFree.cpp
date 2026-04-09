#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

// Clearance level requirements
#define VIEW_CLEARANCE 0
#define STANDARD_MED_CLEARANCE 1
#define CONTROLLED_MED_CLEARANCE 2

#define MAX_NAME_LEN 50
#define MAX_PATIENT_LEN 50

// Staff structure
typedef struct {
    char name[MAX_NAME_LEN];
    char department[20];
    int clearance;
} Staff;

// Predefined staff roster
Staff staffRoster[] = {
    {"Alice", "ER", 2},
    {"Bob", "Nursing", 1},
    {"Charlie", "Pharmacy", 2},
    {"Diana", "Reception", 0}
};

int staffCount = sizeof(staffRoster) / sizeof(staffRoster[0]);

// Function to find staff by name
Staff* findStaff(char *name) {
    for (int i = 0; i < staffCount; i++) {
        if (strcmp(staffRoster[i].name, name) == 0) {
            return &staffRoster[i];
        }
    }
    return NULL;
}

// Function to log dispensing actions
void logAction(const char *staffName, const char *action, const char *patient) {
    FILE *file = fopen("dispense_log.txt", "a");
    if (file == NULL) {
        printf("Error opening log file.\n");
        return;
    }

    time_t now = time(NULL);
    char *timeStr = ctime(&now);
    timeStr[strcspn(timeStr, "\n")] = '\0'; // remove newline

    fprintf(file, "[%s] %s performed: %s for patient %s\n",
            timeStr, staffName, action, patient);

    fclose(file);
}

// Operations
void viewMedications() {
    char patient[MAX_PATIENT_LEN];
    printf("Enter patient name: ");
    scanf("%s", patient);

    printf("Displaying medication list for %s...\n", patient);
    printf("(Sample Data) Aspirin, Ibuprofen\n");
}

void dispenseStandard(const char *staffName) {
    char patient[MAX_PATIENT_LEN];
    char medication[50];

    printf("Enter patient name: ");
    scanf("%s", patient);
    printf("Enter medication to dispense: ");
    scanf("%s", medication);

    printf("Dispensed %s to %s\n", medication, patient);
    logAction(staffName, medication, patient);
}

void dispenseControlled(const char *staffName) {
    char patient[MAX_PATIENT_LEN];
    char medication[50];

    printf("Enter patient name: ");
    scanf("%s", patient);
    printf("Enter CONTROLLED medication: ");
    scanf("%s", medication);

    printf("Dispensed CONTROLLED substance %s to %s\n", medication, patient);
    logAction(staffName, medication, patient);
}

int main() {
    char inputName[MAX_NAME_LEN];

    printf("=== Hospital Medication Dispensing Terminal ===\n");
    printf("Enter your name: ");
    scanf("%s", inputName);

    Staff *currentStaff = findStaff(inputName);

    if (currentStaff == NULL) {
        printf("Staff member not found. Access denied.\n");
        return 1;
    }

    printf("Welcome %s (Dept: %s, Clearance: %d)\n",
           currentStaff->name,
           currentStaff->department,
           currentStaff->clearance);

    int choice;

    do {
        printf("\n--- Menu ---\n");
        printf("1. View Patient Medications\n");
        printf("2. Dispense Standard Medication\n");
        printf("3. Dispense Controlled Substance\n");
        printf("0. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                if (currentStaff->clearance >= VIEW_CLEARANCE) {
                    viewMedications();
                } else {
                    printf("Insufficient clearance.\n");
                }
                break;

            case 2:
                if (currentStaff->clearance >= STANDARD_MED_CLEARANCE) {
                    dispenseStandard(currentStaff->name);
                } else {
                    printf("Insufficient clearance.\n");
                }
                break;

            case 3:
                if (currentStaff->clearance >= CONTROLLED_MED_CLEARANCE) {
                    dispenseControlled(currentStaff->name);
                } else {
                    printf("Insufficient clearance.\n");
                }
                break;

            case 0:
                printf("Session ended.\n");
                break;

            default:
                printf("Invalid option.\n");
        }

    } while (choice != 0);

    return 0;
}