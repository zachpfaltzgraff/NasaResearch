import java.util.*;
import java.util.concurrent.*;
import java.util.concurrent.atomic.*;

/**
 * Multi-threaded Banking System Simulation
 *
 * Design highlights:
 *  - Each BankAccount uses a ReentrantLock for fine-grained locking.
 *  - Deadlock is prevented by always acquiring locks in ascending account-ID order.
 *  - AtomicLong counters track successful and failed (insufficient-funds) transfers
 *    without any extra synchronisation cost.
 *  - A thread-safe audit log (ConcurrentLinkedQueue) records every transfer for
 *    optional review after the run.
 */
public class BankingSimulation {

    // -------------------------------------------------------------------------
    // Configuration – feel free to tweak these
    // -------------------------------------------------------------------------
    private static final int    NUM_THREADS           = 8;
    private static final int    TRANSFERS_PER_THREAD  = 10_000;
    private static final int    MAX_TRANSFER_AMOUNT   = 500;   // max units per transfer
    private static final int    AUDIT_SAMPLE_SIZE     = 10;    // entries printed at the end

    // -------------------------------------------------------------------------
    // BankAccount
    // -------------------------------------------------------------------------
    static class BankAccount {

        private final int    id;
        private       long   balance;
        // Use a fair lock so no thread starves when contention is high
        private final java.util.concurrent.locks.ReentrantLock lock =
                new java.util.concurrent.locks.ReentrantLock(true);

        BankAccount(int id, long initialBalance) {
            this.id      = id;
            this.balance = initialBalance;
        }

        int  getId()      { return id; }
        long getBalance() { return balance; }   // call only while lock is held

        void lock()   { lock.lock(); }
        void unlock() { lock.unlock(); }

        /**
         * Transfer {@code amount} to {@code target}.
         * Both accounts MUST already be locked by the caller.
         *
         * @return true if the transfer succeeded, false if funds were insufficient.
         */
        boolean transferTo(BankAccount target, long amount) {
            if (this.balance < amount) return false;
            this.balance   -= amount;
            target.balance += amount;
            return true;
        }

        @Override
        public String toString() {
            return String.format("Account[%3d] balance = %,d", id, balance);
        }
    }

    // -------------------------------------------------------------------------
    // TransferTask  (Callable so we can collect per-thread statistics)
    // -------------------------------------------------------------------------
    static class TransferTask implements Callable<long[]> {

        private final BankAccount[]              accounts;
        private final int                        numTransfers;
        private final int                        maxAmount;
        private final AtomicLong                 successCounter;
        private final AtomicLong                 failCounter;
        private final Queue<String>              auditLog;
        private final Random                     rng = new Random();

        TransferTask(BankAccount[] accounts, int numTransfers, int maxAmount,
                     AtomicLong successCounter, AtomicLong failCounter,
                     Queue<String> auditLog) {
            this.accounts       = accounts;
            this.numTransfers   = numTransfers;
            this.maxAmount      = maxAmount;
            this.successCounter = successCounter;
            this.failCounter    = failCounter;
            this.auditLog       = auditLog;
        }

        @Override
        public long[] call() {
            long localSuccess = 0, localFail = 0;

            for (int i = 0; i < numTransfers; i++) {

                // Pick two distinct accounts
                int srcIdx = rng.nextInt(accounts.length);
                int dstIdx;
                do { dstIdx = rng.nextInt(accounts.length); } while (dstIdx == srcIdx);

                long amount = 1 + rng.nextInt(maxAmount);   // [1 .. maxAmount]

                // --- Deadlock-prevention: lock in ascending ID order ----------
                BankAccount first, second;
                if (accounts[srcIdx].getId() < accounts[dstIdx].getId()) {
                    first  = accounts[srcIdx];
                    second = accounts[dstIdx];
                } else {
                    first  = accounts[dstIdx];
                    second = accounts[srcIdx];
                }

                first.lock();
                try {
                    second.lock();
                    try {
                        boolean ok = accounts[srcIdx].transferTo(accounts[dstIdx], amount);
                        if (ok) {
                            localSuccess++;
                            successCounter.incrementAndGet();
                            // Sample 1 in 500 transfers to keep the log manageable
                            if (rng.nextInt(500) == 0) {
                                auditLog.add(String.format(
                                    "Thread %-24s | A%d -> A%d | $%,d",
                                    Thread.currentThread().getName(),
                                    accounts[srcIdx].getId(),
                                    accounts[dstIdx].getId(),
                                    amount));
                            }
                        } else {
                            localFail++;
                            failCounter.incrementAndGet();
                        }
                    } finally { second.unlock(); }
                } finally { first.unlock(); }
            }

            return new long[]{ localSuccess, localFail };
        }
    }

    // -------------------------------------------------------------------------
    // main
    // -------------------------------------------------------------------------
    public static void main(String[] args) throws Exception {

        Scanner scanner = new Scanner(System.in);

        // --- User input -------------------------------------------------------
        System.out.println("╔══════════════════════════════════════════╗");
        System.out.println("║   Multi-Threaded Banking Simulation      ║");
        System.out.println("╚══════════════════════════════════════════╝");
        System.out.println();

        System.out.print("Enter number of bank accounts : ");
        int numAccounts = Integer.parseInt(scanner.nextLine().trim());
        if (numAccounts < 2) {
            System.out.println("Need at least 2 accounts. Defaulting to 2.");
            numAccounts = 2;
        }

        System.out.print("Enter starting balance for each account: ");
        long startingBalance = Long.parseLong(scanner.nextLine().trim());
        if (startingBalance < 1) {
            System.out.println("Balance must be >= 1. Defaulting to 1000.");
            startingBalance = 1_000;
        }

        // --- Setup ------------------------------------------------------------
        BankAccount[] accounts = new BankAccount[numAccounts];
        for (int i = 0; i < numAccounts; i++) {
            accounts[i] = new BankAccount(i, startingBalance);
        }

        long initialTotal = (long) numAccounts * startingBalance;

        AtomicLong    globalSuccess = new AtomicLong();
        AtomicLong    globalFail    = new AtomicLong();
        Queue<String> auditLog      = new ConcurrentLinkedQueue<>();

        int totalTransfers = NUM_THREADS * TRANSFERS_PER_THREAD;

        System.out.println();
        System.out.printf("  Accounts          : %,d%n",   numAccounts);
        System.out.printf("  Starting balance  : $%,d each%n", startingBalance);
        System.out.printf("  Initial total     : $%,d%n",   initialTotal);
        System.out.printf("  Worker threads    : %d%n",    NUM_THREADS);
        System.out.printf("  Total transfers   : %,d%n",   totalTransfers);
        System.out.println();

        // --- Run --------------------------------------------------------------
        ExecutorService executor   = Executors.newFixedThreadPool(NUM_THREADS);
        List<Future<long[]>> futures = new ArrayList<>();

        long startTime = System.currentTimeMillis();

        for (int t = 0; t < NUM_THREADS; t++) {
            futures.add(executor.submit(new TransferTask(
                    accounts, TRANSFERS_PER_THREAD, MAX_TRANSFER_AMOUNT,
                    globalSuccess, globalFail, auditLog)));
        }

        // Collect per-thread results (also ensures all threads have finished)
        long[][] threadStats = new long[NUM_THREADS][];
        for (int t = 0; t < NUM_THREADS; t++) {
            threadStats[t] = futures.get(t).get();
        }

        executor.shutdown();
        long elapsed = System.currentTimeMillis() - startTime;

        // --- Results ----------------------------------------------------------
        System.out.println("══════════════════════════════════════════════");
        System.out.println("  FINAL ACCOUNT BALANCES");
        System.out.println("══════════════════════════════════════════════");

        long finalTotal = 0;
        for (BankAccount acc : accounts) {
            System.out.printf("  %s%n", acc);
            finalTotal += acc.getBalance();
        }

        System.out.println("──────────────────────────────────────────────");
        System.out.printf("  Initial total balance : $%,d%n", initialTotal);
        System.out.printf("  Final   total balance : $%,d%n", finalTotal);
        System.out.printf("  Balance conserved     : %s%n",
                (initialTotal == finalTotal) ? "✔  YES" : "✘  NO – BUG!");
        System.out.println();

        System.out.println("══════════════════════════════════════════════");
        System.out.println("  THREAD STATISTICS");
        System.out.println("══════════════════════════════════════════════");
        for (int t = 0; t < NUM_THREADS; t++) {
            System.out.printf("  Thread %d : %,d succeeded, %,d failed (insufficient funds)%n",
                    t + 1, threadStats[t][0], threadStats[t][1]);
        }
        System.out.println("──────────────────────────────────────────────");
        System.out.printf("  Total successful transfers : %,d%n", globalSuccess.get());
        System.out.printf("  Total failed    transfers : %,d%n",  globalFail.get());
        System.out.printf("  Elapsed time              : %,d ms%n", elapsed);
        System.out.printf("  Throughput                : %,.0f transfers/sec%n",
                totalTransfers / (elapsed / 1000.0));
        System.out.println();

        // Audit log sample
        System.out.println("══════════════════════════════════════════════");
        System.out.printf("  AUDIT LOG SAMPLE (up to %d entries)%n", AUDIT_SAMPLE_SIZE);
        System.out.println("══════════════════════════════════════════════");
        int shown = 0;
        for (String entry : auditLog) {
            System.out.println("  " + entry);
            if (++shown >= AUDIT_SAMPLE_SIZE) break;
        }
        System.out.println("══════════════════════════════════════════════");
    }
}