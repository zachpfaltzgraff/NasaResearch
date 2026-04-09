/*
 * med_terminal.c
 * Hospital Medication Dispensing Terminal
 *
 * Compile:  gcc -Wall -Wextra -o med_terminal med_terminal.c
 * Run:      ./med_terminal
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ─── Clearance thresholds ─────────────────────────────────────────────────── */
#define CLEARANCE_VIEW_MEDS        1   /* view a patient's medication list    */
#define CLEARANCE_DISPENSE_STD     2   /* dispense a standard medication      */
#define CLEARANCE_DISPENSE_CTRL    4   /* dispense a controlled substance     */

/* ─── Limits ────────────────────────────────────────────────────────────────── */
#define MAX_NAME_LEN      64
#define MAX_DEPT_LEN      32
#define MAX_MED_LEN       64
#define MAX_PATIENTS       5
#define MAX_MEDS_PER_PAT   4
#define LOG_FILE          "dispense_log.txt"

/* ─── Staff roster ──────────────────────────────────────────────────────────── */
typedef struct {
    char name[MAX_NAME_LEN];
    char department[MAX_DEPT_LEN];
    int  clearance;
} StaffMember;

static const StaffMember ROSTER[] = {
    { "Alice Nguyen",    "Nursing",        1 },
    { "Bob Harmon",      "Nursing",        2 },
    { "Carol Reyes",     "Pharmacy",       3 },
    { "David Kim",       "Pharmacy",       5 },
    { "Eva Thornton",    "Oncology",       4 },
    { "Frank Osei",      "Administration", 1 },
    { "Grace Patel",     "ICU",            5 },
    { "Henry Blake",     "Neurology",      2 },
};
static const int ROSTER_SIZE = (int)(sizeof(ROSTER) / sizeof(ROSTER[0]));

/* ─── Simulated patient medication lists ────────────────────────────────────── */
typedef struct {
    int  patient_id;
    char patient_name[MAX_NAME_LEN];
    char medications[MAX_MEDS_PER_PAT][MAX_MED_LEN];
    int  med_count;
} PatientRecord;

static const PatientRecord PATIENTS[] = {
    { 1001, "Margaret Sullivan",
      { "Lisinopril 10mg", "Atorvastatin 20mg", "Metformin 500mg" }, 3 },
    { 1002, "James Ortega",
      { "Amoxicillin 500mg", "Ibuprofen 400mg" }, 2 },
    { 1003, "Priya Sharma",
      { "Warfarin 5mg", "Digoxin 0.125mg", "Furosemide 40mg", "Spironolactone 25mg" }, 4 },
    { 1004, "Thomas Weaver",
      { "Omeprazole 20mg" }, 1 },
    { 1005, "Sandra Bloom",
      { "Morphine 10mg", "Ondansetron 4mg", "Dexamethasone 8mg" }, 3 },
};
static const int PATIENT_COUNT = (int)(sizeof(PATIENTS) / sizeof(PATIENTS[0]));

/* ─── Standard medications available for dispensing ─────────────────────────── */
static const char *STD_MEDS[] = {
    "Paracetamol 500mg",
    "Ibuprofen 400mg",
    "Amoxicillin 500mg",
    "Omeprazole 20mg",
    "Metformin 500mg",
    "Lisinopril 10mg",
    "Atorvastatin 20mg",
    "Furosemide 40mg",
};
static const int STD_MED_COUNT = (int)(sizeof(STD_MEDS) / sizeof(STD_MEDS[0]));

/* ─── Controlled substances available for dispensing ────────────────────────── */
static const char *CTRL_MEDS[] = {
    "Morphine 10mg",
    "Oxycodone 5mg",
    "Fentanyl 25mcg patch",
    "Midazolam 2mg",
    "Lorazepam 1mg",
    "Diazepam 5mg",
};
static const int CTRL_MED_COUNT = (int)(sizeof(CTRL_MEDS) / sizeof(CTRL_MEDS[0]));

/* ══════════════════════════════════════════════════════════════════════════════
 * Utility helpers
 * ══════════════════════════════════════════════════════════════════════════════ */

/* Strip trailing newline from fgets() result */
static void strip_newline(char *s) {
    size_t len = strlen(s);
    if (len > 0 && s[len - 1] == '\n') s[len - 1] = '\0';
}

/* Case-insensitive string comparison */
static int str_iequal(const char *a, const char *b) {
    while (*a && *b) {
        char ca = (*a >= 'A' && *a <= 'Z') ? (*a + 32) : *a;
        char cb = (*b >= 'A' && *b <= 'Z') ? (*b + 32) : *b;
        if (ca != cb) return 0;
        a++; b++;
    }
    return (*a == '\0' && *b == '\0');
}

/* Current timestamp as a formatted string */
static void timestamp(char *buf, size_t buf_size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buf, buf_size, "%Y-%m-%d %H:%M:%S", t);
}

/* Print a horizontal separator */
static void separator(void) {
    printf("─────────────────────────────────────────────────────\n");
}

/* ══════════════════════════════════════════════════════════════════════════════
 * Logging
 * ══════════════════════════════════════════════════════════════════════════════ */

static void log_action(const StaffMember *staff,
                        int patient_id, const char *patient_name,
                        const char *action, const char *detail) {
    FILE *f = fopen(LOG_FILE, "a");
    if (!f) {
        fprintf(stderr, "[WARNING] Could not open log file '%s'.\n", LOG_FILE);
        return;
    }

    char ts[32];
    timestamp(ts, sizeof(ts));

    fprintf(f, "[%s] STAFF=\"%s\" DEPT=\"%s\" CLR=%d | "
               "ACTION=%s | PATIENT_ID=%d PATIENT=\"%s\" | DETAIL=\"%s\"\n",
            ts,
            staff->name, staff->department, staff->clearance,
            action,
            patient_id, patient_name,
            detail);

    fclose(f);
}

/* ══════════════════════════════════════════════════════════════════════════════
 * Lookup helpers
 * ══════════════════════════════════════════════════════════════════════════════ */

/* Return pointer into ROSTER or NULL if not found */
static const StaffMember *find_staff(const char *name) {
    for (int i = 0; i < ROSTER_SIZE; i++) {
        if (str_iequal(ROSTER[i].name, name))
            return &ROSTER[i];
    }
    return NULL;
}

/* Return pointer into PATIENTS or NULL */
static const PatientRecord *find_patient_by_id(int id) {
    for (int i = 0; i < PATIENT_COUNT; i++) {
        if (PATIENTS[i].patient_id == id)
            return &PATIENTS[i];
    }
    return NULL;
}

/* ══════════════════════════════════════════════════════════════════════════════
 * Terminal operations
 * ══════════════════════════════════════════════════════════════════════════════ */

/* ── Operation 1: View patient medication list ──────────────────────────────── */
static void op_view_meds(const StaffMember *staff) {
    if (staff->clearance < CLEARANCE_VIEW_MEDS) {
        printf("  [DENIED] Clearance %d required. Your clearance: %d.\n",
               CLEARANCE_VIEW_MEDS, staff->clearance);
        return;
    }

    printf("\n  Enter patient ID: ");
    int pid;
    if (scanf("%d", &pid) != 1) { while (getchar() != '\n'); return; }
    while (getchar() != '\n');

    const PatientRecord *p = find_patient_by_id(pid);
    if (!p) {
        printf("  [ERROR] No patient found with ID %d.\n", pid);
        return;
    }

    printf("\n");
    separator();
    printf("  Patient : %s  (ID: %d)\n", p->patient_name, p->patient_id);
    separator();
    printf("  Current Medications:\n");
    for (int i = 0; i < p->med_count; i++)
        printf("    %d. %s\n", i + 1, p->medications[i]);
    separator();

    log_action(staff, p->patient_id, p->patient_name,
               "VIEW_MEDS", "Viewed medication list");
    printf("  Action logged.\n");
}

/* ── Operation 2: Dispense standard medication ──────────────────────────────── */
static void op_dispense_std(const StaffMember *staff) {
    if (staff->clearance < CLEARANCE_DISPENSE_STD) {
        printf("  [DENIED] Clearance %d required. Your clearance: %d.\n",
               CLEARANCE_DISPENSE_STD, staff->clearance);
        return;
    }

    printf("\n  Enter patient ID: ");
    int pid;
    if (scanf("%d", &pid) != 1) { while (getchar() != '\n'); return; }
    while (getchar() != '\n');

    const PatientRecord *p = find_patient_by_id(pid);
    if (!p) {
        printf("  [ERROR] No patient found with ID %d.\n", pid);
        return;
    }

    printf("\n  Available standard medications:\n");
    for (int i = 0; i < STD_MED_COUNT; i++)
        printf("    %2d. %s\n", i + 1, STD_MEDS[i]);

    printf("\n  Select medication (1-%d): ", STD_MED_COUNT);
    int choice;
    if (scanf("%d", &choice) != 1) { while (getchar() != '\n'); return; }
    while (getchar() != '\n');

    if (choice < 1 || choice > STD_MED_COUNT) {
        printf("  [ERROR] Invalid selection.\n");
        return;
    }

    const char *med = STD_MEDS[choice - 1];
    printf("\n  [OK] Dispensing \"%s\" for patient %s (ID: %d).\n",
           med, p->patient_name, p->patient_id);

    char detail[128];
    snprintf(detail, sizeof(detail), "Dispensed standard medication: %s", med);
    log_action(staff, p->patient_id, p->patient_name, "DISPENSE_STD", detail);
    printf("  Action logged.\n");
}

/* ── Operation 3: Dispense controlled substance ─────────────────────────────── */
static void op_dispense_ctrl(const StaffMember *staff) {
    if (staff->clearance < CLEARANCE_DISPENSE_CTRL) {
        printf("  [DENIED] Clearance %d required. Your clearance: %d.\n",
               CLEARANCE_DISPENSE_CTRL, staff->clearance);
        return;
    }

    printf("\n  Enter patient ID: ");
    int pid;
    if (scanf("%d", &pid) != 1) { while (getchar() != '\n'); return; }
    while (getchar() != '\n');

    const PatientRecord *p = find_patient_by_id(pid);
    if (!p) {
        printf("  [ERROR] No patient found with ID %d.\n", pid);
        return;
    }

    /* Second-factor confirmation for controlled substances */
    char confirm[8];
    printf("  [CONTROLLED SUBSTANCE] Confirm dispensing for %s? (yes/no): ",
           p->patient_name);
    if (!fgets(confirm, sizeof(confirm), stdin)) return;
    strip_newline(confirm);
    if (!str_iequal(confirm, "yes")) {
        printf("  Dispensing cancelled.\n");
        return;
    }

    printf("\n  Available controlled substances:\n");
    for (int i = 0; i < CTRL_MED_COUNT; i++)
        printf("    %2d. %s\n", i + 1, CTRL_MEDS[i]);

    printf("\n  Select medication (1-%d): ", CTRL_MED_COUNT);
    int choice;
    if (scanf("%d", &choice) != 1) { while (getchar() != '\n'); return; }
    while (getchar() != '\n');

    if (choice < 1 || choice > CTRL_MED_COUNT) {
        printf("  [ERROR] Invalid selection.\n");
        return;
    }

    const char *med = CTRL_MEDS[choice - 1];
    printf("\n  [OK] Dispensing controlled substance \"%s\" for patient %s (ID: %d).\n",
           med, p->patient_name, p->patient_id);

    char detail[128];
    snprintf(detail, sizeof(detail),
             "Dispensed controlled substance: %s", med);
    log_action(staff, p->patient_id, p->patient_name, "DISPENSE_CTRL", detail);
    printf("  Action logged.\n");
}

/* ══════════════════════════════════════════════════════════════════════════════
 * Session loop
 * ══════════════════════════════════════════════════════════════════════════════ */

static void run_session(const StaffMember *staff) {
    char ts[32];
    timestamp(ts, sizeof(ts));

    printf("\n");
    separator();
    printf("  SESSION STARTED  %s\n", ts);
    printf("  Staff  : %s\n", staff->name);
    printf("  Dept   : %s\n", staff->department);
    printf("  Level  : %d\n", staff->clearance);
    separator();

    char menu_choice[8];
    while (1) {
        printf("\n  MAIN MENU\n");
        printf("  ─────────\n");
        printf("  [1] View patient medication list     (clearance ≥ %d)\n",
               CLEARANCE_VIEW_MEDS);
        printf("  [2] Dispense standard medication     (clearance ≥ %d)\n",
               CLEARANCE_DISPENSE_STD);
        printf("  [3] Dispense controlled substance    (clearance ≥ %d)\n",
               CLEARANCE_DISPENSE_CTRL);
        printf("  [0] End session\n");
        printf("\n  Select option: ");

        if (!fgets(menu_choice, sizeof(menu_choice), stdin)) break;
        strip_newline(menu_choice);

        if (strcmp(menu_choice, "0") == 0) {
            printf("\n  Session ended. Goodbye, %s.\n\n", staff->name);
            break;
        } else if (strcmp(menu_choice, "1") == 0) {
            op_view_meds(staff);
        } else if (strcmp(menu_choice, "2") == 0) {
            op_dispense_std(staff);
        } else if (strcmp(menu_choice, "3") == 0) {
            op_dispense_ctrl(staff);
        } else {
            printf("  [ERROR] Unknown option '%s'.\n", menu_choice);
        }
    }
}

/* ══════════════════════════════════════════════════════════════════════════════
 * Entry point
 * ══════════════════════════════════════════════════════════════════════════════ */

int main(void) {
    printf("\n");
    printf("╔═════════════════════════════════════════════════════╗\n");
    printf("║     HOSPITAL MEDICATION DISPENSING TERMINAL v1.0   ║\n");
    printf("╚═════════════════════════════════════════════════════╝\n");
    printf("  All dispensing actions are logged to: %s\n", LOG_FILE);

    char name_buf[MAX_NAME_LEN];

    while (1) {
        printf("\n  Enter your full name (or 'quit' to exit): ");
        if (!fgets(name_buf, sizeof(name_buf), stdin)) break;
        strip_newline(name_buf);

        if (str_iequal(name_buf, "quit")) {
            printf("  Terminal shutting down.\n\n");
            break;
        }

        if (strlen(name_buf) == 0) continue;

        const StaffMember *staff = find_staff(name_buf);
        if (!staff) {
            printf("  [ERROR] No staff record found for \"%s\".\n", name_buf);
            printf("          Please check your name and try again.\n");
            continue;
        }

        run_session(staff);
    }

    return EXIT_SUCCESS;
}