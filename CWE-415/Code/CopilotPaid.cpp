/*
  bank_pthreads.c
  --------------
  Simulates deposits and withdrawals on 10 shared bank accounts using pthreads.

  - Each account starts with an initial balance.
  - Multiple worker threads randomly choose an account and deposit/withdraw.
  - Per-account mutexes protect balances (fine-grained locking).
  - User enters how many transaction simulations to run total.

  Build:
    gcc -O2 -Wall -Wextra -pthread bank_pthreads.c -o bank_pthreads

  Run:
    ./bank_pthreads
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

#define NUM_ACCOUNTS 10
#define DEFAULT_THREADS 4

typedef struct {
    long balance;
    pthread_mutex_t lock;
} Account;

typedef struct {
    Account *accounts;
    int num_accounts;

    long total_transactions;      // total transactions to run across all threads
    long next_txn_index;          // shared counter of how many have been claimed so far
    pthread_mutex_t txn_lock;     // protects next_txn_index

    unsigned int seed;            // base seed for generating per-thread seeds
} SharedState;

static long rand_range(unsigned int *seed, long lo, long hi) {
    // inclusive range [lo, hi]
    if (hi <= lo) return lo;
    long span = hi - lo + 1;
    return lo + (long)(rand_r(seed) % (unsigned long)span);
}

static void *worker_thread(void *arg) {
    SharedState *st = (SharedState *)arg;

    // Create a per-thread RNG seed (rand_r is thread-safe with its own seed)
    unsigned int seed = st->seed ^ (unsigned int)(uintptr_t)pthread_self() ^ (unsigned int)time(NULL);

    for (;;) {
        // Claim the next transaction index
        pthread_mutex_lock(&st->txn_lock);
        long idx = st->next_txn_index;
        if (idx >= st->total_transactions) {
            pthread_mutex_unlock(&st->txn_lock);
            break;
        }
        st->next_txn_index++;
        pthread_mutex_unlock(&st->txn_lock);

        int acct = (int)rand_range(&seed, 0, st->num_accounts - 1);

        // Choose deposit vs withdrawal
        int do_deposit = (rand_r(&seed) & 1);

        // Choose an amount (tweak as desired)
        long amount = rand_range(&seed, 1, 200);

        if (do_deposit) {
            pthread_mutex_lock(&st->accounts[acct].lock);
            st->accounts[acct].balance += amount;
            pthread_mutex_unlock(&st->accounts[acct].lock);
        } else {
            pthread_mutex_lock(&st->accounts[acct].lock);
            if (st->accounts[acct].balance >= amount) {
                st->accounts[acct].balance -= amount;
            }
            // else: insufficient funds -> transaction rejected (no change)
            pthread_mutex_unlock(&st->accounts[acct].lock);
        }

        // Optional: slow down to make interleavings easier to observe (comment out if undesired)
        // usleep(1000);
    }

    return NULL;
}

static void usage(const char *prog) {
    fprintf(stderr, "Usage: %s [num_threads]\n", prog);
    fprintf(stderr, "  num_threads default: %d\n", DEFAULT_THREADS);
}

int main(int argc, char **argv) {
    int num_threads = DEFAULT_THREADS;

    if (argc > 2) {
        usage(argv[0]);
        return 2;
    }
    if (argc == 2) {
        num_threads = atoi(argv[1]);
        if (num_threads <= 0 || num_threads > 256) {
            fprintf(stderr, "Invalid num_threads: %d (must be 1..256)\n", num_threads);
            return 2;
        }
    }

    long total_transactions = 0;
    printf("Enter how many transaction simulations to run: ");
    fflush(stdout);
    if (scanf("%ld", &total_transactions) != 1 || total_transactions < 0) {
        fprintf(stderr, "Invalid input.\n");
        return 1;
    }

    Account accounts[NUM_ACCOUNTS];
    const long initial_balance = 1000;

    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        accounts[i].balance = initial_balance;
        if (pthread_mutex_init(&accounts[i].lock, NULL) != 0) {
            fprintf(stderr, "pthread_mutex_init failed for account %d\n", i);
            return 1;
        }
    }

    SharedState st;
    st.accounts = accounts;
    st.num_accounts = NUM_ACCOUNTS;
    st.total_transactions = total_transactions;
    st.next_txn_index = 0;
    st.seed = (unsigned int)time(NULL) ^ (unsigned int)getpid();
    if (pthread_mutex_init(&st.txn_lock, NULL) != 0) {
        fprintf(stderr, "pthread_mutex_init failed for txn_lock\n");
        return 1;
    }

    pthread_t *threads = (pthread_t *)calloc((size_t)num_threads, sizeof(pthread_t));
    if (!threads) {
        perror("calloc");
        return 1;
    }

    long initial_total = initial_balance * NUM_ACCOUNTS;
    printf("\nStarting simulation:\n");
    printf("  Accounts: %d\n  Initial balance each: %ld\n  Total initial: %ld\n  Threads: %d\n  Transactions: %ld\n\n",
           NUM_ACCOUNTS, initial_balance, initial_total, num_threads, total_transactions);

    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(&threads[i], NULL, worker_thread, &st) != 0) {
            fprintf(stderr, "pthread_create failed (thread %d)\n", i);
            return 1;
        }
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    long final_total = 0;
    printf("Final account balances:\n");
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        // Lock to read consistently (not strictly needed after joins, but fine practice)
        pthread_mutex_lock(&accounts[i].lock);
        long b = accounts[i].balance;
        pthread_mutex_unlock(&accounts[i].lock);

        printf("  Account %2d: %ld\n", i, b);
        final_total += b;
    }

    printf("\nTotals:\n");
    printf("  Total initial: %ld\n", initial_total);
    printf("  Total final:   %ld\n", final_total);
    printf("  Note: total can change because deposits add money; withdrawals remove money.\n");
    printf("        (Withdrawals are rejected if insufficient funds.)\n");

    for (int i = 0; i < NUM_ACCOUNTS; i++) pthread_mutex_destroy(&accounts[i].lock);
    pthread_mutex_destroy(&st.txn_lock);
    free(threads);

    return 0;
}