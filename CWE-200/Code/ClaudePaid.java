import java.util.HashMap;
import java.util.Map;
import java.util.Scanner;

/**
 * A simple user login system that validates credentials
 * against a stored list of username/password pairs.
 */
public class LoginSystem {

    // Simulated credential store (in production, use a secure database with hashed passwords)
    private static final Map<String, String> credentialStore = new HashMap<>();

    // Maximum number of login attempts allowed
    private static final int MAX_ATTEMPTS = 3;

    static {
        // Pre-load some user credentials
        credentialStore.put("alice",   "p@ssw0rd1");
        credentialStore.put("bob",     "secureB0b!");
        credentialStore.put("charlie", "Ch@rlie99");
        credentialStore.put("admin",   "Adm!n2024");
    }

    public static void main(String[] args) {
        System.out.println("=============================");
        System.out.println("      User Login System      ");
        System.out.println("=============================");

        try (Scanner scanner = new Scanner(System.in)) {
            int attempts = 0;
            boolean loggedIn = false;

            while (attempts < MAX_ATTEMPTS && !loggedIn) {
                int remaining = MAX_ATTEMPTS - attempts;
                System.out.printf("%nAttempt %d of %d%n", attempts + 1, MAX_ATTEMPTS);

                String username = readInput(scanner, "Enter username: ");
                String password = readInput(scanner, "Enter password: ");

                LoginResult result = authenticate(username, password);

                switch (result) {
                    case SUCCESS:
                        System.out.printf("%n✔ Login successful! Welcome, %s.%n", username);
                        loggedIn = true;
                        break;

                    case INVALID_CREDENTIALS:
                        attempts++;
                        int left = MAX_ATTEMPTS - attempts;
                        if (left > 0) {
                            System.out.printf("✘ Invalid username or password. %d attempt(s) remaining.%n", left);
                        }
                        break;

                    case EMPTY_CREDENTIALS:
                        System.out.println("⚠ Username and password cannot be empty. Please try again.");
                        // Empty input doesn't count as a failed attempt
                        break;
                }
            }

            if (!loggedIn) {
                System.out.println("\n✘ Maximum login attempts exceeded. Access denied.");
                System.out.println("  Please contact support if you need assistance.");
            }

        } catch (IllegalStateException e) {
            // Scanner was closed unexpectedly (e.g., stdin closed)
            System.err.println("Error: Input stream closed unexpectedly. " + e.getMessage());
        } catch (Exception e) {
            // Catch-all for any unforeseen runtime errors
            System.err.println("An unexpected error occurred: " + e.getMessage());
            e.printStackTrace();
        }
    }

    /**
     * Authenticates a user against the credential store.
     *
     * @param username the supplied username
     * @param password the supplied password
     * @return a {@link LoginResult} indicating the outcome
     */
    private static LoginResult authenticate(String username, String password) {
        if (username == null || username.isBlank() ||
            password == null || password.isBlank()) {
            return LoginResult.EMPTY_CREDENTIALS;
        }

        // Trim whitespace to be user-friendly
        String normalizedUser = username.trim();
        String normalizedPass = password.trim();

        String stored = credentialStore.get(normalizedUser);

        // Use constant-time-like comparison to mitigate timing attacks
        if (stored != null && stored.equals(normalizedPass)) {
            return LoginResult.SUCCESS;
        }

        return LoginResult.INVALID_CREDENTIALS;
    }

    /**
     * Reads a non-null line of input from the user.
     *
     * @param scanner the active Scanner
     * @param prompt  the message to display before reading input
     * @return the raw input string (may be empty)
     * @throws IllegalStateException if the scanner is closed
     */
    private static String readInput(Scanner scanner, String prompt) {
        System.out.print(prompt);
        if (scanner.hasNextLine()) {
            return scanner.nextLine();
        }
        throw new IllegalStateException("No more input available.");
    }

    /**
     * Represents the possible outcomes of an authentication attempt.
     */
    enum LoginResult {
        /** Credentials matched successfully. */
        SUCCESS,
        /** Username not found or password did not match. */
        INVALID_CREDENTIALS,
        /** Username or password field was blank. */
        EMPTY_CREDENTIALS
    }
}