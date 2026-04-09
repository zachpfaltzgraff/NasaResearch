/*
  Hospital Medication Dispensing Terminal (demo)

  - Fixed in-memory staff roster (name, department code, clearance)
  - Staff login by name (exact match)
  - Three operations:
      1) View a patient's current medication list
      2) Dispense a standard medication
      3) Dispense a controlled substance
  - Minimum clearance requirements are constants below
  - Every dispensing action is appended to a log file including who and what
*/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#define STAFF_COUNT 6
#define NAME_MAX    64
#define DEPT_MAX    16
#define MED_MAX     80
#define PAT_MAX     64

/* Clearance thresholds (edit as needed) */
#define CLEAR_VIEW_MEDLIST        1
#define CLEAR_DISPENSE_STANDARD   3
#define CLEAR_DISPENSE_CONTROLLED 7

#define DISPENSE_LOG_FILE "dispense.log"

typedef struct {
    char name[NAME_MAX];
    char dept[DEPT_MAX];
    int clearance;
} Staff;

/* Fixed roster */
static const Staff roster[STAFF_COUNT] = {
    {"Alex Kim",        "ED",    7},
    {"Morgan Patel",    "PHARM", 8},
    {"Casey Nguyen",    "ICU",   5},
    {"Jordan Smith",    "MED",   3},
    {"Taylor Johnson",  "SURG",  4},
    {"Riley Brown",     "ADMIN", 2}
};

static void trim_newline(char *s) {
    size_t n = strlen(s);
    if (n > 0 && s[n - 1] == '\n') s[n - 1] = '\0';
}

static void read_line(const char *prompt, char *buf, size_t buflen) {
    if (prompt) {
        fputs(prompt, stdout);
        fflush(stdout);
    }
    if (!fgets(buf, (int)buflen, stdin)) {
        /* EOF or error: exit cleanly */
        puts("\nInput closed. Exiting.");
        exit(0);
    }
    trim_newline(buf);
}

static const Staff* find_staff_by_name(const char *name) {
    for (int i = 0; i < STAFF_COUNT; i++) {
        if (strcmp(roster[i].name, name) == 0) return &roster[i];
    }
    return NULL;
}

static void now_iso8601(char *out, size_t outlen) {
    time_t t = time(NULL);
    struct tm tmv;
#if defined(_WIN32)
    localtime_s(&tmv, &t);
#else
    localtime_r(&t, &tmv);
#endif
    strftime(out, outlen, "%Y-%m-%dT%H:%M:%S%z", &tmv);
}

static void log_dispense(const Staff *staff,
                         const char *patient_id,
                         const char *med_name,
                         const char *med_type) {
    FILE *fp = fopen(DISPENSE_LOG_FILE, "a");
    if (!fp) {
        perror("ERROR: could not open log file for append");
        return; /* still allow operation, but warn */
    }

    char ts[40];
    now_iso8601(ts, sizeof(ts));

    /* Log format: timestamp | staff | dept | clearance | patient | type | medication */
    fprintf(fp, "%s | staff=\"%s\" | dept=%s | clearance=%d | patient=\"%s\" | type=%s | med=\"%s\"\n",
            ts,
            staff->name,
            staff->dept,
            staff->clearance,
            patient_id,
            med_type,
            med_name);

    fclose(fp);
}

static int has_clearance(const Staff *staff, int required) {
    return staff->clearance >= required;
}

/* Demo medication list (in a real system this would be loaded from a secure EMR) */
static void view_med_list(const Staff *staff) {
    if (!has_clearance(staff, CLEAR_VIEW_MEDLIST)) {
        printf("Access denied: requires clearance >= %d.\n", CLEAR_VIEW_MEDLIST);
        return;
    }

    char patient[PAT_MAX];
    read_line("Enter patient identifier: ", patient, sizeof(patient));

    /* Demo output */
    printf("\nPatient \"%s\" current medication list (demo):\n", patient);
    printf(" - Lisinopril 10mg (daily)\n");
    printf(" - Metformin 500mg (BID)\n");
    printf(" - Acetaminophen 500mg (PRN)\n\n");
}

static void dispense_standard(const Staff *staff) {
    if (!has_clearance(staff, CLEAR_DISPENSE_STANDARD)) {
        printf("Access denied: requires clearance >= %d.\n", CLEAR_DISPENSE_STANDARD);
        return;
    }

    char patient[PAT_MAX];
    char med[MED_MAX];

    read_line("Enter patient identifier: ", patient, sizeof(patient));
    read_line("Enter standard medication to dispense: ", med, sizeof(med));

    printf("Dispensed STANDARD medication \"%s\" to patient \"%s\".\n", med, patient);
    log_dispense(staff, patient, med, "STANDARD");
}

static void dispense_controlled(const Staff *staff) {
    if (!has_clearance(staff, CLEAR_DISPENSE_CONTROLLED)) {
        printf("Access denied: requires clearance >= %d.\n", CLEAR_DISPENSE_CONTROLLED);
        return;
    }

    char patient[PAT_MAX];
    char med[MED_MAX];

    read_line("Enter patient identifier: ", patient, sizeof(patient));
    read_line("Enter CONTROLLED substance to dispense: ", med, sizeof(med));

    /* You could add extra safeguards here: 2-person verification, reason code, quantity, etc. */
    printf("Dispensed CONTROLLED substance \"%s\" to patient \"%s\".\n", med, patient);
    log_dispense(staff, patient, med, "CONTROLLED");
}

static void session_menu(const Staff *staff) {
    for (;;) {
        printf("\n--- Medication Terminal ---\n");
        printf("Logged in as: %s (dept=%s, clearance=%d)\n", staff->name, staff->dept, staff->clearance);
        printf("1) View patient's medication list (requires %d)\n", CLEAR_VIEW_MEDLIST);
        printf("2) Dispense standard medication (requires %d)\n", CLEAR_DISPENSE_STANDARD);
        printf("3) Dispense controlled substance (requires %d)\n", CLEAR_DISPENSE_CONTROLLED);
        printf("4) Logout\n");

        char choice[16];
        read_line("Select an option: ", choice, sizeof(choice));

        if (strcmp(choice, "1") == 0) {
            view_med_list(staff);
        } else if (strcmp(choice, "2") == 0) {
            dispense_standard(staff);
        } else if (strcmp(choice, "3") == 0) {
            dispense_controlled(staff);
        } else if (strcmp(choice, "4") == 0) {
            puts("Logging out...");
            return;
        } else {
            puts("Invalid option.");
        }
    }
}

int main(void) {
    puts("Hospital Medication Dispensing Terminal");
    puts("(Demo program — not for real clinical use)");

    for (;;) {
        char name[NAME_MAX];
        read_line("\nEnter staff name to login (or blank to exit): ", name, sizeof(name));
        if (name[0] == '\0') {
            puts("Goodbye.");
            break;
        }

        const Staff *staff = find_staff_by_name(name);
        if (!staff) {
            puts("No such staff member (name must match roster exactly).");
            puts("Try one of: Alex Kim, Morgan Patel, Casey Nguyen, Jordan Smith, Taylor Johnson, Riley Brown");
            continue;
        }

        session_menu(staff);
    }

    return 0;
}