#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define NUM_ACCOUNTS 10
#define INITIAL_BALANCE 1000
#define MAX_TRANSACTION 500

// Shared bank accounts
int accounts[NUM_ACCOUNTS];

// Mutex lock for each account
pthread_mutex_t account_locks[NUM_ACCOUNTS];

// Thread function
void* transaction(void* arg) {
    int account = rand() % NUM_ACCOUNTS;
    int amount = rand() % MAX_TRANSACTION + 1;
    int action = rand() % 2; // 0 = deposit, 1 = withdraw

    pthread_mutex_lock(&account_locks[account]);

    if (action == 0) {
        // Deposit
        accounts[account] += amount;
        printf("Thread %lu deposited $%d into Account %d | New Balance: $%d\n",
               pthread_self(), amount, account, accounts[account]);
    } else {
        // Withdraw (only if sufficient funds)
        if (accounts[account] >= amount) {
            accounts[account] -= amount;
            printf("Thread %lu withdrew $%d from Account %d | New Balance: $%d\n",
                   pthread_self(), amount, account, accounts[account]);
        } else {
            printf("Thread %lu failed withdrawal of $%d from Account %d | Insufficient Funds ($%d)\n",
                   pthread_self(), amount, account, accounts[account]);
        }
    }

    pthread_mutex_unlock(&account_locks[account]);
    pthread_exit(NULL);
}

int main() {
    int numTransactions;

    srand(time(NULL));

    printf("Enter number of transaction simulations (threads): ");
    scanf("%d", &numTransactions);

    pthread_t threads[numTransactions];

    // Initialize accounts and mutexes
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        accounts[i] = INITIAL_BALANCE;
        pthread_mutex_init(&account_locks[i], NULL);
    }

    // Create threads
    for (int i = 0; i < numTransactions; i++) {
        if (pthread_create(&threads[i], NULL, transaction, NULL) != 0) {
            perror("Failed to create thread");
            return 1;
        }
    }

    // Join threads
    for (int i = 0; i < numTransactions; i++) {
        pthread_join(threads[i], NULL);
    }

    // Final account balances
    printf("\nFinal Account Balances:\n");
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        printf("Account %d: $%d\n", i, accounts[i]);
        pthread_mutex_destroy(&account_locks[i]);
    }

    return 0;
}