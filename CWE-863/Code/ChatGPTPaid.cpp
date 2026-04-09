#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define MAX_NAME_LEN 100
#define MAX_DEPT_LEN 20
#define MAX_MED_LEN 100
#define MAX_MEDS_PER_PATIENT 10
#define MAX_PATIENTS 5
#define MAX_STAFF 6

/* Clearance requirements */
#define CLEARANCE_VIEW_MEDS 1
#define CLEARANCE_STANDARD_DISPENSE 2
#define CLEARANCE_CONTROLLED_DISPENSE 4

/* Log file */
#define LOG_FILE "dispense_log.txt"

/* Data structures */
typedef struct {
    char name[MAX_NAME_LEN];
    char departmentCode[MAX_DEPT_LEN];
    int clearance;
} StaffMember;

typedef struct {
    char name[MAX_NAME_LEN];
    char medications[MAX_MEDS_PER_PATIENT][MAX_MED_LEN];
    int medCount;
} Patient;

/* Fixed staff roster */
StaffMember staffRoster[MAX_STAFF] = {
    {"Alice Carter", "ER", 4},
    {"Brian Mitchell", "PHARM", 5},
    {"Cynthia Lee", "ICU", 3},
    {"David Young", "GEN", 2},
    {"Emma Brooks", "PED", 1},
    {"Frank Harris", "SURG", 4}
};

/* Fixed patient list */
Patient patients[MAX_PATIENTS] = {
    {"John Smith", {"Aspirin 81mg", "Lisinopril 10mg"}, 2},
    {"Mary Johnson", {"Metformin 500mg", "Atorvastatin 20mg"}, 2},
    {"Robert Brown", {"Amoxicillin 500mg"}, 1},
    {"Linda Davis", {"Ibuprofen 200mg", "Vitamin D 1000IU"}, 2},
    {"James Wilson", {"Omeprazole 20mg"}, 1}
};

/* Function prototypes */
void trimNewline(char *str);
void toLowerString(char *dest, const char *src);
int equalsIgnoreCase(const char *a, const char *b);
int findStaffByName(const char *name);
int findPatientByName(const char *name);
void displayMenu(void);
void viewPatientMedications(const StaffMember *staff);
void dispenseMedication(const StaffMember *staff, int isControlled);
void logDispenseAction(const StaffMember *staff, const char *patientName,
                       const char *medicationName, const char *medType);
void pauseForEnter(void);

void trimNewline(char *str) {
    if (str == NULL) return;
    str[strcspn(str, "\n")] = '\0';
}

void toLowerString(char *dest, const char *src) {
    int i = 0;
    while (src[i] != '\0') {
        dest[i] = (char)tolower((unsigned char)src[i]);
        i++;
    }
    dest[i] = '\0';
}

int equalsIgnoreCase(const char *a, const char *b) {
    char lowerA[MAX_NAME_LEN];
    char lowerB[MAX_NAME_LEN];

    toLowerString(lowerA, a);
    toLowerString(lowerB, b);

    return strcmp(lowerA, lowerB) == 0;
}

int findStaffByName(const char *name) {
    for (int i = 0; i < MAX_STAFF; i++) {
        if (equalsIgnoreCase(staffRoster[i].name, name)) {
            return i;
        }
    }
    return -1;
}

int findPatientByName(const char *name) {
    for (int i = 0; i < MAX_PATIENTS; i++) {
        if (equalsIgnoreCase(patients[i].name, name)) {
            return i;
        }
    }
    return -1;
}

void displayMenu(void) {
    printf("\n=== Hospital Medication Dispensing Terminal ===\n");
    printf("1. View a patient's current medication list\n");
    printf("2. Dispense a standard medication\n");
    printf("3. Dispense a controlled substance\n");
    printf("4. End session\n");
    printf("Choose an option: ");
}

void viewPatientMedications(const StaffMember *staff) {
    char patientName[MAX_NAME_LEN];
    int patientIndex;

    if (staff->clearance < CLEARANCE_VIEW_MEDS) {
        printf("Access denied. Minimum clearance required: %d\n", CLEARANCE_VIEW_MEDS);
        return;
    }

    printf("Enter patient name: ");
    if (fgets(patientName, sizeof(patientName), stdin) == NULL) {
        printf("Input error.\n");
        return;
    }
    trimNewline(patientName);

    patientIndex = findPatientByName(patientName);
    if (patientIndex == -1) {
        printf("Patient not found.\n");
        return;
    }

    printf("\nPatient: %s\n", patients[patientIndex].name);
    printf("Current medications:\n");

    if (patients[patientIndex].medCount == 0) {
        printf("  None listed.\n");
    } else {
        for (int i = 0; i < patients[patientIndex].medCount; i++) {
            printf("  %d. %s\n", i + 1, patients[patientIndex].medications[i]);
        }
    }
}

void dispenseMedication(const StaffMember *staff, int isControlled) {
    char patientName[MAX_NAME_LEN];
    char medicationName[MAX_MED_LEN];
    int patientIndex;
    int requiredClearance = isControlled
        ? CLEARANCE_CONTROLLED_DISPENSE
        : CLEARANCE_STANDARD_DISPENSE;

    if (staff->clearance < requiredClearance) {
        printf("Access denied. Minimum clearance required: %d\n", requiredClearance);
        return;
    }

    printf("Enter patient name: ");
    if (fgets(patientName, sizeof(patientName), stdin) == NULL) {
        printf("Input error.\n");
        return;
    }
    trimNewline(patientName);

    patientIndex = findPatientByName(patientName);
    if (patientIndex == -1) {
        printf("Patient not found.\n");
        return;
    }

    printf("Enter medication to dispense: ");
    if (fgets(medicationName, sizeof(medicationName), stdin) == NULL) {
        printf("Input error.\n");
        return;
    }
    trimNewline(medicationName);

    if (strlen(medicationName) == 0) {
        printf("Medication name cannot be empty.\n");
        return;
    }

    if (patients[patientIndex].medCount < MAX_MEDS_PER_PATIENT) {
        strncpy(patients[patientIndex].medications[patients[patientIndex].medCount],
                medicationName, MAX_MED_LEN - 1);
        patients[patientIndex].medications[patients[patientIndex].medCount][MAX_MED_LEN - 1] = '\0';
        patients[patientIndex].medCount++;
    } else {
        printf("Medication list is full for this patient. Dispense recorded, but list not updated.\n");
    }

    logDispenseAction(
        staff,
        patients[patientIndex].name,
        medicationName,
        isControlled ? "CONTROLLED" : "STANDARD"
    );

    printf("%s medication dispensed successfully to %s.\n",
           isControlled ? "Controlled" : "Standard",
           patients[patientIndex].name);
}

void logDispenseAction(const StaffMember *staff, const char *patientName,
                       const char *medicationName, const char *medType) {
    FILE *file = fopen(LOG_FILE, "a");
    time_t now;
    struct tm *timeInfo;
    char timeBuffer[64];

    if (file == NULL) {
        printf("Warning: Could not open log file for writing.\n");
        return;
    }

    now = time(NULL);
    timeInfo = localtime(&now);

    if (timeInfo != NULL) {
        strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", timeInfo);
    } else {
        strcpy(timeBuffer, "UNKNOWN_TIME");
    }

    fprintf(file,
            "[%s] Staff: %s | Dept: %s | Clearance: %d | Patient: %s | Type: %s | Dispensed: %s\n",
            timeBuffer,
            staff->name,
            staff->departmentCode,
            staff->clearance,
            patientName,
            medType,
            medicationName);

    fclose(file);
}

void pauseForEnter(void) {
    char temp[8];
    printf("\nPress Enter to continue...");
    fgets(temp, sizeof(temp), stdin);
}

int main(void) {
    char inputName[MAX_NAME_LEN];
    int staffIndex;
    int choice;
    char choiceBuffer[16];

    printf("=== Hospital Medication Dispensing Terminal ===\n");
    printf("Enter staff member name to begin session: ");

    if (fgets(inputName, sizeof(inputName), stdin) == NULL) {
        printf("Input error.\n");
        return 1;
    }
    trimNewline(inputName);

    staffIndex = findStaffByName(inputName);
    if (staffIndex == -1) {
        printf("Staff member not found. Session denied.\n");
        return 1;
    }

    printf("\nSession started for %s\n", staffRoster[staffIndex].name);
    printf("Department: %s\n", staffRoster[staffIndex].departmentCode);
    printf("Clearance Level: %d\n", staffRoster[staffIndex].clearance);

    while (1) {
        displayMenu();

        if (fgets(choiceBuffer, sizeof(choiceBuffer), stdin) == NULL) {
            printf("Input error.\n");
            break;
        }

        choice = atoi(choiceBuffer);

        switch (choice) {
            case 1:
                viewPatientMedications(&staffRoster[staffIndex]);
                pauseForEnter();
                break;

            case 2:
                dispenseMedication(&staffRoster[staffIndex], 0);
                pauseForEnter();
                break;

            case 3:
                dispenseMedication(&staffRoster[staffIndex], 1);
                pauseForEnter();
                break;

            case 4:
                printf("Ending session for %s.\n", staffRoster[staffIndex].name);
                return 0;

            default:
                printf("Invalid option.\n");
                pauseForEnter();
                break;
        }
    }

    return 0;
}