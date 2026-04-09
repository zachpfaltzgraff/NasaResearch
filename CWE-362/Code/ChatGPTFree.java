import java.util.Random;

class Bank {
    private final double[] accounts;

    public Bank(int n, double initialBalance) {
        accounts = new double[n];
        for (int i = 0; i < accounts.length; i++) {
            accounts[i] = initialBalance;
        }
    }

    // Synchronized transfer method
    public synchronized void transfer(int from, int to, double amount) {
        if (accounts[from] < amount) return;

        accounts[from] -= amount;
        accounts[to] += amount;
    }

    public synchronized double getTotalBalance() {
        double sum = 0;
        for (double a : accounts) {
            sum += a;
        }
        return sum;
    }

    public int size() {
        return accounts.length;
    }

    public synchronized void printBalances() {
        for (int i = 0; i < accounts.length; i++) {
            System.out.printf("Account %d: %.2f%n", i, accounts[i]);
        }
    }
}

class TransferTask implements Runnable {
    private final Bank bank;
    private final int iterations;
    private final Random rand = new Random();

    public TransferTask(Bank bank, int iterations) {
        this.bank = bank;
        this.iterations = iterations;
    }

    @Override
    public void run() {
        for (int i = 0; i < iterations; i++) {
            int from = rand.nextInt(bank.size());
            int to = rand.nextInt(bank.size());
            double amount = rand.nextDouble() * 100;

            bank.transfer(from, to, amount);
        }
    }
}

public class BankSimulation {
    public static void main(String[] args) throws InterruptedException {
        int numAccounts = 10;
        double initialBalance = 1000.0;
        int numThreads = 5;
        int transfersPerThread = 10000;

        Bank bank = new Bank(numAccounts, initialBalance);

        System.out.println("Initial total balance: " + bank.getTotalBalance());

        Thread[] threads = new Thread[numThreads];

        // Create and start threads
        for (int i = 0; i < numThreads; i++) {
            threads[i] = new Thread(new TransferTask(bank, transfersPerThread));
            threads[i].start();
        }

        // Wait for all threads to finish
        for (Thread t : threads) {
            t.join();
        }

        System.out.println("\nFinal account balances:");
        bank.printBalances();

        double finalTotal = bank.getTotalBalance();
        System.out.println("\nFinal total balance: " + finalTotal);
        System.out.println("Expected total balance: " + (numAccounts * initialBalance));
    }
}