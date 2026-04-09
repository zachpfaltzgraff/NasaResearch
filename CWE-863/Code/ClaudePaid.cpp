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

/* ── Clearance thresholds ─────────────────────────────────────────────────── */
#define CLEARANCE_VIEW_MEDS        1   /* view a patient's medication list    */
#define CLEARANCE_DISPENSE_STD     2   /* dispense a standard medication      */
#define CLEARANCE_DISPENSE_CTRL    4   /* dispense a controlled substance     */

/* ── Misc constants ───────────────────────────────────────────────────────── */
#define MAX_NAME_LEN   64
#define LOG_FILE       "dispense_log.txt"

/* ── Data types ───────────────────────────────────────────────────────────── */
typedef struct {
    const char *name;
    const char *department;
    int         clearance;
} StaffMember;

typedef struct {
    int         patient_id;
    const char *name;
    const char *medications[4];   /* current standard meds (NULL-terminated) */
} Patient;

/* ── Staff roster (fixed array) ───────────────────────────────────────────── */
static const StaffMember ROSTER[] = {
    { "Alice Nguyen",    "ICU",       5 },
    { "Bob Okafor",      "Pharmacy",  4 },
    { "Carmen Reyes",    "Oncology",  3 },
    { "David Park",      "General",   2 },
    { "Elena Vasquez",   "Radiology", 1 },
    { "Frank Morrison",  "Admin",     0 },
};
static const int ROSTER_SIZE = (int)(sizeof(ROSTER) / sizeof(ROSTER[0]));

/* ── Standard medications catalogue ──────────────────────────────────────── */
static const char *STANDARD_MEDS[] = {
    "Ibuprofen 400mg",
    "Amoxicillin 500mg",
    "Lisinopril 10mg",
    "Metformin 850mg",
    "Atorvastatin 20mg",
    NULL
};

/* ── Controlled substances catalogue ─────────────────────────────────────── */
static const char *CONTROLLED_MEDS[] = {
    "Morphine 10mg",
    "Oxycodone 5mg",
    "Midazolam 2mg",
    "Fentanyl 50mcg patch",
    NULL
};

/* ── Patient registry (demo data) ────────────────────────────────────────── */
static const Patient PATIENTS[] = {
    { 1001, "John Smith",    { "Lisinopril 10mg", "Metformin 850mg", NULL } },
    { 1002, "Maria Garcia",  { "Ibuprofen 400mg", "Atorvastatin 20mg", "Amoxicillin 500mg", NULL } },
    { 1003, "Wei Zhang",     { "Atorvastatin 20mg", NULL } },
};
static const int PATIENT_COUNT = (int)(sizeof(PATIENTS) / sizeof(PATIENTS[0]));

/* ════════════════════════════════════════════════════════════════════════════
 * Utility helpers
 * ════════════════════════════════════════════════════════════════════════════ */

/* Strip trailing newline left by fgets(). */
static void strip_newline(char *s)
{
    size_t len = strlen(s);
    if (len > 0 && s[len - 1] == '\n')
        s[len - 1] = '\0';
}

static int istr_eq(const char *a, const char *b)
{
    while (*a && *b) {
        if ((*a | 32) != (*b | 32))
            return 0;
        a++; b++;
    }
    return *a == '\0' && *b == '\0';
}
/* ════════════════════════════════════════════════════════════════════════════
 * Logging
 * ════════════════════════════════════════════════════════════════════════════ */

static void log_action(const StaffMember *staff,
                        const char        *action,
                        int                patient_id,
                        const char        *detail)
{
    FILE *fp = fopen(LOG_FILE, "a");
    if (!fp) {
        fprintf(stderr, "[WARNING] Could not open log file '%s'.\n", LOG_FILE);
        return;
    }

    time_t     now = time(NULL);
    struct tm *tm  = localtime(&now);
    char       ts[32];
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", tm);

    fprintf(fp, "[%s] STAFF=\"%s\" DEPT=%s CLR=%d ACTION=%s PATIENT_ID=%d DETAIL=\"%s\"\n",
            ts,
            staff->name,
            staff->department,
            staff->clearance,
            action,
            patient_id,
            detail ? detail : "N/A");

    fclose(fp);
}

/* ════════════════════════════════════════════════════════════════════════════
 * Staff lookup
 * ════════════════════════════════════════════════════════════════════════════ */

static const StaffMember *find_staff(const char *name)
{
    for (int i = 0; i < ROSTER_SIZE; i++) {
        if (istr_eq(ROSTER[i].name, name))
            return &ROSTER[i];
    }
    return NULL;
}

/* ════════════════════════════════════════════════════════════════════════════
 * Patient lookup
 * ════════════════════════════════════════════════════════════════════════════ */

static const Patient *find_patient_by_id(int id)
{
    for (int i = 0; i < PATIENT_COUNT; i++) {
        if (PATIENTS[i].patient_id == id)
            return &PATIENTS[i];
    }
    return NULL;
}

/* ════════════════════════════════════════════════════════════════════════════
 * Terminal operations
 * ════════════════════════════════════════════════════════════════════════════ */

/* ── Operation 1: View patient medication list ───────────────────────────── */
static void op_view_meds(const StaffMember *staff)
{
    if (staff->clearance < CLEARANCE_VIEW_MEDS) {
        printf("  [ACCESS DENIED] Clearance %d required (yours: %d).\n",
               CLEARANCE_VIEW_MEDS, staff->clearance);
        return;
    }

    printf("  Enter patient ID: ");
    int pid;
    if (scanf("%d", &pid) != 1) { while (getchar() != '\n'); return; }
    while (getchar() != '\n'); /* flush */

    const Patient *p = find_patient_by_id(pid);
    if (!p) {
        printf("  [ERROR] No patient found with ID %d.\n", pid);
        return;
    }

    printf("\n  Patient #%d — %s\n", p->patient_id, p->name);
    printf("  Current medication list:\n");
    int count = 0;
    for (int i = 0; p->medications[i] != NULL; i++) {
        printf("    %d. %s\n", i + 1, p->medications[i]);
        count++;
    }
    if (count == 0)
        printf("    (none on record)\n");

    log_action(staff, "VIEW_MEDS", pid, p->name);
    printf("\n  Record accessed and logged.\n");
}

/* ── Shared catalogue printer & picker ──────────────────────────────────── */
static const char *pick_from_catalogue(const char **catalogue, const char *label)
{
    printf("\n  Available %s:\n", label);
    int count = 0;
    for (int i = 0; catalogue[i] != NULL; i++) {
        printf("    %d. %s\n", i + 1, catalogue[i]);
        count++;
    }
    printf("  Select (1-%d): ", count);

    int choice;
    if (scanf("%d", &choice) != 1) { while (getchar() != '\n'); return NULL; }
    while (getchar() != '\n');

    if (choice < 1 || choice > count) {
        printf("  [ERROR] Invalid selection.\n");
        return NULL;
    }
    return catalogue[choice - 1];
}

/* ── Operation 2: Dispense standard medication ───────────────────────────── */
static void op_dispense_standard(const StaffMember *staff)
{
    if (staff->clearance < CLEARANCE_DISPENSE_STD) {
        printf("  [ACCESS DENIED] Clearance %d required (yours: %d).\n",
               CLEARANCE_DISPENSE_STD, staff->clearance);
        return;
    }

    printf("  Enter patient ID: ");
    int pid;
    if (scanf("%d", &pid) != 1) { while (getchar() != '\n'); return; }
    while (getchar() != '\n');

    const Patient *p = find_patient_by_id(pid);
    if (!p) {
        printf("  [ERROR] No patient found with ID %d.\n", pid);
        return;
    }

    const char *med = pick_from_catalogue(STANDARD_MEDS, "standard medications");
    if (!med) return;

    printf("\n  Dispensing [%s] to patient %s (#%d)...\n", med, p->name, pid);
    log_action(staff, "DISPENSE_STD", pid, med);
    printf("  Done. Action logged.\n");
}

/* ── Operation 3: Dispense controlled substance ─────────────────────────── */
static void op_dispense_controlled(const StaffMember *staff)
{
    if (staff->clearance < CLEARANCE_DISPENSE_CTRL) {
        printf("  [ACCESS DENIED] Clearance %d required (yours: %d).\n",
               CLEARANCE_DISPENSE_CTRL, staff->clearance);
        return;
    }

    printf("  Enter patient ID: ");
    int pid;
    if (scanf("%d", &pid) != 1) { while (getchar() != '\n'); return; }
    while (getchar() != '\n');

    const Patient *p = find_patient_by_id(pid);
    if (!p) {
        printf("  [ERROR] No patient found with ID %d.\n", pid);
        return;
    }

    /* Extra confirmation step for controlled substances */
    printf("  ** CONTROLLED SUBSTANCE PROTOCOL **\n");
    printf("  Confirm dispense by entering your name again: ");
    char confirm[MAX_NAME_LEN];
    if (!fgets(confirm, sizeof(confirm), stdin)) return;
    strip_newline(confirm);

    if (!istr_eq(confirm, staff->name)) {
        printf("  [ABORTED] Name confirmation failed.\n");
        log_action(staff, "CTRL_CONFIRM_FAIL", pid, "Name mismatch");
        return;
    }

    const char *med = pick_from_catalogue(CONTROLLED_MEDS, "controlled substances");
    if (!med) return;

    printf("\n  Dispensing [%s] to patient %s (#%d)...\n", med, p->name, pid);
    log_action(staff, "DISPENSE_CTRL", pid, med);
    printf("  Done. Action logged.\n");
}

/* ════════════════════════════════════════════════════════════════════════════
 * Session loop
 * ════════════════════════════════════════════════════════════════════════════ */

static void run_session(const StaffMember *staff)
{
    printf("\n");
    printf("  ┌─────────────────────────────────────────┐\n");
    printf("  │  SESSION STARTED                        │\n");
    printf("  │  Staff   : %-29s│\n", staff->name);
    printf("  │  Dept    : %-29s│\n", staff->department);
    printf("  │  Clearance level : %-20d│\n", staff->clearance);
    printf("  └─────────────────────────────────────────┘\n");

    int running = 1;
    while (running) {
        printf("\n");
        printf("  ┌── MAIN MENU ─────────────────────────────┐\n");
        printf("  │  1. View patient medication list  [CLR≥%d] │\n", CLEARANCE_VIEW_MEDS);
        printf("  │  2. Dispense standard medication  [CLR≥%d] │\n", CLEARANCE_DISPENSE_STD);
        printf("  │  3. Dispense controlled substance [CLR≥%d] │\n", CLEARANCE_DISPENSE_CTRL);
        printf("  │  0. End session                           │\n");
        printf("  └───────────────────────────────────────────┘\n");
        printf("  Choice: ");

        int choice;
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            printf("  [ERROR] Invalid input.\n");
            continue;
        }
        while (getchar() != '\n');
        printf("\n");

        switch (choice) {
            case 1: op_view_meds(staff);          break;
            case 2: op_dispense_standard(staff);  break;
            case 3: op_dispense_controlled(staff); break;
            case 0:
                printf("  Session ended. Goodbye, %s.\n", staff->name);
                log_action(staff, "SESSION_END", 0, NULL);
                running = 0;
                break;
            default:
                printf("  [ERROR] Unknown option '%d'.\n", choice);
        }
    }
}

/* ════════════════════════════════════════════════════════════════════════════
 * Entry point
 * ════════════════════════════════════════════════════════════════════════════ */

int main(void)
{
    printf("╔══════════════════════════════════════════════════╗\n");
    printf("║   HOSPITAL MEDICATION DISPENSING TERMINAL v1.0  ║\n");
    printf("╚══════════════════════════════════════════════════╝\n");
    printf("  Log file: %s\n\n", LOG_FILE);

    char name[MAX_NAME_LEN];

    while (1) {
        printf("Enter staff name (or 'quit' to shut down): ");
        if (!fgets(name, sizeof(name), stdin))
            break;
        strip_newline(name);

        if (istr_eq(name, "quit")) {
            printf("Terminal shutting down.\n");
            break;
        }

        if (name[0] == '\0')
            continue;

        const StaffMember *staff = find_staff(name);
        if (!staff) {
            printf("  [ERROR] '%s' not found in staff roster.\n\n", name);
            continue;
        }

        if (staff->clearance == 0) {
            printf("  [ACCESS DENIED] %s has no terminal clearance.\n\n", staff->name);
            log_action(staff, "LOGIN_DENIED", 0, "Clearance 0");
            continue;
        }

        log_action(staff, "SESSION_START", 0, NULL);
        run_session(staff);
        printf("\n");
    }

    return 0;
}