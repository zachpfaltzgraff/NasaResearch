import java.security.SecureRandom;
import java.time.Duration;
import java.time.Instant;
import java.time.ZoneId;
import java.time.format.DateTimeFormatter;
import java.util.Scanner;

/**
 * Simple console-based "chat room login" demo.
 * - Prompts user for a username (and optional password just for show)
 * - Generates a random session ID
 * - Stores login time and can display how long the user has been logged in
 *
 * NOTE: This is not secure authentication (no real password checking, hashing, DB, etc.).
 */
public class ChatRoomLogin {

    private static final SecureRandom RNG = new SecureRandom();
    private static final DateTimeFormatter TIME_FMT =
            DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss").withZone(ZoneId.systemDefault());

    static class Session {
        private final String username;
        private final String sessionId;
        private final Instant loginTime;

        Session(String username, String sessionId, Instant loginTime) {
            this.username = username;
            this.sessionId = sessionId;
            this.loginTime = loginTime;
        }

        public String getUsername() { return username; }
        public String getSessionId() { return sessionId; }
        public Instant getLoginTime() { return loginTime; }

        public Duration getLoggedInDuration() {
            return Duration.between(loginTime, Instant.now());
        }
    }

    public static void main(String[] args) {
        try (Scanner scanner = new Scanner(System.in)) {

            System.out.println("=== Chat Room Login ===");

            System.out.print("Username: ");
            String username = scanner.nextLine().trim();
            while (username.isEmpty()) {
                System.out.print("Username cannot be empty. Username: ");
                username = scanner.nextLine().trim();
            }

            // Optional "password" prompt (not used for real authentication here)
            System.out.print("Password (not validated in this demo): ");
            scanner.nextLine();

            // Create the session
            String sessionId = generateSessionId(24); // 24 bytes -> 48 hex chars
            Instant loginTime = Instant.now();
            Session session = new Session(username, sessionId, loginTime);

            System.out.println();
            System.out.println("Login successful!");
            System.out.println("User:       " + session.getUsername());
            System.out.println("Session ID: " + session.getSessionId());
            System.out.println("Login time: " + TIME_FMT.format(session.getLoginTime()));
            System.out.println();

            // Simple "chat room" menu loop
            boolean running = true;
            while (running) {
                System.out.println("Choose an option:");
                System.out.println("1) Show how long I've been logged in");
                System.out.println("2) Send a message (demo)");
                System.out.println("3) Logout");
                System.out.print("> ");

                String choice = scanner.nextLine().trim();
                switch (choice) {
                    case "1" -> showLoggedInTime(session);
                    case "2" -> sendMessage(scanner, session);
                    case "3" -> {
                        running = false;
                        System.out.println("Logged out. Goodbye!");
                    }
                    default -> System.out.println("Invalid option.\n");
                }
            }
        }
    }

    private static void showLoggedInTime(Session session) {
        Duration d = session.getLoggedInDuration();
        long hours = d.toHours();
        long minutes = d.toMinutesPart();
        long seconds = d.toSecondsPart();

        System.out.printf("You have been logged in for %d hour(s), %d minute(s), %d second(s).%n%n",
                hours, minutes, seconds);
    }

    private static void sendMessage(Scanner scanner, Session session) {
        System.out.print("Enter message: ");
        String msg = scanner.nextLine();
        // In a real chat room this would broadcast to other users.
        System.out.println("[" + session.getUsername() + "] " + msg + "\n");
    }

    // Generates a cryptographically-strong random session id in hex.
    private static String generateSessionId(int numBytes) {
        byte[] bytes = new byte[numBytes];
        RNG.nextBytes(bytes);
        return toHex(bytes);
    }

    private static String toHex(byte[] bytes) {
        StringBuilder sb = new StringBuilder(bytes.length * 2);
        for (byte b : bytes) sb.append(String.format("%02x", b));
        return sb.toString();
    }
}