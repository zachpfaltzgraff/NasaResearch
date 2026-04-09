import java.util.HashMap;
import java.util.Map;
import java.util.Scanner;

public class LoginSystem {

    // In a real system, never store plaintext passwords.
    // This is a simple educational example.
    private static final Map<String, String> CREDENTIALS = new HashMap<>();

    static {
        CREDENTIALS.put("alice", "password123");
        CREDENTIALS.put("bob", "qwerty");
        CREDENTIALS.put("admin", "adminpass");
    }

    public static void main(String[] args) {
        try (Scanner scanner = new Scanner(System.in)) {

            System.out.print("Enter username: ");
            String username = safeReadLine(scanner);

            System.out.print("Enter password: ");
            String password = safeReadLine(scanner);

            if (username.isBlank() || password.isBlank()) {
                System.out.println("Login failed: username and password must not be empty.");
                return;
            }

            if (authenticate(username, password)) {
                System.out.println("Login successful. Welcome, " + username + "!");
            } else {
                // Avoid telling the user which field was wrong (basic security best practice)
                System.out.println("Login failed: invalid username or password.");
            }

        } catch (Exception e) {
            // Graceful error handling: don’t crash, inform the user
            System.out.println("An unexpected error occurred during login. Please try again.");
            // Optional for debugging (comment out in production):
            // e.printStackTrace();
        }
    }

    private static boolean authenticate(String username, String password) {
        String storedPassword = CREDENTIALS.get(username);
        return storedPassword != null && storedPassword.equals(password);
    }

    private static String safeReadLine(Scanner scanner) {
        String line = scanner.nextLine();
        return line == null ? "" : line.trim();
    }
}