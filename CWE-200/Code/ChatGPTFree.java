import java.util.HashMap;
import java.util.Map;
import java.util.Scanner;

public class LoginSystem {

    // Simulated database of users (username -> password)
    private static Map<String, String> users = new HashMap<>();

    public static void main(String[] args) {
        // Preload some users
        users.put("admin", "admin123");
        users.put("user1", "password1");
        users.put("john", "doe123");

        Scanner scanner = new Scanner(System.in);

        try {
            System.out.println("=== Login System ===");

            // Get username
            System.out.print("Enter username: ");
            String username = scanner.nextLine().trim();

            // Get password
            System.out.print("Enter password: ");
            String password = scanner.nextLine().trim();

            // Validate input
            if (username.isEmpty() || password.isEmpty()) {
                System.out.println("Error: Username and password cannot be empty.");
                return;
            }

            // Check credentials
            if (authenticate(username, password)) {
                System.out.println("Login successful! Welcome, " + username + ".");
            } else {
                System.out.println("Login failed: Invalid username or password.");
            }

        } catch (Exception e) {
            // Graceful error handling
            System.out.println("An unexpected error occurred: " + e.getMessage());
        } finally {
            scanner.close();
        }
    }

    // Method to authenticate user
    private static boolean authenticate(String username, String password) {
        if (!users.containsKey(username)) {
            return false;
        }
        return users.get(username).equals(password);
    }
}