import java.util.*;
import java.util.concurrent.*;
import java.util.concurrent.atomic.*;
import java.util.concurrent.locks.*;

/**
 * Multi-threaded Banking System Simulation
 *
 * Demonstrates thread-safe concurrent transfers using per-account ReentrantLocks.
 * To prevent deadlocks, locks are always acquired in a consistent order (by account ID).
 */
public class BankingSimulation {

    // -------------------------------------------------------------------------
    // BankAccount
    // -------------------------------------------------------------------------
    static class BankAccount {
        private final int id;
        private long balance;           // stored in cents to avoid floating-point issues
        private final ReentrantLock lock = new ReentrantLock();

        BankAccount(int id, long initialBalance) {
            this.id = id;
            this.balance = initialBalance;
        }

        int getId()      { return id; }
        ReentrantLock getLock() { return lock; }

        long getBalance() {
            lock.lock();
            try { return balance; }
            finally { lock.unlock(); }
        }

        /** Raw, lock-free access – only called while BOTH account locks are held. */
        long getBalanceUnsafe()               { return balance; }
        void depositUnsafe(long amount)       { balance += amount; }
        void withdrawUnsafe(long amount)      { balance -= amount; }
    }

    // -------------------------------------------------------------------------
    // TransferWorker
    // -------------------------------------------------------------------------
    static class TransferWorker implements Runnable {
        private final BankAccount[]  accounts;
        private final int            transfersPerThread;
        private final Random         rng = new Random();
        private final AtomicLong     totalTransfersCompleted;
        private final AtomicLong     totalAmountMoved;

        // Per-thread stats (written only by this thread – no sync needed)
        private long localTransfers = 0;
        private long localAmountMoved = 0;

        TransferWorker(BankAccount[] accounts,
                       int transfersPerThread,
                       AtomicLong totalTransfersCompleted,
                       AtomicLong totalAmountMoved) {
            this.accounts               = accounts;
            this.transfersPerThread     = transfersPerThread;
            this.totalTransfersCompleted = totalTransfersCompleted;
            this.totalAmountMoved       = totalAmountMoved;
        }

        @Override
        public void run() {
            int n = accounts.length;

            for (int i = 0; i < transfersPerThread; i++) {

                // Pick two distinct accounts
                int srcIdx = rng.nextInt(n);
                int dstIdx;
                do { dstIdx = rng.nextInt(n); } while (dstIdx == srcIdx);

                BankAccount src = accounts[srcIdx];
                BankAccount dst = accounts[dstIdx];

                // Always lock the lower-ID account first → deadlock-free
                BankAccount first  = src.getId() < dst.getId() ? src : dst;
                BankAccount second = src.getId() < dst.getId() ? dst : src;

                first.getLock().lock();
                try {
                    second.getLock().lock();
                    try {
                        long available = src.getBalanceUnsafe();
                        if (available > 0) {
                            // Transfer between 1 cent and the full balance
                            long amount = (long)(rng.nextDouble() * available) + 1;
                            src.withdrawUnsafe(amount);
                            dst.depositUnsafe(amount);
                            localTransfers++;
                            localAmountMoved += amount;
                        }
                    } finally {
                        second.getLock().unlock();
                    }
                } finally {
                    first.getLock().unlock();
                }
            }

            // Publish local stats atomically
            totalTransfersCompleted.addAndGet(localTransfers);
            totalAmountMoved.addAndGet(localAmountMoved);
        }
    }

    // -------------------------------------------------------------------------
    // Main
    // -------------------------------------------------------------------------
    public static void main(String[] args) throws InterruptedException {

        Scanner scanner = new Scanner(System.in);

        System.out.println("╔══════════════════════════════════════════╗");
        System.out.println("║   Multi-Threaded Banking System Sim      ║");
        System.out.println("╚══════════════════════════════════════════╝");
        System.out.println();

        // --- Configuration ---
        int numAccounts = promptInt(scanner,
                "Enter number of bank accounts (e.g. 10): ", 2, 10_000);

        long startingBalanceDollars = promptLong(scanner,
                "Enter starting balance per account in $ (e.g. 1000): ", 1, 1_000_000_000L);

        int numThreads = promptInt(scanner,
                "Enter number of concurrent threads (e.g. 8): ", 1, 1_000);

        int transfersPerThread = promptInt(scanner,
                "Enter transfers per thread (e.g. 100000): ", 1, 10_000_000);

        // Convert dollars → cents
        long startingBalanceCents = startingBalanceDollars * 100L;

        // --- Create accounts ---
        BankAccount[] accounts = new BankAccount[numAccounts];
        for (int i = 0; i < numAccounts; i++) {
            accounts[i] = new BankAccount(i, startingBalanceCents);
        }

        long initialTotal = (long) numAccounts * startingBalanceCents;

        System.out.println();
        System.out.printf("▶ Accounts       : %,d%n", numAccounts);
        System.out.printf("▶ Balance each   : $%,.2f%n", startingBalanceDollars * 1.0);
        System.out.printf("▶ Initial total  : $%,.2f%n", initialTotal / 100.0);
        System.out.printf("▶ Threads        : %,d%n", numThreads);
        System.out.printf("▶ Transfers/thrd : %,d%n", transfersPerThread);
        System.out.printf("▶ Max transfers  : %,d%n", (long) numThreads * transfersPerThread);
        System.out.println();
        System.out.println("Running simulation…");

        // --- Launch threads ---
        AtomicLong totalTransfers = new AtomicLong(0);
        AtomicLong totalMoved     = new AtomicLong(0);

        Thread[] threads = new Thread[numThreads];
        for (int t = 0; t < numThreads; t++) {
            threads[t] = new Thread(new TransferWorker(
                    accounts, transfersPerThread, totalTransfers, totalMoved));
        }

        long startTime = System.currentTimeMillis();
        for (Thread t : threads) t.start();
        for (Thread t : threads) t.join();
        long elapsed = System.currentTimeMillis() - startTime;

        // --- Compute final total (all locks released; simple sequential read) ---
        long finalTotal = 0;
        for (BankAccount a : accounts) finalTotal += a.getBalance();

        // --- Report ---
        System.out.println();
        System.out.println("═══════════════════════════════════════════════");
        System.out.println("                 SIMULATION RESULTS            ");
        System.out.println("═══════════════════════════════════════════════");
        System.out.println();

        // Print individual balances (cap to 50 lines for readability)
        int printLimit = Math.min(numAccounts, 50);
        System.out.printf("%-12s %20s%n", "Account", "Final Balance");
        System.out.println("─────────────────────────────────");
        for (int i = 0; i < printLimit; i++) {
            System.out.printf("Account %4d  %,20.2f%n",
                    accounts[i].getId(), accounts[i].getBalance() / 100.0);
        }
        if (numAccounts > printLimit) {
            System.out.printf("  … (%,d more accounts not shown)%n", numAccounts - printLimit);
        }

        System.out.println();
        System.out.println("═══════════════════════════════════════════════");
        System.out.printf("  Initial total balance : $%,25.2f%n", initialTotal / 100.0);
        System.out.printf("  Final   total balance : $%,25.2f%n", finalTotal   / 100.0);
        System.out.printf("  Difference            : $%,25.2f%n", (finalTotal - initialTotal) / 100.0);
        System.out.println("═══════════════════════════════════════════════");

        boolean consistent = (finalTotal == initialTotal);
        System.out.println();
        System.out.println("  Balance integrity : " + (consistent ? "✔ PASSED" : "✘ FAILED"));
        System.out.println();
        System.out.printf("  Completed transfers   : %,d%n",      totalTransfers.get());
        System.out.printf("  Total amount moved    : $%,.2f%n",   totalMoved.get() / 100.0);
        System.out.printf("  Elapsed time          : %,d ms%n",   elapsed);
        System.out.printf("  Throughput            : %,.0f tx/s%n",
                totalTransfers.get() / Math.max(elapsed / 1000.0, 0.001));
        System.out.println();
    }

    // -------------------------------------------------------------------------
    // Helpers
    // -------------------------------------------------------------------------
    private static int promptInt(Scanner s, String msg, int min, int max) {
        while (true) {
            System.out.print(msg);
            try {
                int v = Integer.parseInt(s.nextLine().trim());
                if (v >= min && v <= max) return v;
                System.out.printf("  Please enter a value between %,d and %,d.%n", min, max);
            } catch (NumberFormatException e) {
                System.out.println("  Invalid input – please enter an integer.");
            }
        }
    }

    private static long promptLong(Scanner s, String msg, long min, long max) {
        while (true) {
            System.out.print(msg);
            try {
                long v = Long.parseLong(s.nextLine().trim());
                if (v >= min && v <= max) return v;
                System.out.printf("  Please enter a value between %,d and %,d.%n", min, max);
            } catch (NumberFormatException e) {
                System.out.println("  Invalid input – please enter an integer.");
            }
        }
    }
}