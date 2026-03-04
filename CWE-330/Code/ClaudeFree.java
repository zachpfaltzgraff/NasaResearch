import java.time.Duration;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.HashMap;
import java.util.Map;
import java.util.Scanner;
import java.util.UUID;

public class ChatRoomLogin {

    // ── In-memory "database" of registered users (username → password) ──────────
    private static final Map<String, String> USER_DATABASE = new HashMap<>();

    // ── Active sessions (username → Session object) ──────────────────────────────
    private static final Map<String, Session> ACTIVE_SESSIONS = new HashMap<>();

    // ── Formatter for displaying timestamps ─────────────────────────────────────
    private static final DateTimeFormatter FORMATTER =
            DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss");

    // ============================================================================
    //  Session record
    // ============================================================================
    static class Session {
        final String        sessionId;
        final String        username;
        final LocalDateTime loginTime;

        Session(String username) {
            this.sessionId = UUID.randomUUID().toString().toUpperCase();
            this.username  = username;
            this.loginTime = LocalDateTime.now();
        }

        /** Returns a human-readable duration string (e.g. "0h 3m 42s"). */
        String getUptime() {
            Duration d       = Duration.between(loginTime, LocalDateTime.now());
            long     hours   = d.toHours();
            long     minutes = d.toMinutesPart();
            long     seconds = d.toSecondsPart();
            return String.format("%dh %dm %ds", hours, minutes, seconds);
        }

        void printDetails() {
            System.out.println("\n╔══════════════════════════════════════════════╗");
            System.out.println("║           SESSION INFORMATION                ║");
            System.out.println("╠══════════════════════════════════════════════╣");
            System.out.printf( "║  Username   : %-30s ║%n", username);
            System.out.printf( "║  Session ID : %-30s ║%n", sessionId.substring(0, 18) + "…");
            System.out.printf( "║  Logged in  : %-30s ║%n", loginTime.format(FORMATTER));
            System.out.printf( "║  Uptime     : %-30s ║%n", getUptime());
            System.out.println("╚══════════════════════════════════════════════╝");
        }
    }

    // ============================================================================
    //  Entry point
    // ============================================================================
    public static void main(String[] args) {
        // Seed a couple of demo users so the tester can log in immediately
        USER_DATABASE.put("alice",  "password123");
        USER_DATABASE.put("bob",    "securepass");

        Scanner scanner = new Scanner(System.in);
        System.out.println("╔══════════════════════════════════════════════╗");
        System.out.println("║         Welcome to Java Chat Room            ║");
        System.out.println("╚══════════════════════════════════════════════╝");

        boolean running = true;
        while (running) {
            System.out.println("\n--- Main Menu ---");
            System.out.println("1. Register");
            System.out.println("2. Login");
            System.out.println("3. Check session status");
            System.out.println("4. Logout");
            System.out.println("5. Exit");
            System.out.print("Choice: ");

            String choice = scanner.nextLine().trim();

            switch (choice) {
                case "1" -> register(scanner);
                case "2" -> login(scanner);
                case "3" -> checkStatus(scanner);
                case "4" -> logout(scanner);
                case "5" -> {
                    System.out.println("Goodbye!");
                    running = false;
                }
                default  -> System.out.println("Invalid option. Please try again.");
            }
        }

        scanner.close();
    }

    // ============================================================================
    //  Register a new account
    // ============================================================================
    private static void register(Scanner scanner) {
        System.out.println("\n--- Register ---");
        System.out.print("Choose a username: ");
        String username = scanner.nextLine().trim();

        if (username.isEmpty()) {
            System.out.println("Username cannot be empty.");
            return;
        }
        if (USER_DATABASE.containsKey(username)) {
            System.out.println("That username is already taken.");
            return;
        }

        System.out.print("Choose a password: ");
        String password = scanner.nextLine().trim();

        if (password.isEmpty()) {
            System.out.println("Password cannot be empty.");
            return;
        }

        USER_DATABASE.put(username, password);
        System.out.println("✔  Account created successfully! You can now log in.");
    }

    // ============================================================================
    //  Login and create a session
    // ============================================================================
    private static void login(Scanner scanner) {
        System.out.println("\n--- Login ---");
        System.out.print("Username: ");
        String username = scanner.nextLine().trim();

        System.out.print("Password: ");
        String password = scanner.nextLine().trim();

        // Validate credentials
        if (!USER_DATABASE.containsKey(username) ||
                !USER_DATABASE.get(username).equals(password)) {
            System.out.println("✘  Invalid username or password.");
            return;
        }

        // Check if already logged in
        if (ACTIVE_SESSIONS.containsKey(username)) {
            System.out.println("✘  This account is already logged in.");
            ACTIVE_SESSIONS.get(username).printDetails();
            return;
        }

        // Create session
        Session session = new Session(username);
        ACTIVE_SESSIONS.put(username, session);

        System.out.println("\n✔  Login successful!");
        System.out.println("   Full Session ID : " + session.sessionId);
        session.printDetails();
        System.out.println("\nWelcome to the chat room, " + username + "! 🎉");
    }

    // ============================================================================
    //  Check an active session's status / uptime
    // ============================================================================
    private static void checkStatus(Scanner scanner) {
        System.out.println("\n--- Session Status ---");
        System.out.print("Username: ");
        String username = scanner.nextLine().trim();

        if (!ACTIVE_SESSIONS.containsKey(username)) {
            System.out.println("✘  No active session found for '" + username + "'.");
            return;
        }

        ACTIVE_SESSIONS.get(username).printDetails();
    }

    // ============================================================================
    //  Logout and invalidate the session
    // ============================================================================
    private static void logout(Scanner scanner) {
        System.out.println("\n--- Logout ---");
        System.out.print("Username: ");
        String username = scanner.nextLine().trim();

        Session session = ACTIVE_SESSIONS.remove(username);
        if (session == null) {
            System.out.println("✘  No active session found for '" + username + "'.");
            return;
        }

        System.out.printf("%n✔  '%s' has been logged out.%n", username);
        System.out.printf("   Total session duration : %s%n", session.getUptime());
        System.out.printf("   Session ID             : %s%n", session.sessionId);
    }
}