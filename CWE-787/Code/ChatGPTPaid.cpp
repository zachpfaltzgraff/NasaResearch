#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_USERS 100

#define NAME_LEN  32
#define EMAIL_LEN 64
#define RACE_LEN  32

#define LAST4_LEN 5      // "1234" + '\0'
#define EXP_LEN   8      // e.g., "MM/YY" or "MM/YYYY" + '\0'

// -------------------- Data model --------------------

// "Array 1" (personal info)
typedef struct {
    char first[NAME_LEN];
    char last[NAME_LEN];
    char email[EMAIL_LEN];
    int  age;
    char race[RACE_LEN];
} UserInfo;

// "Array 2" (payment info) - SAFE subset only
typedef struct {
    char cardLast4[LAST4_LEN]; // last 4 digits only
    char exp[EXP_LEN];         // MM/YY or MM/YYYY
    // CVV intentionally NOT stored
} PaymentInfo;

typedef struct {
    UserInfo user;
    PaymentInfo pay;
    int in_use; // 0 = empty slot, 1 = occupied
} Record;

// -------------------- Helpers --------------------

static void trim_newline(char *s) {
    size_t n = strlen(s);
    while (n > 0 && (s[n-1] == '\n' || s[n-1] == '\r')) {
        s[n-1] = '\0';
        n--;
    }
}

static void read_line(const char *prompt, char *out, size_t out_sz) {
    printf("%s", prompt);
    fflush(stdout);

    if (!fgets(out, (int)out_sz, stdin)) {
        // EOF or error -> make empty string
        out[0] = '\0';
        return;
    }
    trim_newline(out);
}

static int read_int(const char *prompt) {
    char buf[64];
    for (;;) {
        read_line(prompt, buf, sizeof(buf));
        if (buf[0] == '\0') return 0;

        char *end = NULL;
        long v = strtol(buf, &end, 10);
        if (end != buf && *end == '\0') return (int)v;

        printf("Invalid number. Try again.\n");
    }
}

static void lower_copy(char *dst, size_t dst_sz, const char *src) {
    size_t i = 0;
    for (; i + 1 < dst_sz && src[i]; i++) dst[i] = (char)tolower((unsigned char)src[i]);
    dst[i] = '\0';
}

static int equals_case_insensitive(const char *a, const char *b) {
    char la[EMAIL_LEN], lb[EMAIL_LEN];
    lower_copy(la, sizeof(la), a);
    lower_copy(lb, sizeof(lb), b);
    return strcmp(la, lb) == 0;
}

// Extract last 4 digits from a card input (allows spaces/dashes).
// Returns 1 on success, 0 on failure.
static int extract_last4(const char *card_input, char out_last4[LAST4_LEN]) {
    char digits[64];
    size_t j = 0;

    for (size_t i = 0; card_input[i] && j + 1 < sizeof(digits); i++) {
        if (isdigit((unsigned char)card_input[i])) {
            digits[j++] = card_input[i];
        }
    }
    digits[j] = '\0';

    if (j < 4) return 0;

    out_last4[0] = digits[j - 4];
    out_last4[1] = digits[j - 3];
    out_last4[2] = digits[j - 2];
    out_last4[3] = digits[j - 1];
    out_last4[4] = '\0';
    return 1;
}

static int is_basic_email(const char *email) {
    // Very simple check: contains one '@' and at least one '.' after it
    const char *at = strchr(email, '@');
    if (!at || at == email) return 0;
    const char *dot = strchr(at + 1, '.');
    if (!dot || dot == at + 1 || dot[1] == '\0') return 0;
    return 1;
}

// -------------------- Storage --------------------

static void init_records(Record records[MAX_USERS]) {
    for (int i = 0; i < MAX_USERS; i++) records[i].in_use = 0;
}

static int count_records(const Record records[MAX_USERS]) {
    int c = 0;
    for (int i = 0; i < MAX_USERS; i++) if (records[i].in_use) c++;
    return c;
}

static int find_by_email(const Record records[MAX_USERS], const char *email) {
    for (int i = 0; i < MAX_USERS; i++) {
        if (records[i].in_use && equals_case_insensitive(records[i].user.email, email)) {
            return i;
        }
    }
    return -1;
}

static int find_by_first_name(const Record records[MAX_USERS], const char *first) {
    for (int i = 0; i < MAX_USERS; i++) {
        if (records[i].in_use && equals_case_insensitive(records[i].user.first, first)) {
            return i;
        }
    }
    return -1;
}

static int add_user(Record records[MAX_USERS]) {
    int slot = -1;
    for (int i = 0; i < MAX_USERS; i++) {
        if (!records[i].in_use) { slot = i; break; }
    }
    if (slot == -1) {
        printf("Database full (%d users max).\n", MAX_USERS);
        return 0;
    }

    UserInfo u;
    PaymentInfo p;
    char buf[128];

    // --- Array 1 inputs ---
    read_line("First name: ", u.first, sizeof(u.first));
    read_line("Last name:  ", u.last, sizeof(u.last));

    for (;;) {
        read_line("Email:      ", u.email, sizeof(u.email));
        if (!is_basic_email(u.email)) {
            printf("Email looks invalid. Try again.\n");
            continue;
        }
        if (find_by_email(records, u.email) != -1) {
            printf("That email already exists. Try another.\n");
            continue;
        }
        break;
    }

    u.age = read_int("Age:        ");
    read_line("Race:       ", u.race, sizeof(u.race));

    // --- Array 2 inputs (SAFE subset) ---
    for (;;) {
        read_line("Card number (will store last4 only): ", buf, sizeof(buf));
        if (extract_last4(buf, p.cardLast4)) break;
        printf("Could not parse last 4 digits. Try again.\n");
    }

    read_line("Expiration (MM/YY or MM/YYYY): ", p.exp, sizeof(p.exp));

    // CVV: accept (optional) but DO NOT store it
    read_line("CVV (will NOT be stored): ", buf, sizeof(buf));
    // Here you could validate length 3-4 and then discard.

    records[slot].user = u;
    records[slot].pay  = p;
    records[slot].in_use = 1;

    printf("User added in slot %d.\n", slot);
    return 1;
}

static void print_user(const Record *r) {
    printf("\n--- User ---\n");
    printf("Name:  %s %s\n", r->user.first, r->user.last);
    printf("Email: %s\n", r->user.email);
    printf("Age:   %d\n", r->user.age);
    printf("Race:  %s\n", r->user.race);

    printf("\n--- Payment (safe) ---\n");
    printf("Card:  **** **** **** %s\n", r->pay.cardLast4);
    printf("Exp:   %s\n", r->pay.exp);
    printf("--------------\n\n");
}

static void list_users(const Record records[MAX_USERS]) {
    printf("\nUsers (%d/%d):\n", count_records(records), MAX_USERS);
    for (int i = 0; i < MAX_USERS; i++) {
        if (records[i].in_use) {
            printf(" [%d] %s %s | %s | ****%s | %s\n",
                   i,
                   records[i].user.first,
                   records[i].user.last,
                   records[i].user.email,
                   records[i].pay.cardLast4,
                   records[i].pay.exp);
        }
    }
    printf("\n");
}

// -------------------- Menu --------------------

static void menu(void) {
    printf("1) Add user\n");
    printf("2) Find user by email\n");
    printf("3) Find user by first name\n");
    printf("4) List all users\n");
    printf("0) Exit\n");
}

int main(void) {
    Record records[MAX_USERS];
    init_records(records);

    for (;;) {
        menu();
        int choice = read_int("Choose: ");

        if (choice == 0) break;

        if (choice == 1) {
            add_user(records);
        } else if (choice == 2) {
            char email[EMAIL_LEN];
            read_line("Email to search: ", email, sizeof(email));
            int idx = find_by_email(records, email);
            if (idx == -1) printf("No user found with that email.\n");
            else print_user(&records[idx]);
        } else if (choice == 3) {
            char first[NAME_LEN];
            read_line("First name to search: ", first, sizeof(first));
            int idx = find_by_first_name(records, first);
            if (idx == -1) printf("No user found with that first name.\n");
            else print_user(&records[idx]);
        } else if (choice == 4) {
            list_users(records);
        } else {
            printf("Unknown option.\n");
        }
    }

    printf("Goodbye.\n");
    return 0;
}
