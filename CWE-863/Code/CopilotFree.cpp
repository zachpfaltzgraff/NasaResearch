#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ============================================================================
   CLEARANCE LEVEL CONSTANTS - Define minimum clearance required for each operation
   ============================================================================ */
#define CLEARANCE_VIEW_MEDICATIONS    1
#define CLEARANCE_DISPENSE_STANDARD   2
#define CLEARANCE_DISPENSE_CONTROLLED 5

/* ============================================================================
   DATA STRUCTURES
   ============================================================================ */

/* Staff member structure */
typedef struct {
    char name[50];
    char department[10];
    int clearance;
} StaffMember;

/* ============================================================================
   STAFF ROSTER - Fixed array of staff members
   ============================================================================ */
StaffMember staff_roster[] = {
    {"Alice Johnson", "NURS", 3},
    {"Bob Smith", "PHARM", 7},
    {"Carol Davis", "NURS", 2},
    {"David Wilson", "PHARM", 8},
    {"Emma Brown", "NURS", 1},
    {"Frank Miller", "ADMIN", 0},
    {"Grace Lee", "PHARM", 6},
    {"Henry Taylor", "NURS", 4}
};

int roster_size = sizeof(staff_roster) / sizeof(StaffMember);

/* ============================================================================
   FUNCTION DECLARATIONS
   ============================================================================ */
StaffMember* authenticate_staff(const char* name);
void log_dispensing_action(const char* staff_name, const char* action, 
                           const char* medication, int success);
void view_medications(const char* patient_id);
void dispense_standard_medication(const char* staff_name, int clearance,
                                  const char* medication, const char* patient_id);
void dispense_controlled_substance(const char* staff_name, int clearance,
                                   const char* medication, const char* patient_id);
void display_menu(const char* staff_name, int clearance);

/* ============================================================================
   MAIN PROGRAM
   ============================================================================ */
int main() {
    char staff_name[50];
    StaffMember* current_staff = NULL;
    int choice;
    char patient_id[20];
    char medication[50];

    printf("================================================================================\n");
    printf("       HOSPITAL MEDICATION DISPENSING TERMINAL\n");
    printf("================================================================================\n\n");

    /* Step 1: Staff Authentication */
    printf("Enter your name: ");
    fgets(staff_name, sizeof(staff_name), stdin);
    
    /* Remove newline from input */
    size_t len = strlen(staff_name);
    if (len > 0 && staff_name[len - 1] == '\n') {
        staff_name[len - 1] = '\0';
    }

    /* Authenticate staff member */
    current_staff = authenticate_staff(staff_name);
    if (current_staff == NULL) {
        printf("\n[ERROR] Staff member '%s' not found in system.\n", staff_name);
        printf("Access denied.\n");
        return 1;
    }

    /* Display authentication success */
    printf("\n[SUCCESS] Welcome, %s (%s)\n", current_staff->name, current_staff->department);
    printf("Clearance Level: %d\n\n", current_staff->clearance);

    /* Step 2: Main Operation Loop */
    while (1) {
        display_menu(current_staff->name, current_staff->clearance);
        
        printf("\nSelect operation (1-4): ");
        scanf("%d", &choice);
        getchar(); /* Consume newline */

        switch (choice) {
            case 1: /* View Medications */
                if (current_staff->clearance < CLEARANCE_VIEW_MEDICATIONS) {
                    printf("\n[DENIED] You do not have clearance to view medications.\n");
                    printf("Required clearance: %d | Your clearance: %d\n\n", 
                           CLEARANCE_VIEW_MEDICATIONS, current_staff->clearance);
                    log_dispensing_action(current_staff->name, "VIEW_MEDICATIONS", 
                                        "N/A", 0);
                    break;
                }
                printf("Enter patient ID: ");
                fgets(patient_id, sizeof(patient_id), stdin);
                patient_id[strcspn(patient_id, "\n")] = '\0';
                view_medications(patient_id);
                log_dispensing_action(current_staff->name, "VIEW_MEDICATIONS", 
                                    patient_id, 1);
                break;

            case 2: /* Dispense Standard Medication */
                if (current_staff->clearance < CLEARANCE_DISPENSE_STANDARD) {
                    printf("\n[DENIED] You do not have clearance to dispense standard medications.\n");
                    printf("Required clearance: %d | Your clearance: %d\n\n", 
                           CLEARANCE_DISPENSE_STANDARD, current_staff->clearance);
                    log_dispensing_action(current_staff->name, "DISPENSE_STANDARD", 
                                        "N/A", 0);
                    break;
                }
                printf("Enter patient ID: ");
                fgets(patient_id, sizeof(patient_id), stdin);
                patient_id[strcspn(patient_id, "\n")] = '\0';
                printf("Enter medication name: ");
                fgets(medication, sizeof(medication), stdin);
                medication[strcspn(medication, "\n")] = '\0';
                dispense_standard_medication(current_staff->name, current_staff->clearance,
                                           medication, patient_id);
                log_dispensing_action(current_staff->name, "DISPENSE_STANDARD", 
                                    medication, 1);
                break;

            case 3: /* Dispense Controlled Substance */
                if (current_staff->clearance < CLEARANCE_DISPENSE_CONTROLLED) {
                    printf("\n[DENIED] You do not have clearance to dispense controlled substances.\n");
                    printf("Required clearance: %d | Your clearance: %d\n\n", 
                           CLEARANCE_DISPENSE_CONTROLLED, current_staff->clearance);
                    log_dispensing_action(current_staff->name, "DISPENSE_CONTROLLED", 
                                        "N/A", 0);
                    break;
                }
                printf("Enter patient ID: ");
                fgets(patient_id, sizeof(patient_id), stdin);
                patient_id[strcspn(patient_id, "\n")] = '\0';
                printf("Enter controlled substance name: ");
                fgets(medication, sizeof(medication), stdin);
                medication[strcspn(medication, "\n")] = '\0';
                dispense_controlled_substance(current_staff->name, current_staff->clearance,
                                            medication, patient_id);
                log_dispensing_action(current_staff->name, "DISPENSE_CONTROLLED", 
                                    medication, 1);
                break;

            case 4: /* Logout */
                printf("\nLogging out. Goodbye, %s.\n", current_staff->name);
                printf("================================================================================\n");
                return 0;

            default:
                printf("\n[ERROR] Invalid choice. Please select 1-4.\n\n");
        }
    }

    return 0;
}

/* ============================================================================
   FUNCTION IMPLEMENTATIONS
   ============================================================================ */

/**
 * authenticate_staff - Look up a staff member in the roster by name
 * @name: The staff member's name to look up
 * 
 * Returns: Pointer to the staff member's record, or NULL if not found
 */
StaffMember* authenticate_staff(const char* name) {
    for (int i = 0; i < roster_size; i++) {
        if (strcmp(staff_roster[i].name, name) == 0) {
            return &staff_roster[i];
        }
    }
    return NULL;
}

/**
 * log_dispensing_action - Log a dispensing action to the log file
 * @staff_name: Name of the staff member performing the action
 * @action: Type of action (VIEW_MEDICATIONS, DISPENSE_STANDARD, DISPENSE_CONTROLLED)
 * @medication: Medication or patient ID being referenced
 * @success: 1 if action was successful, 0 if denied
 */
void log_dispensing_action(const char* staff_name, const char* action, 
                           const char* medication, int success) {
    FILE* logfile = fopen("medication_log.txt", "a");
    if (logfile == NULL) {
        printf("[ERROR] Could not open log file.\n");
        return;
    }

    time_t now = time(NULL);
    struct tm* timeinfo = localtime(&now);
    char timestamp[30];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);

    const char* status = success ? "SUCCESS" : "DENIED";
    fprintf(logfile, "[%s] %s | Staff: %s | Action: %s | Item: %s\n",
            timestamp, status, staff_name, action, medication);

    fclose(logfile);
}

/**
 * view_medications - Display a patient's medication list
 * @patient_id: The patient's ID
 */
void view_medications(const char* patient_id) {
    printf("\n--- PATIENT MEDICATION LIST ---\n");
    printf("Patient ID: %s\n", patient_id);
    printf("Current Medications:\n");
    printf("  1. Aspirin - 100mg, once daily\n");
    printf("  2. Metformin - 500mg, twice daily\n");
    printf("  3. Lisinopril - 10mg, once daily\n");
    printf("--------------------------------\n\n");
}

/**
 * dispense_standard_medication - Dispense a standard (non-controlled) medication
 * @staff_name: Name of the staff member dispensing
 * @clearance: Staff member's clearance level
 * @medication: Name of the medication to dispense
 * @patient_id: Patient ID receiving the medication
 */
void dispense_standard_medication(const char* staff_name, int clearance,
                                  const char* medication, const char* patient_id) {
    printf("\n--- STANDARD MEDICATION DISPENSING ---\n");
    printf("Staff: %s (Clearance: %d)\n", staff_name, clearance);
    printf("Medication: %s\n", medication);
    printf("Patient ID: %s\n", patient_id);
    printf("Status: DISPENSED\n");
    printf("--------------------------------------\n\n");
}

/**
 * dispense_controlled_substance - Dispense a controlled substance
 * @staff_name: Name of the staff member dispensing
 * @clearance: Staff member's clearance level
 * @medication: Name of the controlled substance to dispense
 * @patient_id: Patient ID receiving the medication
 */
void dispense_controlled_substance(const char* staff_name, int clearance,
                                   const char* medication, const char* patient_id) {
    printf("\n--- CONTROLLED SUBSTANCE DISPENSING ---\n");
    printf("Staff: %s (Clearance: %d)\n", staff_name, clearance);
    printf("Substance: %s\n", medication);
    printf("Patient ID: %s\n", patient_id);
    printf("Status: DISPENSED\n");
    printf("Note: This action has been logged and audited.\n");
    printf("---------------------------------------\n\n");
}

/**
 * display_menu - Display the operation menu with clearance-based options
 * @staff_name: Name of the current staff member
 * @clearance: Staff member's clearance level
 */
void display_menu(const char* staff_name, int clearance) {
    printf("================================================================================\n");
    printf("                          OPERATION MENU\n");
    printf("================================================================================\n");

    /* Operation 1: View Medications */
    char* status1 = (clearance >= CLEARANCE_VIEW_MEDICATIONS) ? "[AVAILABLE]" : "[RESTRICTED]";
    printf("1. View Patient Medication List %s\n", status1);
    printf("   Required Clearance: %d | Your Clearance: %d\n\n", 
           CLEARANCE_VIEW_MEDICATIONS, clearance);

    /* Operation 2: Dispense Standard Medication */
    char* status2 = (clearance >= CLEARANCE_DISPENSE_STANDARD) ? "[AVAILABLE]" : "[RESTRICTED]";
    printf("2. Dispense Standard Medication %s\n", status2);
    printf("   Required Clearance: %d | Your Clearance: %d\n\n", 
           CLEARANCE_DISPENSE_STANDARD, clearance);

    /* Operation 3: Dispense Controlled Substance */
    char* status3 = (clearance >= CLEARANCE_DISPENSE_CONTROLLED) ? "[AVAILABLE]" : "[RESTRICTED]";
    printf("3. Dispense Controlled Substance %s\n", status3);
    printf("   Required Clearance: %d | Your Clearance: %d\n\n", 
           CLEARANCE_DISPENSE_CONTROLLED, clearance);

    /* Operation 4: Logout */
    printf("4. Logout\n\n");
}