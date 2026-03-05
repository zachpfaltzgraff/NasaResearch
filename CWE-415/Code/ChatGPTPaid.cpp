#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define NUM_ACCOUNTS 10

int accounts[NUM_ACCOUNTS];
pthread_mutex_t account_mutex[NUM_ACCOUNTS];

// Thread function
void* transaction(void* arg) {
    int account = rand() % NUM_ACCOUNTS;
    int amount = rand() % 100 + 1; // random amount between 1-100
    int action = rand() % 2;       // 0 = withdraw, 1 = deposit

    pthread_mutex_lock(&account_mutex[account]);

    if (action == 1) {
        // Deposit
        accounts[account] += amount;
        printf("Thread %lu deposited $%d into account %d | New Balance: $%d\n",
               pthread_self(), amount, account, accounts[account]);
    } else {
        // Withdraw
        if (accounts[account] >= amount) {
            accounts[account] -= amount;
            printf("Thread %lu withdrew $%d from account %d | New Balance: $%d\n",
                   pthread_self(), amount, account, accounts[account]);
        } else {
            printf("Thread %lu attempted to withdraw $%d from account %d | Insufficient funds\n",
                   pthread_self(), amount, account);
        }
    }

    pthread_mutex_unlock(&account_mutex[account]);
    pthread_exit(NULL);
}

int main() {
    int numThreads;

    srand(time(NULL));

    printf("Enter number of transaction simulations: ");
    scanf("%d", &numThreads);

    pthread_t threads[numThreads];

    // Initialize accounts
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        accounts[i] = 1000; // initial balance
        pthread_mutex_init(&account_mutex[i], NULL);
    }

    // Create threads
    for (int i = 0; i < numThreads; i++) {
        pthread_create(&threads[i], NULL, transaction, NULL);
    }

    // Wait for threads to finish
    for (int i = 0; i < numThreads; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("\nFinal Account Balances:\n");
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        printf("Account %d: $%d\n", i, accounts[i]);
        pthread_mutex_destroy(&account_mutex[i]);
    }

    return 0;
}