import java.util.*;
import java.util.concurrent.*;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

public class BankingSystem {
    private static class BankAccount {
        private final int accountId;
        private double balance;
        private final Lock lock;

        public BankAccount(int accountId, double initialBalance) {
            this.accountId = accountId;
            this.balance = initialBalance;
            this.lock = new ReentrantLock();
        }

        public void withdraw(double amount) {
            lock.lock();
            try {
                this.balance -= amount;
            } finally {
                lock.unlock();
            }
        }

        public void deposit(double amount) {
            lock.lock();
            try {
                this.balance += amount;
            } finally {
                lock.unlock();
            }
        }

        public double getBalance() {
            lock.lock();
            try {
                return this.balance;
            } finally {
                lock.unlock();
            }
        }

        public int getAccountId() {
            return accountId;
        }
    }

    private static class TransferTask implements Runnable {
        private final BankAccount[] accounts;
        private final int transferCount;
        private final double maxTransferAmount;
        private final Random random;

        public TransferTask(BankAccount[] accounts, int transferCount, double maxTransferAmount) {
            this.accounts = accounts;
            this.transferCount = transferCount;
            this.maxTransferAmount = maxTransferAmount;
            this.random = new Random();
        }

        @Override
        public void run() {
            for (int i = 0; i < transferCount; i++) {
                // Randomly select source and destination accounts
                int sourceIndex = random.nextInt(accounts.length);
                int destIndex = random.nextInt(accounts.length);

                // Avoid transferring to the same account
                if (sourceIndex == destIndex) {
                    i--;
                    continue;
                }

                // Generate random transfer amount
                double transferAmount = random.nextDouble() * maxTransferAmount;

                // Perform transfer
                transfer(accounts[sourceIndex], accounts[destIndex], transferAmount);
            }
        }

        private void transfer(BankAccount source, BankAccount dest, double amount) {
            // Use account IDs to ensure consistent lock ordering and prevent deadlock
            BankAccount first, second;
            if (source.getAccountId() < dest.getAccountId()) {
                first = source;
                second = dest;
            } else {
                first = dest;
                second = source;
            }

            // Acquire locks in consistent order to prevent deadlock
            first.lock.lock();
            try {
                second.lock.lock();
                try {
                    // Check if source has sufficient balance
                    if (source.balance >= amount) {
                        source.balance -= amount;
                        dest.balance += amount;
                    }
                } finally {
                    second.lock.unlock();
                }
            } finally {
                first.lock.unlock();
            }
        }
    }

    public static void main(String[] args) throws InterruptedException {
        Scanner scanner = new Scanner(System.in);

        System.out.println("=== Banking System Simulator ===\n");

        // Get number of accounts
        System.out.print("Enter the number of bank accounts: ");
        int accountCount = scanner.nextInt();

        if (accountCount <= 0) {
            System.out.println("Number of accounts must be positive.");
            scanner.close();
            return;
        }

        // Get starting balance
        System.out.print("Enter the starting balance for each account: $");
        double startingBalance = scanner.nextDouble();

        if (startingBalance < 0) {
            System.out.println("Starting balance cannot be negative.");
            scanner.close();
            return;
        }

        // Get number of threads
        System.out.print("Enter the number of transfer threads: ");
        int threadCount = scanner.nextInt();

        if (threadCount <= 0) {
            System.out.println("Number of threads must be positive.");
            scanner.close();
            return;
        }

        // Get transfers per thread
        System.out.print("Enter the number of transfers per thread: ");
        int transfersPerThread = scanner.nextInt();

        if (transfersPerThread <= 0) {
            System.out.println("Number of transfers must be positive.");
            scanner.close();
            return;
        }

        scanner.close();

        // Initialize bank accounts
        BankAccount[] accounts = new BankAccount[accountCount];
        for (int i = 0; i < accountCount; i++) {
            accounts[i] = new BankAccount(i, startingBalance);
        }

        double initialTotalBalance = accountCount * startingBalance;
        System.out.println("\n--- Initial State ---");
        System.out.println("Total Accounts: " + accountCount);
        System.out.println("Starting Balance per Account: $" + String.format("%.2f", startingBalance));
        System.out.println("Initial Total Balance: $" + String.format("%.2f", initialTotalBalance));
        System.out.println("Number of Threads: " + threadCount);
        System.out.println("Transfers per Thread: " + transfersPerThread);
        System.out.println("\nStarting transfers...\n");

        long startTime = System.currentTimeMillis();

        // Create and start transfer threads
        ExecutorService executor = Executors.newFixedThreadPool(threadCount);
        double maxTransferAmount = startingBalance / 10; // Max transfer is 10% of starting balance

        for (int i = 0; i < threadCount; i++) {
            executor.submit(new TransferTask(accounts, transfersPerThread, maxTransferAmount));
        }

        // Shutdown executor and wait for all threads to complete
        executor.shutdown();
        executor.awaitTermination(Long.MAX_VALUE, TimeUnit.NANOSECONDS);

        long endTime = System.currentTimeMillis();

        // Display final results
        System.out.println("\n--- Final State ---");
        System.out.println("Total Transfers Completed: " + (threadCount * transfersPerThread));
        System.out.println("Time Elapsed: " + (endTime - startTime) + " ms\n");

        double finalTotalBalance = 0;
        System.out.println("Final Account Balances:");
        System.out.println("-".repeat(50));
        for (BankAccount account : accounts) {
            double balance = account.getBalance();
            finalTotalBalance += balance;
            System.out.println(String.format("Account %d: $%-15.2f", account.getAccountId(), balance));
        }
        System.out.println("-".repeat(50));

        System.out.println("\n--- Balance Verification ---");
        System.out.println("Initial Total Balance: $" + String.format("%.2f", initialTotalBalance));
        System.out.println("Final Total Balance:   $" + String.format("%.2f", finalTotalBalance));
        System.out.println("Difference:            $" + String.format("%.2f", Math.abs(initialTotalBalance - finalTotalBalance)));

        if (Math.abs(initialTotalBalance - finalTotalBalance) < 0.01) {
            System.out.println("\n✓ SUCCESS: Total balance is consistent!");*
