import java.util.Arrays;
import java.util.Random;

public class BankSimulation {

    // Thread-safe bank model (coarse-grained locking: one lock for the whole bank)
    static class Bank {
        private final long[] accounts;      // store in cents to avoid floating-point issues
        private final Object lock = new Object();

        Bank(int numAccounts, long initialBalanceCents) {
            this.accounts = new long[numAccounts];
            Arrays.fill(this.accounts, initialBalanceCents);
        }

        int size() {
            return accounts.length;
        }

        long getBalance(int i) {
            synchronized (lock) {
                return accounts[i];
            }
        }

        long getTotalBalance() {
            synchronized (lock) {
                long sum = 0;
                for (long b : accounts) sum += b;
                return sum;
            }
        }

        boolean transfer(int from, int to, long amountCents) {
            if (from == to) return false;
            if (amountCents <= 0) return false;

            synchronized (lock) {
                if (accounts[from] < amountCents) {
                    return false; // insufficient funds
                }
                accounts[from] -= amountCents;
                accounts[to]   += amountCents;
                return true;
            }
        }
    }

    static class TransferWorker implements Runnable {
        private final Bank bank;
        private final int iterations;
        private final long maxTransferCents;
        private final Random rnd;

        TransferWorker(Bank bank, int iterations, long maxTransferCents, long seed) {
            this.bank = bank;
            this.iterations = iterations;
            this.maxTransferCents = maxTransferCents;
            this.rnd = new Random(seed);
        }

        @Override
        public void run() {
            int n = bank.size();
            for (int i = 0; i < iterations; i++) {
                int from = rnd.nextInt(n);
                int to = rnd.nextInt(n);

                long amount = 1 + (Math.abs(rnd.nextLong()) % maxTransferCents);
                bank.transfer(from, to, amount);
            }
        }
    }

    private static long dollarsToCents(String dollars) {
        // Minimal parsing: accept values like "100", "100.5", "100.50"
        String s = dollars.trim();
        boolean neg = s.startsWith("-");
        if (neg) s = s.substring(1);

        String[] parts = s.split("\\.", 2);
        long whole = Long.parseLong(parts[0]);
        long frac = 0;

        if (parts.length == 2) {
            String f = parts[1];
            if (f.length() == 1) f = f + "0";
            if (f.length() > 2) f = f.substring(0, 2);
            while (f.length() < 2) f += "0";
            frac = Long.parseLong(f);
        }

        long cents = whole * 100 + frac;
        return neg ? -cents : cents;
    }

    private static String centsToDollars(long cents) {
        long abs = Math.abs(cents);
        long dollars = abs / 100;
        long rem = abs % 100;
        return (cents < 0 ? "-" : "") + dollars + "." + (rem < 10 ? "0" : "") + rem;
    }

    public static void main(String[] args) throws InterruptedException {
        if (args.length < 2) {
            System.out.println("Usage: java BankSimulation <numAccounts> <initialBalanceDollars> [numThreads] [transfersPerThread] [maxTransferDollars]");
            System.out.println("Example: java BankSimulation 10 1000.00 8 200000 50.00");
            return;
        }

        int numAccounts = Integer.parseInt(args[0]);
        long initialBalanceCents = dollarsToCents(args[1]);

        int numThreads = (args.length >= 3) ? Integer.parseInt(args[2]) : 8;
        int transfersPerThread = (args.length >= 4) ? Integer.parseInt(args[3]) : 200_000;
        long maxTransferCents = (args.length >= 5) ? dollarsToCents(args[4]) : dollarsToCents("50.00");

        if (numAccounts <= 0 || numThreads <= 0 || transfersPerThread < 0 || initialBalanceCents < 0 || maxTransferCents <= 0) {
            throw new IllegalArgumentException("Invalid arguments.");
        }

        Bank bank = new Bank(numAccounts, initialBalanceCents);

        long initialTotal = bank.getTotalBalance();
        System.out.println("Initial total balance: $" + centsToDollars(initialTotal));

        Thread[] threads = new Thread[numThreads];
        long baseSeed = System.nanoTime();

        for (int i = 0; i < numThreads; i++) {
            threads[i] = new Thread(
                new TransferWorker(bank, transfersPerThread, maxTransferCents, baseSeed + i),
                "TransferWorker-" + i
            );
        }

        long start = System.currentTimeMillis();
        for (Thread t : threads) t.start();
        for (Thread t : threads) t.join();
        long elapsed = System.currentTimeMillis() - start;

        System.out.println("\nAll threads finished in " + elapsed + " ms.\n");

        // Print final balances
        for (int i = 0; i < numAccounts; i++) {
            System.out.println("Account " + i + ": $" + centsToDollars(bank.getBalance(i)));
        }

        long finalTotal = bank.getTotalBalance();
        System.out.println("\nFinal total balance:   $" + centsToDollars(finalTotal));
        System.out.println("Expected total balance:$" + centsToDollars(initialTotal));

        if (finalTotal == initialTotal) {
            System.out.println("TOTAL CHECK: OK (total remained consistent)");
        } else {
            System.out.println("TOTAL CHECK: FAIL (total changed!)");
        }
    }
}