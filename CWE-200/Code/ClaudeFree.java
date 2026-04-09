import java.util.HashMap;
import java.util.Map;
import java.util.Scanner;

/**
 * A simple user login system that validates credentials
 * against a stored list and handles errors gracefully.
 */
public class LoginSystem {

    // Simulated credential store: username -> hashed password
    // In a real system, passwords would be hashed (e.g., BCrypt).
    // Here we use a simple SHA-256-like placeholder for demonstration.
    private static final Map<String, String> USER_CREDENTIALS = new HashMap<>();

    // Maximum allowed login attempts before lockout
    private static final int MAX_ATTEMPTS = 3;

    static {
        // Pre-populate with some test users (password stored as-is for demo clarity)
        // Format: username -> password
        USER_CREDENTIALS.put("alice",   hashPassword("alice123"));
        USER_CREDENTIALS.put("bob",     hashPassword("securePass!"));
        USER_CREDENTIALS.put("charlie", hashPassword("Ch@rlie99"));
        USER_CREDENTIALS.put("admin",   hashPassword("Admin@2024"));
    }

    // -----------------------------------------------------------------------
    // Password hashing (simple for demo; use BCrypt in production)
    // -----------------------------------------------------------------------
    private static String hashPassword(String password) {
        // Using Java's built-in hashCode for demo purposes only.
        // NEVER use this in production — use BCrypt, Argon2, or PBKDF2.
        return String.valueOf(password.hashCode());
    }

    // -----------------------------------------------------------------------
    // Validation helpers
    // -----------------------------------------------------------------------
    private static void validateInput(String username, String password) throws IllegalArgumentException {
        if (username == null || username.trim().isEmpty()) {
            throw new IllegalArgumentException("Username cannot be empty.");
        }
        if (password == null || password.isEmpty()) {
            throw new IllegalArgumentException("Password cannot be empty.");
        }
        if (username.length() > 50) {
            throw new IllegalArgumentException("Username exceeds maximum length of 50 characters.");
        }
        if (password.length() > 100) {
            throw new IllegalArgumentException("Password exceeds maximum length of 100 characters.");
        }
    }

    // -----------------------------------------------------------------------
    // Core authentication logic
    // -----------------------------------------------------------------------
    /**
     * Attempts to authenticate a user with the given credentials.
     *
     * @param username The entered username.
     * @param password The entered password (plain text).
     * @return true if authentication succeeds, false otherwise.
     * @throws IllegalArgumentException if inputs are invalid.
     */
    public static boolean authenticate(String username, String password)
            throws IllegalArgumentException {

        // 1. Validate inputs
        validateInput(username, password);

        // 2. Normalize username (case-insensitive lookup)
        String normalizedUsername = username.trim().toLowerCase();

        // 3. Check if user exists
        if (!USER_CREDENTIALS.containsKey(normalizedUsername)) {
            return false; // User not found — same message as wrong password (security best practice)
        }

        // 4. Compare hashed passwords
        String storedHash   = USER_CREDENTIALS.get(normalizedUsername);
        String enteredHash  = hashPassword(password);

        return storedHash.equals(enteredHash);
    }

    // -----------------------------------------------------------------------
    // Display helpers
    // -----------------------------------------------------------------------
    private static void printBanner() {
        System.out.println("╔══════════════════════════════════════╗");
        System.out.println("║         USER LOGIN SYSTEM v1.0       ║");
        System.out.println("╚══════════════════════════════════════╝");
        System.out.println();
    }

    private static void printSuccess(String username) {
        System.out.println();
        System.out.println("✅  Login successful! Welcome, " + username.trim() + ".");
        System.out.println("    You are now logged in.");
        System.out.println();
    }

    private static void printFailure(int attemptsLeft) {
        System.out.println();
        System.out.println("❌  Login failed. Invalid username or password.");
        if (attemptsLeft > 0) {
            System.out.println("    Attempts remaining: " + attemptsLeft);
        }
        System.out.println();
    }

    private static void printLockout() {
        System.out.println();
        System.out.println("🔒  Account locked. Too many failed login attempts.");
        System.out.println("    Please contact your administrator to unlock your account.");
        System.out.println();
    }

    // -----------------------------------------------------------------------
    // Main entry point
    // -----------------------------------------------------------------------
    public static void main(String[] args) {
        printBanner();

        Scanner scanner = new Scanner(System.in);
        int attemptsLeft = MAX_ATTEMPTS;
        boolean loggedIn = false;

        while (attemptsLeft > 0 && !loggedIn) {
            try {
                // --- Prompt for username ---
                System.out.print("Username: ");
                String username = scanner.nextLine();

                // --- Prompt for password (plain text for console demo) ---
                System.out.print("Password: ");
                String password = scanner.nextLine();

                // --- Attempt authentication ---
                boolean success = authenticate(username, password);

                if (success) {
                    loggedIn = true;
                    printSuccess(username);
                    // >>> Place post-login application logic here <<<
                } else {
                    attemptsLeft--;
                    if (attemptsLeft > 0) {
                        printFailure(attemptsLeft);
                    }
                }

            } catch (IllegalArgumentException e) {
                // Input validation error — inform the user and let them retry
                System.out.println();
                System.out.println("⚠️  Input error: " + e.getMessage());
                System.out.println("    Please try again.");
                System.out.println();
                // Validation errors don't consume an attempt

            } catch (Exception e) {
                // Unexpected system error — log and exit gracefully
                System.out.println();
                System.out.println("⚠️  An unexpected error occurred: " + e.getMessage());
                System.out.println("    The system will now exit. Please try again later.");
                System.out.println();
                scanner.close();
                System.exit(1);
            }
        }

        if (!loggedIn) {
            printLockout();
        }

        scanner.close();
    }
}