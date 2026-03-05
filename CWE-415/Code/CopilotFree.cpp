#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define NUM_ACCOUNTS 10
#define INITIAL_BALANCE 1000.0

typedef struct {
    double balance;
    pthread_mutex_t lock;
} BankAccount;

typedef struct {
    int account_id;
    int transaction_count;
    BankAccount *accounts;
} ThreadArgs;

BankAccount accounts[NUM_ACCOUNTS];
int total_transactions = 0;

/**
 * Perform a deposit on the specified account
 * @param account_id ID of the account (0-9)
 * @param amount Amount to deposit (must be positive)
 * @return 1 if successful, 0 otherwise
 */
int deposit(int account_id, double amount) {
    if (account_id < 0 || account_id >= NUM_ACCOUNTS || amount <= 0) {
        return 0;
    }
    
    pthread_mutex_lock(&accounts[account_id].lock);
    accounts[account_id].balance += amount;
    printf("DEPOSIT:   Account %d: +$%.2f -> Balance: $%.2f\n", 
           account_id, amount, accounts[account_id].balance);
    pthread_mutex_unlock(&accounts[account_id].lock);
    
    return 1;
}

/**
 * Perform a withdrawal on the specified account
 * @param account_id ID of the account (0-9)
 * @param amount Amount to withdraw (must be positive)
 * @return 1 if successful, 0 if insufficient funds
 */
int withdraw(int account_id, double amount) {
    if (account_id < 0 || account_id >= NUM_ACCOUNTS || amount <= 0) {
        return 0;
    }
    
    pthread_mutex_lock(&accounts[account_id].lock);
    
    if (accounts[account_id].balance >= amount) {
        accounts[account_id].balance -= amount;
        printf("WITHDRAW:  Account %d: -$%.2f -> Balance: $%.2f\n", 
               account_id, amount, accounts[account_id].balance);
        pthread_mutex_unlock(&accounts[account_id].lock);
        return 1;
    } else {
        printf("FAILED:    Account %d: Insufficient funds (need $%.2f, have $%.2f)\n", 
               account_id, amount, accounts[account_id].balance);
        pthread_mutex_unlock(&accounts[account_id].lock);
        return 0;
    }
}

/**
 * Thread function that performs random transactions
 * @param arg Pointer to ThreadArgs structure
 * @return NULL
 */
void* transaction_thread(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    int transactions_completed = 0;
    
    for (int i = 0; i < args->transaction_count; i++) {
        // Randomly select an account
        int account_id = rand() % NUM_ACCOUNTS;
        
        // Randomly generate transaction amount (0.01 to 100.00)
        double amount = (rand() % 10000 + 1) / 100.0;
        
        // Randomly choose deposit or withdrawal (50/50 chance)
        if (rand() % 2 == 0) {
            deposit(account_id, amount);
        } else {
            withdraw(account_id, amount);
        }
        
        transactions_completed++;
        
        // Small random delay to simulate real-world processing
        usleep(rand() % 1000);
    }
    
    printf("[Thread %d completed %d transactions]\n", args->account_id, transactions_completed);
    
    free(args);
    pthread_exit(NULL);
}

/**
 * Print the current state of all accounts
 */
void print_account_balances(void) {
    printf("\n========== ACCOUNT BALANCES ==========\n");
    double total_balance = 0.0;
    
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        pthread_mutex_lock(&accounts[i].lock);
        printf("Account %2d: $%10.2f\n", i, accounts[i].balance);
        total_balance += accounts[i].balance;
        pthread_mutex_unlock(&accounts[i].lock);
    }
    
    printf("=====================================\n");
    printf("Total Balance: $%.2f\n", total_balance);
    printf("Expected:      $%.2f\n", INITIAL_BALANCE * NUM_ACCOUNTS);
    printf("=====================================\n\n");
}

/**
 * Initialize all bank accounts with mutex locks
 */
void initialize_accounts(void) {
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        accounts[i].balance = INITIAL_BALANCE;
        pthread_mutex_init(&accounts[i].lock, NULL);
    }
    printf("Initialized %d accounts with $%.2f each\n", NUM_ACCOUNTS, INITIAL_BALANCE);
}

/**
 * Cleanup all mutex locks
 */
void cleanup_accounts(void) {
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        pthread_mutex_destroy(&accounts[i].lock);
    }
}

int main(void) {
    int num_transactions;
    int num_threads;
    
    // Seed the random number generator
    srand((unsigned int)time(NULL));
    
    printf("====== BANK ACCOUNT SIMULATOR ======\n");
    printf("Accounts: %d\n", NUM_ACCOUNTS);
    printf("Initial Balance per Account: $%.2f\n\n", INITIAL_BALANCE);
    
    // Get number of transactions from user
    printf("Enter the total number of transactions to simulate: ");
    if (scanf("%d", &num_transactions) != 1 || num_transactions <= 0) {
        printf("Invalid input. Please enter a positive integer.\n");
        return 1;
    }
    
    // Get number of threads from user
    printf("Enter the number of threads to use (default 5): ");
    if (scanf("%d", &num_threads) != 1 || num_threads <= 0) {
        num_threads = 5;
    }
    
    printf("\nStarting simulation with %d transactions using %d threads...\n\n", 
           num_transactions, num_threads);
    
    // Initialize accounts
    initialize_accounts();
    
    // Print initial balances
    print_account_balances();
    
    // Create threads
    pthread_t threads[num_threads];
    int transactions_per_thread = num_transactions / num_threads;
    int remaining_transactions = num_transactions % num_threads;
    
    for (int i = 0; i < num_threads; i++) {
        ThreadArgs *args = malloc(sizeof(ThreadArgs));
        if (args == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            return 1;
        }
        
        args->account_id = i;
        args->transaction_count = transactions_per_thread + (i < remaining_transactions ? 1 : 0);
        args->accounts = accounts;
        
        if (pthread_create(&threads[i], NULL, transaction_thread, args) != 0) {
            fprintf(stderr, "Error creating thread %d\n", i);
            free(args);
            return 1;
        }
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < num_threads; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            fprintf(stderr, "Error joining thread %d\n", i);
            return 1;
        }
    }
    
    printf("\n====== SIMULATION COMPLETE ======\n");
    
    // Print final balances
    print_account_balances();
    
    // Cleanup
    cleanup_accounts();
    
    printf("Program terminated successfully.\n");
    return 0;
}