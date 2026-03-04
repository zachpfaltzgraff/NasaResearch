import java.util.Scanner;
import java.util.UUID;
import java.time.Duration;
import java.time.Instant;

public class ChatLoginSystem {

    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);

        System.out.println("--- Welcome to the Dev Chat Room ---");
        System.out.print("Enter your username to login: ");
        String username = scanner.nextLine();

        // 1. Generate a random Session ID
        String sessionId = UUID.randomUUID().toString();

        // 2. Capture the exact login time
        Instant loginTime = Instant.now();

        System.out.println("\nLogin Successful!");
        System.out.println("User: " + username);
        System.out.println("Session ID: " + sessionId);
        System.out.println("Login Time: " + loginTime);
        System.out.println("\n(Type 'status' to see session duration or 'exit' to quit)");

        // Simple loop to simulate being "logged in"
        while (true) {
            System.out.print("> ");
            String input = scanner.nextLine();

            if (input.equalsIgnoreCase("status")) {
                // 3. Calculate the difference between now and login time
                Duration sessionDuration = Duration.between(loginTime, Instant.now());
                
                long seconds = sessionDuration.getSeconds();
                System.out.printf("Account logged in for: %d seconds%n", seconds);
                
            } else if (input.equalsIgnoreCase("exit")) {
                System.out.println("Logging out... Goodbye!");
                break;
            } else {
                System.out.println("Message sent: " + input);
            }
        }

        scanner.close();
    }
}