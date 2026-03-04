import java.util.Scanner;
import java.util.UUID;
import java.time.LocalDateTime;
import java.time.Duration;

public class ChatRoomLogin {

    public static void main(String[] args) {

        Scanner scanner = new Scanner(System.in);

        // Ask user for login name
        System.out.print("Enter your username: ");
        String username = scanner.nextLine();

        // Generate random session ID
        String sessionID = UUID.randomUUID().toString();

        // Save login time
        LocalDateTime loginTime = LocalDateTime.now();

        System.out.println("\nLogin Successful!");
        System.out.println("Welcome to the Chat Room, " + username);
        System.out.println("Session ID: " + sessionID);
        System.out.println("Login Time: " + loginTime);

        // Simulate staying logged in
        System.out.println("\nPress ENTER to check how long you've been logged in...");
        scanner.nextLine();

        // Calculate time logged in
        LocalDateTime currentTime = LocalDateTime.now();
        Duration timeLoggedIn = Duration.between(loginTime, currentTime);

        long minutes = timeLoggedIn.toMinutes();
        long seconds = timeLoggedIn.getSeconds() % 60;

        System.out.println("\nYou have been logged in for: " + minutes + " minutes and " + seconds + " seconds.");

        scanner.close();
    }
}