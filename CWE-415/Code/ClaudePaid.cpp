/*
 * bank_simulation.c
 *
 * Simulates concurrent deposits and withdrawals on 10 shared bank accounts
 * using POSIX threads (pthreads) with mutex-based synchronization.
 *
 * Compile: gcc -o bank_simulation bank_simulation.c -lpthread
 * Run:     ./bank_simulation
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <string.h>

/* ─── Constants ───────────────────────────────────────────────────── */
#define NUM_ACCOUNTS     10
#define NUM_THREADS      5          /* worker threads running in parallel  */
#define INITIAL_BALANCE  1000.00    /* every account starts at $1,000      */
#define MAX_AMOUNT       200.00     /* maximum single transaction amount    */

/* ─── Shared state ────────────────────────────────────────────────── */
typedef struct {
    double          balance;
    pthread_mutex_t lock;
    int             account_id;
    long            total_deposits;
    long            total_withdrawals;
    long            rejected_withdrawals;  /* insufficient funds */
} Account;

static Account accounts[NUM_ACCOUNTS];

/* Controls how many transactions remain across all threads */
static long         transactions_remaining;
static pthread_mutex_t counter_lock = PTHREAD_MUTEX_INITIALIZER;

/* Aggregate stats */
static long total_completed   = 0;
static long total_rejected    = 0;
static pthread_mutex_t stats_lock = PTHREAD_MUTEX_INITIALIZER;

/* ─── Thread argument ─────────────────────────────────────────────── */
typedef struct {
    int thread_id;
} ThreadArg;

/* ─── Helper: thread-safe random double in [0, max] ──────────────── */
static double rand_amount(unsigned int *seed) {
    return ((double)rand_r(seed) / RAND_MAX) * MAX_AMOUNT;
}

/* ─── Worker thread ───────────────────────────────────────────────── */
static void *worker(void *arg) {
    ThreadArg    *targ = (ThreadArg *)arg;
    unsigned int  seed = (unsigned int)(time(NULL) ^ (targ->thread_id * 1000003UL));

    while (1) {
        /* Atomically claim one transaction slot */
        pthread_mutex_lock(&counter_lock);
        if (transactions_remaining <= 0) {
            pthread_mutex_unlock(&counter_lock);
            break;
        }
        transactions_remaining--;
        pthread_mutex_unlock(&counter_lock);

        /* Pick a random account and amount */
        int    acc_idx = rand_r(&seed) % NUM_ACCOUNTS;
        double amount  = rand_amount(&seed);
        int    is_deposit = rand_r(&seed) % 2;   /* 0 = withdraw, 1 = deposit */

        Account *acc = &accounts[acc_idx];
        pthread_mutex_lock(&acc->lock);

        if (is_deposit) {
            acc->balance += amount;
            acc->total_deposits++;
            printf("  [T%d] DEPOSIT   Account #%02d  +$%8.2f  =>  Balance: $%10.2f\n",
                   targ->thread_id, acc->account_id, amount, acc->balance);

            pthread_mutex_lock(&stats_lock);
            total_completed++;
            pthread_mutex_unlock(&stats_lock);
        } else {
            if (amount <= acc->balance) {
                acc->balance -= amount;
                acc->total_withdrawals++;
                printf("  [T%d] WITHDRAW  Account #%02d  -$%8.2f  =>  Balance: $%10.2f\n",
                       targ->thread_id, acc->account_id, amount, acc->balance);

                pthread_mutex_lock(&stats_lock);
                total_completed++;
                pthread_mutex_unlock(&stats_lock);
            } else {
                acc->rejected_withdrawals++;
                printf("  [T%d] REJECTED  Account #%02d  -$%8.2f  (insufficient funds, balance $%.2f)\n",
                       targ->thread_id, acc->account_id, amount, acc->balance);

                pthread_mutex_lock(&stats_lock);
                total_rejected++;
                pthread_mutex_unlock(&stats_lock);
            }
        }

        pthread_mutex_unlock(&acc->lock);
    }

    return NULL;
}

/* ─── Init / teardown helpers ─────────────────────────────────────── */
static void init_accounts(void) {
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        accounts[i].account_id          = i + 1;
        accounts[i].balance             = INITIAL_BALANCE;
        accounts[i].total_deposits      = 0;
        accounts[i].total_withdrawals   = 0;
        accounts[i].rejected_withdrawals = 0;
        pthread_mutex_init(&accounts[i].lock, NULL);
    }
}

static void destroy_accounts(void) {
    for (int i = 0; i < NUM_ACCOUNTS; i++)
        pthread_mutex_destroy(&accounts[i].lock);
    pthread_mutex_destroy(&counter_lock);
    pthread_mutex_destroy(&stats_lock);
}

static void print_summary(void) {
    double total_starting = NUM_ACCOUNTS * INITIAL_BALANCE;
    double total_ending   = 0.0;

    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                   FINAL ACCOUNT SUMMARY                     ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║  %-6s  %-12s  %-10s  %-10s  %-8s  ║\n",
           "Acct", "Balance", "Deposits", "Withdrawals", "Rejected");
    printf("╠══════════════════════════════════════════════════════════════╣\n");

    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        Account *a = &accounts[i];
        total_ending += a->balance;
        printf("║  #%-5d  $%-11.2f  %-10ld  %-10ld  %-8ld  ║\n",
               a->account_id, a->balance,
               a->total_deposits, a->total_withdrawals,
               a->rejected_withdrawals);
    }

    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║  Starting total balance : $%-34.2f  ║\n", total_starting);
    printf("║  Ending   total balance : $%-34.2f  ║\n", total_ending);
    printf("║  Net change             : $%-34.2f  ║\n", total_ending - total_starting);
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║  Completed transactions : %-35ld  ║\n", total_completed);
    printf("║  Rejected  transactions : %-35ld  ║\n", total_rejected);
    printf("╚══════════════════════════════════════════════════════════════╝\n");
}

/* ─── Main ────────────────────────────────────────────────────────── */
int main(void) {
    long num_transactions;

    printf("╔════════════════════════════════════════╗\n");
    printf("║   pthread Bank Account Simulator       ║\n");
    printf("║   Accounts: %d  |  Threads: %d          ║\n", NUM_ACCOUNTS, NUM_THREADS);
    printf("╚════════════════════════════════════════╝\n\n");

    printf("Enter number of transaction simulations to run: ");
    if (scanf("%ld", &num_transactions) != 1 || num_transactions <= 0) {
        fprintf(stderr, "Invalid input. Please enter a positive integer.\n");
        return EXIT_FAILURE;
    }

    init_accounts();
    transactions_remaining = num_transactions;

    /* Print initial balances */
    printf("\nInitial balance for all %d accounts: $%.2f each\n",
           NUM_ACCOUNTS, INITIAL_BALANCE);
    printf("Launching %d worker threads for %ld transactions...\n\n",
           NUM_THREADS, num_transactions);

    /* Spawn threads */
    pthread_t threads[NUM_THREADS];
    ThreadArg args[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        args[i].thread_id = i + 1;
        if (pthread_create(&threads[i], NULL, worker, &args[i]) != 0) {
            perror("pthread_create");
            return EXIT_FAILURE;
        }
    }

    /* Wait for all threads to finish */
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);

    print_summary();
    destroy_accounts();

    return EXIT_SUCCESS;
}