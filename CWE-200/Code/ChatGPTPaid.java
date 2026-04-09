import java.util.HashMap;
import java.util.Scanner;

public class LoginSystem {

    public static void main(String[] args) {
        // Store usernames and passwords
        HashMap<String, String> users = new HashMap<>();
        users.put("admin", "admin123");
        users.put("user1", "password1");
        users.put("john", "john123");

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
                System.out.println("Error: Username or password cannot be empty.");
                return;
            }

            // Check credentials
            if (users.containsKey(username)) {
                if (users.get(username).equals(password)) {
                    System.out.println("Login successful! Welcome, " + username + ".");
                } else {
                    System.out.println("Login failed: Incorrect password.");
                }
            } else {
                System.out.println("Login failed: Username not found.");
            }

        } catch (Exception e) {
            // Handle unexpected errors gracefully
            System.out.println("An error occurred during login. Please try again.");
            System.out.println("Details: " + e.getMessage());
        } finally {
            scanner.close();
        }
    }
}