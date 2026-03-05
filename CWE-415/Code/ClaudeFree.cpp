/*
 * bank_simulation.c
 *
 * Simulates concurrent deposits and withdrawals on 10 shared bank accounts
 * using POSIX threads (pthreads) with mutex locks for thread safety.
 *
 * Compile:  gcc -o bank_simulation bank_simulation.c -lpthread
 * Run:      ./bank_simulation
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <string.h>

/* ─── Constants ─────────────────────────────────────────────── */
#define NUM_ACCOUNTS     10
#define NUM_THREADS      4          /* worker threads */
#define INITIAL_BALANCE  1000.00    /* starting balance per account ($) */
#define MAX_AMOUNT       500.00     /* maximum single transaction amount ($) */

/* ─── Shared State ──────────────────────────────────────────── */
typedef struct {
    double         balance;
    int            account_id;
    pthread_mutex_t lock;
    int            total_deposits;
    int            total_withdrawals;
    int            failed_withdrawals;
} Account;

Account accounts[NUM_ACCOUNTS];

/* Remaining transactions counter (shared across threads) */
int      transactions_remaining;
pthread_mutex_t counter_lock = PTHREAD_MUTEX_INITIALIZER;

/* Completed-transaction log (optional display) */
#define LOG_SIZE 10000
typedef struct {
    int    thread_id;
    int    account_id;
    char   type[12];   /* "DEPOSIT" or "WITHDRAWAL" */
    double amount;
    double new_balance;
    int    success;
} LogEntry;

LogEntry txn_log[LOG_SIZE];
int      log_index = 0;
pthread_mutex_t log_lock = PTHREAD_MUTEX_INITIALIZER;

/* ─── Helpers ───────────────────────────────────────────────── */
/* Thread-safe random double in [0, max] */
double rand_amount(unsigned int *seed) {
    return ((double)rand_r(seed) / RAND_MAX) * MAX_AMOUNT;
}

void log_transaction(int tid, int aid, const char *type,
                     double amount, double bal, int ok) {
    pthread_mutex_lock(&log_lock);
    if (log_index < LOG_SIZE) {
        txn_log[log_index].thread_id   = tid;
        txn_log[log_index].account_id  = aid;
        strncpy(txn_log[log_index].type, type, 11);
        txn_log[log_index].type[11]    = '\0';
        txn_log[log_index].amount      = amount;
        txn_log[log_index].new_balance = bal;
        txn_log[log_index].success     = ok;
        log_index++;
    }
    pthread_mutex_unlock(&log_lock);
}

/* ─── Worker Thread ─────────────────────────────────────────── */
void *worker(void *arg) {
    int          tid  = *(int *)arg;
    unsigned int seed = (unsigned int)(time(NULL) ^ (tid * 2654435761u));

    while (1) {
        /* Claim one transaction slot */
        pthread_mutex_lock(&counter_lock);
        if (transactions_remaining <= 0) {
            pthread_mutex_unlock(&counter_lock);
            break;
        }
        transactions_remaining--;
        pthread_mutex_unlock(&counter_lock);

        /* Pick a random account and amount */
        int    aid    = rand_r(&seed) % NUM_ACCOUNTS;
        double amount = rand_amount(&seed);
        int    is_dep = rand_r(&seed) % 2;   /* 0 = withdrawal, 1 = deposit */

        pthread_mutex_lock(&accounts[aid].lock);

        if (is_dep) {
            accounts[aid].balance += amount;
            accounts[aid].total_deposits++;
            log_transaction(tid, aid, "DEPOSIT",
                            amount, accounts[aid].balance, 1);
        } else {
            if (accounts[aid].balance >= amount) {
                accounts[aid].balance -= amount;
                accounts[aid].total_withdrawals++;
                log_transaction(tid, aid, "WITHDRAWAL",
                                amount, accounts[aid].balance, 1);
            } else {
                /* Insufficient funds – skip, but record attempt */
                accounts[aid].failed_withdrawals++;
                log_transaction(tid, aid, "WITHDRAWAL",
                                amount, accounts[aid].balance, 0);
            }
        }

        pthread_mutex_unlock(&accounts[aid].lock);
    }

    return NULL;
}

/* ─── Initialise Accounts ───────────────────────────────────── */
void init_accounts(void) {
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        accounts[i].account_id        = i + 1;
        accounts[i].balance           = INITIAL_BALANCE;
        accounts[i].total_deposits    = 0;
        accounts[i].total_withdrawals = 0;
        accounts[i].failed_withdrawals = 0;
        pthread_mutex_init(&accounts[i].lock, NULL);
    }
}

/* ─── Print Results ─────────────────────────────────────────── */
void print_summary(int total_txns) {
    double total_bal    = 0.0;
    int    tot_dep      = 0;
    int    tot_with     = 0;
    int    tot_failed   = 0;

    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║            BANK SIMULATION — FINAL ACCOUNT SUMMARY          ║\n");
    printf("╠══════╦══════════════╦════════════╦═════════════╦════════════╣\n");
    printf("║  ID  ║    Balance   ║  Deposits  ║ Withdrawals ║  Failed W. ║\n");
    printf("╠══════╬══════════════╬════════════╬═════════════╬════════════╣\n");

    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        printf("║  %2d  ║  $%9.2f ║  %8d  ║   %8d  ║  %8d  ║\n",
               accounts[i].account_id,
               accounts[i].balance,
               accounts[i].total_deposits,
               accounts[i].total_withdrawals,
               accounts[i].failed_withdrawals);

        total_bal  += accounts[i].balance;
        tot_dep    += accounts[i].total_deposits;
        tot_with   += accounts[i].total_withdrawals;
        tot_failed += accounts[i].failed_withdrawals;
    }

    printf("╠══════╬══════════════╬════════════╬═════════════╬════════════╣\n");
    printf("║ TOT  ║  $%9.2f ║  %8d  ║   %8d  ║  %8d  ║\n",
           total_bal, tot_dep, tot_with, tot_failed);
    printf("╚══════╩══════════════╩════════════╩═════════════╩════════════╝\n");

    printf("\n  Transactions requested : %d\n", total_txns);
    printf("  Successful deposits    : %d\n", tot_dep);
    printf("  Successful withdrawals : %d\n", tot_with);
    printf("  Failed withdrawals     : %d  (insufficient funds)\n", tot_failed);
    printf("  Total processed        : %d\n", tot_dep + tot_with + tot_failed);
    printf("  Initial total balance  : $%.2f\n",
           NUM_ACCOUNTS * INITIAL_BALANCE);
    printf("  Final   total balance  : $%.2f\n\n", total_bal);
}

/* Print last N log entries */
void print_log(int n) {
    int start = log_index - n;
    if (start < 0) start = 0;

    printf("  ── Last %d transactions ──\n", log_index - start);
    printf("  %-8s %-8s %-12s %-10s %-12s %s\n",
           "Thread", "Account", "Type", "Amount", "New Bal", "Status");
    printf("  %s\n",
           "────────────────────────────────────────────────────────────");
    for (int i = start; i < log_index; i++) {
        printf("  T%-7d A%-7d %-12s $%-9.2f $%-11.2f %s\n",
               txn_log[i].thread_id,
               txn_log[i].account_id,
               txn_log[i].type,
               txn_log[i].amount,
               txn_log[i].new_balance,
               txn_log[i].success ? "OK" : "INSUFFICIENT FUNDS");
    }
    printf("\n");
}

/* ─── Main ──────────────────────────────────────────────────── */
int main(void) {
    int total_txns;

    printf("\n");
    printf("  ┌─────────────────────────────────────────┐\n");
    printf("  │  Concurrent Bank Account Simulation     │\n");
    printf("  │  %d accounts · %d worker threads · pthreads │\n",
           NUM_ACCOUNTS, NUM_THREADS);
    printf("  └─────────────────────────────────────────┘\n\n");
    printf("  Each account starts with $%.2f.\n", INITIAL_BALANCE);
    printf("  Threads will randomly deposit or withdraw up to $%.2f.\n\n",
           MAX_AMOUNT);

    printf("  Enter number of transactions to simulate: ");
    if (scanf("%d", &total_txns) != 1 || total_txns <= 0) {
        fprintf(stderr, "  Invalid input. Please enter a positive integer.\n");
        return EXIT_FAILURE;
    }

    /* Cap log to avoid overflow */
    if (total_txns > LOG_SIZE) {
        printf("  (Transaction log capped at %d entries for display.)\n",
               LOG_SIZE);
    }

    /* ── Setup ── */
    init_accounts();
    transactions_remaining = total_txns;

    pthread_t threads[NUM_THREADS];
    int       thread_ids[NUM_THREADS];

    printf("\n  Starting %d threads...\n", NUM_THREADS);

    struct timespec ts_start, ts_end;
    clock_gettime(CLOCK_MONOTONIC, &ts_start);

    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i + 1;
        if (pthread_create(&threads[i], NULL, worker, &thread_ids[i]) != 0) {
            perror("  pthread_create");
            return EXIT_FAILURE;
        }
    }

    /* ── Wait ── */
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &ts_end);
    double elapsed = (ts_end.tv_sec  - ts_start.tv_sec) +
                     (ts_end.tv_nsec - ts_start.tv_nsec) * 1e-9;

    printf("  All threads completed in %.4f seconds.\n", elapsed);

    /* ── Show last 20 log entries (or all if fewer) ── */
    int show = (log_index < 20) ? log_index : 20;
    if (show > 0) {
        printf("\n");
        print_log(show);
    }

    /* ── Summary ── */
    print_summary(total_txns);

    /* ── Cleanup ── */
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        pthread_mutex_destroy(&accounts[i].lock);
    }
    pthread_mutex_destroy(&counter_lock);
    pthread_mutex_destroy(&log_lock);

    return EXIT_SUCCESS;
}