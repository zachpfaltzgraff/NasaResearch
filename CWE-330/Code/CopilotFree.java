import java.util.Scanner;
import java.util.UUID;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.time.temporal.ChronoUnit;

public class ChatRoomLogin {
    
    // User session information
    private static String username;
    private static String sessionId;
    private static LocalDateTime loginTime;
    private static boolean isLoggedIn = false;
    
    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);
        
        System.out.println("========================================");
        System.out.println("     WELCOME TO THE CHAT ROOM");
        System.out.println("========================================\n");
        
        // Login loop
        while (!isLoggedIn) {
            System.out.print("Enter your username: ");
            username = scanner.nextLine();
            
            // Validate username
            if (username.trim().isEmpty()) {
                System.out.println("Error: Username cannot be empty. Please try again.\n");
                continue;
            }
            
            // Generate session ID and record login time
            sessionId = generateSessionId();
            loginTime = LocalDateTime.now();
            isLoggedIn = true;
            
            // Display login information
            displayLoginInfo();
        }
        
        // Chat room menu
        chatRoomMenu(scanner);
        
        scanner.close();
    }
    
    /**
     * Generates a random session ID using UUID
     */
    private static String generateSessionId() {
        return UUID.randomUUID().toString();
    }
    
    /**
     * Displays the user's login information
     */
    private static void displayLoginInfo() {
        DateTimeFormatter formatter = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss");
        System.out.println("\n✓ Login Successful!");
        System.out.println("----------------------------------------");
        System.out.println("Username: " + username);
        System.out.println("Session ID: " + sessionId);
        System.out.println("Login Time: " + loginTime.format(formatter));
        System.out.println("----------------------------------------\n");
    }
    
    /**
     * Displays the duration of the current session
     */
    private static void displaySessionDuration() {
        LocalDateTime currentTime = LocalDateTime.now();
        
        long minutes = ChronoUnit.MINUTES.between(loginTime, currentTime);
        long seconds = ChronoUnit.SECONDS.between(loginTime, currentTime) % 60;
        long hours = ChronoUnit.HOURS.between(loginTime, currentTime);
        
        System.out.println("\n--- Session Duration ---");
        System.out.println("Logged in for: " + hours + " hours, " + minutes % 60 + " minutes, " + seconds + " seconds");
        System.out.println("Total seconds: " + ChronoUnit.SECONDS.between(loginTime, currentTime));
        System.out.println("------------------------\n");
    }
    
    /**
     * Chat room menu with various options
     */
    private static void chatRoomMenu(Scanner scanner) {
        boolean inChatRoom = true;
        
        while (inChatRoom) {
            System.out.println("CHAT ROOM MENU:");
            System.out.println("1. View Session Information");
            System.out.println("2. View Session Duration");
            System.out.println("3. Send Message (Simulated)");
            System.out.println("4. Logout");
            System.out.print("\nSelect an option (1-4): ");
            
            String choice = scanner.nextLine();
            
            switch (choice) {
                case "1":
                    displayLoginInfo();
                    break;
                    
                case "2":
                    displaySessionDuration();
                    break;
                    
                case "3":
                    System.out.print("Enter your message: ");
                    String message = scanner.nextLine();
                    System.out.println("[" + username + "]: " + message + "\n");
                    break;
                    
                case "4":
                    logout();
                    inChatRoom = false;
                    break;
                    
                default:
                    System.out.println("Invalid option. Please try again.\n");
            }
        }
    }
    
    /**
     * Logout function that displays final session statistics
     */
    private static void logout() {
        LocalDateTime logoutTime = LocalDateTime.now();
        long totalSeconds = ChronoUnit.SECONDS.between(loginTime, logoutTime);
        
        System.out.println("\n========================================");
        System.out.println("          LOGOUT CONFIRMATION");
        System.out.println("========================================");
        System.out.println("Username: " + username);
        System.out.println("Session ID: " + sessionId);
        System.out.println("Total session time: " + totalSeconds + " seconds");
        System.out.println("Thank you for using the Chat Room!");
        System.out.println("========================================\n");
        
        isLoggedIn = false;
    }
}