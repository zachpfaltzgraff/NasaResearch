import java.util.Random;
import java.util.Scanner;

/**
 * Multi-threaded banking simulation.
 *
 * Features:
 * - User chooses number of accounts
 * - User chooses one shared starting balance for all accounts
 * - Multiple threads perform random transfers simultaneously
 * - Final balances and total balance are printed
 * - Total should remain equal to the initial total
 */
public class BankingSimulation {

    static class Bank {
        private final double[] accounts;

        public Bank(int numberOfAccounts, double initialBalance) {
            accounts = new double[numberOfAccounts];
            for (int i = 0; i < numberOfAccounts; i++) {
                accounts[i] = initialBalance;
            }
        }

        public int size() {
            return accounts.length;
        }

        public synchronized void transfer(int from, int to, double amount) {
            if (from == to) {
                return;
            }

            if (amount <= 0) {
                return;
            }

            if (accounts[from] >= amount) {
                accounts[from] -= amount;
                accounts[to] += amount;
            }
        }

        public synchronized double getBalance(int account) {
            return accounts[account];
        }

        public synchronized double getTotalBalance() {
            double total = 0.0;
            for (double balance : accounts) {
                total += balance;
            }
            return total;
        }

        public synchronized void printAllBalances() {
            System.out.println("\nFinal account balances:");
            for (int i = 0; i < accounts.length; i++) {
                System.out.printf("Account %d: %.2f%n", i, accounts[i]);
            }
        }
    }

    static class TransferTask implements Runnable {
        private final Bank bank;
        private final int transfersToPerform;
        private final double maxTransferAmount;
        private final Random random = new Random();

        public TransferTask(Bank bank, int transfersToPerform, double maxTransferAmount) {
            this.bank = bank;
            this.transfersToPerform = transfersToPerform;
            this.maxTransferAmount = maxTransferAmount;
        }

        @Override
        public void run() {
            for (int i = 0; i < transfersToPerform; i++) {
                int from = random.nextInt(bank.size());
                int to = random.nextInt(bank.size());
                double amount = random.nextDouble() * maxTransferAmount;

                bank.transfer(from, to, amount);
            }
        }
    }

    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);

        try {
            System.out.print("Enter number of bank accounts: ");
            int numberOfAccounts = scanner.nextInt();

            if (numberOfAccounts <= 0) {
                System.out.println("Number of accounts must be greater than 0.");
                return;
            }

            System.out.print("Enter starting balance for each account: ");
            double initialBalance = scanner.nextDouble();

            if (initialBalance < 0) {
                System.out.println("Starting balance cannot be negative.");
                return;
            }

            System.out.print("Enter number of threads: ");
            int numberOfThreads = scanner.nextInt();

            if (numberOfThreads <= 0) {
                System.out.println("Number of threads must be greater than 0.");
                return;
            }

            System.out.print("Enter number of transfers per thread: ");
            int transfersPerThread = scanner.nextInt();

            if (transfersPerThread < 0) {
                System.out.println("Transfers per thread cannot be negative.");
                return;
            }

            Bank bank = new Bank(numberOfAccounts, initialBalance);
            double expectedTotal = numberOfAccounts * initialBalance;

            Thread[] threads = new Thread[numberOfThreads];

            for (int i = 0; i < numberOfThreads; i++) {
                Runnable task = new TransferTask(bank, transfersPerThread, initialBalance);
                threads[i] = new Thread(task, "TransferThread-" + i);
                threads[i].start();
            }

            for (Thread thread : threads) {
                thread.join();
            }

            bank.printAllBalances();

            double finalTotal = bank.getTotalBalance();

            System.out.printf("%nExpected total balance: %.2f%n", expectedTotal);
            System.out.printf("Final total balance:    %.2f%n", finalTotal);

            if (Math.abs(expectedTotal - finalTotal) < 0.0001) {
                System.out.println("Total balance remained consistent.");
            } else {
                System.out.println("Warning: total balance changed.");
            }

        } catch (Exception e) {
            System.out.println("An error occurred: " + e.getMessage());
        } finally {
            scanner.close();
        }
    }
}