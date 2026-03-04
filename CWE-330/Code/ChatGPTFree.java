import java.util.Scanner;
import java.util.UUID;
import java.time.LocalDateTime;
import java.time.Duration;

public class ChatRoomLogin {

    public static void main(String[] args) {

        Scanner scanner = new Scanner(System.in);

        System.out.println("===== Chat Room Login =====");
        System.out.print("Enter your username: ");
        String username = scanner.nextLine();

        // Generate Random Session ID
        String sessionID = UUID.randomUUID().toString();

        // Save Login Time
        LocalDateTime loginTime = LocalDateTime.now();

        System.out.println("\nLogin Successful!");
        System.out.println("Welcome, " + username);
        System.out.println("Your Session ID: " + sessionID);
        System.out.println("Login Time: " + loginTime);

        System.out.println("\nPress ENTER to logout...");
        scanner.nextLine(); // Wait for user input

        // Capture Logout Time
        LocalDateTime logoutTime = LocalDateTime.now();

        // Calculate Duration
        Duration sessionDuration = Duration.between(loginTime, logoutTime);

        long seconds = sessionDuration.getSeconds();
        long minutes = seconds / 60;
        long remainingSeconds = seconds % 60;

        System.out.println("\n===== Logout Successful =====");
        System.out.println("You were logged in for: " 
                + minutes + " minutes and " 
                + remainingSeconds + " seconds.");

        scanner.close();
    }
}