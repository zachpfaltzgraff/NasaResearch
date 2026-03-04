import java.util.Scanner;
import java.util.UUID;
import java.time.LocalDateTime;
import java.time.Duration;
import java.time.format.DateTimeFormatter;

public class ChatRoomLogin {

    // Simulated user database (username -> password)
    private static final String[][] USER_DB = {
        {"alice",   "password123"},
        {"bob",     "securepass"},
        {"charlie", "chatroom99"},
        {"guest",   "guest"}
    };

    // Active session data
    private static String loggedInUser  = null;
    private static String sessionId     = null;
    private static LocalDateTime loginTime = null;

    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);

        System.out.println("==========================================");
        System.out.println("        Welcome to JavaChat Room          ");
        System.out.println("==========================================");

        boolean running = true;
        while (running) {
            if (loggedInUser == null) {
                // ── Not logged in: show auth menu ──────────────────────────
                System.out.println("\n[1] Login");
                System.out.println("[2] Exit");
                System.out.print("Choose an option: ");
                String choice = scanner.nextLine().trim();

                switch (choice) {
                    case "1" -> login(scanner);
                    case "2" -> {
                        System.out.println("Goodbye!");
                        running = false;
                    }
                    default  -> System.out.println("Invalid option. Try again.");
                }
            } else {
                // ── Logged in: show chat menu ──────────────────────────────
                System.out.println("\n[1] View Session Info");
                System.out.println("[2] Check Login Duration");
                System.out.println("[3] Send a Message");
                System.out.println("[4] Logout");
                System.out.println("[5] Exit");
                System.out.print("Choose an option: ");
                String choice = scanner.nextLine().trim();

                switch (choice) {
                    case "1" -> viewSessionInfo();
                    case "2" -> checkLoginDuration();
                    case "3" -> sendMessage(scanner);
                    case "4" -> logout();
                    case "5" -> {
                        logout();
                        System.out.println("Goodbye!");
                        running = false;
                    }
                    default  -> System.out.println("Invalid option. Try again.");
                }
            }
        }

        scanner.close();
    }

    // ── Login ──────────────────────────────────────────────────────────────────
    private static void login(Scanner scanner) {
        System.out.println("\n--- Login ---");
        System.out.print("Username: ");
        String username = scanner.nextLine().trim();

        System.out.print("Password: ");
        String password = scanner.nextLine().trim();

        if (authenticate(username, password)) {
            loggedInUser = username;
            sessionId    = generateSessionId();
            loginTime    = LocalDateTime.now();

            System.out.println("\n✔ Login successful! Welcome, " + loggedInUser + "!");
            System.out.println("  Session ID  : " + sessionId);
            System.out.println("  Login Time  : " + formatDateTime(loginTime));
        } else {
            System.out.println("✘ Invalid username or password. Please try again.");
        }
    }

    // ── Authentication ─────────────────────────────────────────────────────────
    private static boolean authenticate(String username, String password) {
        for (String[] user : USER_DB) {
            if (user[0].equalsIgnoreCase(username) && user[1].equals(password)) {
                return true;
            }
        }
        return false;
    }

    // ── Session ID generator ───────────────────────────────────────────────────
    private static String generateSessionId() {
        // Combines a UUID with a short timestamp-derived suffix for uniqueness
        String uuid      = UUID.randomUUID().toString().replace("-", "").toUpperCase();
        String timestamp = String.valueOf(System.currentTimeMillis()).substring(7); // last 6 digits
        return "SID-" + uuid.substring(0, 8) + "-" + timestamp;
    }

    // ── View session info ──────────────────────────────────────────────────────
    private static void viewSessionInfo() {
        System.out.println("\n--- Session Info ---");
        System.out.println("  Username   : " + loggedInUser);
        System.out.println("  Session ID : " + sessionId);
        System.out.println("  Login Time : " + formatDateTime(loginTime));
    }

    // ── Login duration ─────────────────────────────────────────────────────────
    private static void checkLoginDuration() {
        Duration duration = Duration.between(loginTime, LocalDateTime.now());

        long hours   = duration.toHours();
        long minutes = duration.toMinutesPart();
        long seconds = duration.toSecondsPart();

        System.out.println("\n--- Login Duration ---");
        System.out.printf("  You have been logged in for: %02d:%02d:%02d (HH:MM:SS)%n",
                          hours, minutes, seconds);
    }

    // ── Send a message ─────────────────────────────────────────────────────────
    private static void sendMessage(Scanner scanner) {
        System.out.print("\n[" + loggedInUser + "] Type your message: ");
        String message = scanner.nextLine().trim();

        if (!message.isEmpty()) {
            String timestamp = formatDateTime(LocalDateTime.now());
            System.out.println("\n  [" + timestamp + "] <" + loggedInUser + ">: " + message);
        } else {
            System.out.println("  Message cannot be empty.");
        }
    }

    // ── Logout ─────────────────────────────────────────────────────────────────
    private static void logout() {
        if (loggedInUser != null) {
            checkLoginDuration();   // show total session time before clearing
            System.out.println("\n✔ " + loggedInUser + " has been logged out. Session " + sessionId + " ended.");
            loggedInUser = null;
            sessionId    = null;
            loginTime    = null;
        }
    }

    // ── Utility: format date/time ──────────────────────────────────────────────
    private static String formatDateTime(LocalDateTime dt) {
        return dt.format(DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss"));
    }
}