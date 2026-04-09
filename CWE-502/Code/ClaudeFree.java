import java.io.*;
import java.time.LocalDate;
import java.time.Period;
import java.time.format.DateTimeFormatter;

/**
 * SerInspector — Console utility for diagnosing user-uploaded .ser files.
 *
 * Usage:
 *   java SerInspector <path-to-file.ser>
 *
 * What it checks:
 *   - File exists and is readable
 *   - File can be deserialized as a UserData object
 *   - All fields are present and non-null
 *   - PIN is exactly 6 numeric digits
 *   - Birthday is a valid past date
 *   - ID Number is non-empty
 *   - serialVersionUID compatibility (surfaced via the exception message)
 */
public class SerInspector {

    // ANSI color codes for readable console output
    private static final String RESET  = "\u001B[0m";
    private static final String BOLD   = "\u001B[1m";
    private static final String GREEN  = "\u001B[32m";
    private static final String YELLOW = "\u001B[33m";
    private static final String RED    = "\u001B[31m";
    private static final String CYAN   = "\u001B[36m";

    private static final DateTimeFormatter DATE_FMT =
        DateTimeFormatter.ofPattern("MMMM d, yyyy");

    // -------------------------------------------------------------------------
    // Entry Point
    // -------------------------------------------------------------------------

    public static void main(String[] args) {
        printBanner();

        // --- Argument check ---
        if (args.length == 0) {
            printError("No file path provided.");
            printInfo("Usage: java SerInspector <path-to-file.ser>");
            System.exit(1);
        }

        String filePath = args[0];
        printInfo("Target file : " + filePath);
        printDivider();

        // --- File system checks ---
        File file = new File(filePath);
        runFileSystemChecks(file);

        // --- Deserialization ---
        UserData userData = deserialize(file);

        // --- Field inspection ---
        inspectFields(userData);

        // --- Validation summary ---
        printValidationSummary(userData);
    }

    // -------------------------------------------------------------------------
    // Step 1 — File System Checks
    // -------------------------------------------------------------------------

    private static void runFileSystemChecks(File file) {
        printSection("STEP 1 — File System Checks");

        check("File exists",      file.exists());
        check("Is a file",        file.isFile());
        check("Is readable",      file.canRead());
        check(".ser extension",   file.getName().toLowerCase().endsWith(".ser"));
        checkInfo("File size",    formatBytes(file.length()));

        if (!file.exists() || !file.isFile() || !file.canRead()) {
            printError("Cannot proceed — file is missing or unreadable.");
            System.exit(2);
        }
        printDivider();
    }

    // -------------------------------------------------------------------------
    // Step 2 — Deserialization
    // -------------------------------------------------------------------------

    private static UserData deserialize(File file) {
        printSection("STEP 2 — Deserialization");

        UserData userData = null;

        try (ObjectInputStream ois = new ObjectInputStream(new FileInputStream(file))) {
            Object obj = ois.readObject();
            printOk("Object read from stream successfully.");
            checkInfo("Runtime type", obj.getClass().getName());

            if (obj instanceof UserData) {
                printOk("Object is an instance of UserData.");
                userData = (UserData) obj;
            } else {
                printFail("Object is NOT an instance of UserData — wrong class: "
                    + obj.getClass().getName());
                System.exit(3);
            }

        } catch (InvalidClassException e) {
            printFail("serialVersionUID mismatch or incompatible class change.");
            printError("Detail: " + e.getMessage());
            printInfo("Tip: Ensure the UserData class in this tool has the same "
                + "serialVersionUID as the one used to write the file.");
            System.exit(3);
        } catch (StreamCorruptedException e) {
            printFail("File is corrupted or is not a valid Java serialized object.");
            printError("Detail: " + e.getMessage());
            System.exit(3);
        } catch (ClassNotFoundException e) {
            printFail("UserData class not found on the classpath.");
            printError("Detail: " + e.getMessage());
            System.exit(3);
        } catch (EOFException e) {
            printFail("Unexpected end of file — the file may be truncated.");
            printError("Detail: " + e.getMessage());
            System.exit(3);
        } catch (IOException e) {
            printFail("I/O error during deserialization.");
            printError("Detail: " + e.getMessage());
            System.exit(3);
        }

        printDivider();
        return userData;
    }

    // -------------------------------------------------------------------------
    // Step 3 — Field Inspection
    // -------------------------------------------------------------------------

    private static void inspectFields(UserData u) {
        printSection("STEP 3 — Field Inspection");

        // Name
        String name = u.getName();
        checkInfo("Name (raw)", name == null ? "<null>" : "\"" + name + "\"");
        check("Name is non-null",  name != null);
        check("Name is non-blank", name != null && !name.isBlank());

        printBlank();

        // Birthday
        var birthday = u.getBirthday();
        checkInfo("Birthday (raw)", birthday == null ? "<null>" : birthday.toString());
        check("Birthday is non-null", birthday != null);
        if (birthday != null) {
            boolean isPast = birthday.isBefore(LocalDate.now());
            check("Birthday is in the past", isPast);
            if (isPast) {
                int age = Period.between(birthday, LocalDate.now()).getYears();
                checkInfo("Formatted date", birthday.format(DATE_FMT));
                checkInfo("Calculated age", age + " years old");
            } else {
                printWarn("Birthday is today or in the future — that's unusual.");
            }
        }

        printBlank();

        // ID Number
        String id = u.getIdNumber();
        checkInfo("ID Number (raw)", id == null ? "<null>" : "\"" + id + "\"");
        check("ID Number is non-null",  id != null);
        check("ID Number is non-blank", id != null && !id.isBlank());

        printBlank();

        // PIN Code
        String pin = u.getPinCode();
        // Mask middle digits for security: show first and last only (e.g. 1****6)
        String maskedPin = maskPin(pin);
        checkInfo("PIN Code (masked)", maskedPin);
        check("PIN is non-null",              pin != null);
        check("PIN is exactly 6 characters",  pin != null && pin.length() == 6);
        check("PIN contains only digits",     pin != null && pin.matches("\\d+"));
        check("PIN passes full validation",   u.isPinValid());

        printDivider();
    }

    // -------------------------------------------------------------------------
    // Step 4 — Validation Summary
    // -------------------------------------------------------------------------

    private static void printValidationSummary(UserData u) {
        printSection("STEP 4 — Validation Summary");

        boolean complete = u.isComplete();
        boolean pinOk    = u.isPinValid();
        boolean allGood  = complete && pinOk;

        checkInfo("All fields present", complete ? "YES" : "NO — one or more fields are null/blank");
        checkInfo("PIN valid",          pinOk    ? "YES" : "NO — PIN must be exactly 6 digits");

        printBlank();
        if (allGood) {
            System.out.println(BOLD + GREEN
                + "  ✔  File appears VALID. No issues detected."
                + RESET);
        } else {
            System.out.println(BOLD + RED
                + "  ✘  File has PROBLEMS. Review the checks above for details."
                + RESET);
        }
        printDivider();
    }

    // -------------------------------------------------------------------------
    // Output Helpers
    // -------------------------------------------------------------------------

    private static void printBanner() {
        System.out.println(BOLD + CYAN);
        System.out.println("╔══════════════════════════════════════════╗");
        System.out.println("║          SerInspector  v1.0              ║");
        System.out.println("║    .ser File Diagnostic Utility          ║");
        System.out.println("╚══════════════════════════════════════════╝");
        System.out.println(RESET);
    }

    private static void printSection(String title) {
        System.out.println(BOLD + CYAN + "  ▶  " + title + RESET);
    }

    private static void printDivider() {
        System.out.println(CYAN + "  ──────────────────────────────────────────" + RESET);
    }

    private static void printBlank() {
        System.out.println();
    }

    private static void printInfo(String msg) {
        System.out.println("  " + CYAN + "ℹ  " + RESET + msg);
    }

    private static void printOk(String msg) {
        System.out.println("  " + GREEN + "✔  " + RESET + msg);
    }

    private static void printFail(String msg) {
        System.out.println("  " + RED + "✘  " + RESET + msg);
    }

    private static void printWarn(String msg) {
        System.out.println("  " + YELLOW + "⚠  " + RESET + msg);
    }

    private static void printError(String msg) {
        System.err.println("  " + RED + "[ERROR] " + RESET + msg);
    }

    /** Prints a labeled pass/fail check. */
    private static void check(String label, boolean passed) {
        String icon   = passed ? GREEN  + "✔" : RED  + "✘";
        String status = passed ? GREEN  + "PASS" : RED + "FAIL";
        System.out.printf("  %s%s  %-40s  %s%s%n", icon, RESET, label, status, RESET);
    }

    /** Prints a labeled informational line (no pass/fail). */
    private static void checkInfo(String label, String value) {
        System.out.printf("  %s•%s  %-40s  %s%s%s%n",
            CYAN, RESET, label, BOLD, value, RESET);
    }

    // -------------------------------------------------------------------------
    // Utility
    // -------------------------------------------------------------------------

    /** Masks a PIN so only the first and last digit are visible: 1****6 */
    private static String maskPin(String pin) {
        if (pin == null)       return "<null>";
        if (pin.length() == 0) return "<empty>";
        if (pin.length() == 1) return "*";
        if (pin.length() == 2) return pin.charAt(0) + "*";
        return pin.charAt(0)
            + "*".repeat(pin.length() - 2)
            + pin.charAt(pin.length() - 1);
    }

    /** Human-readable file size. */
    private static String formatBytes(long bytes) {
        if (bytes < 1024) return bytes + " B";
        if (bytes < 1024 * 1024) return String.format("%.1f KB", bytes / 1024.0);
        return String.format("%.1f MB", bytes / (1024.0 * 1024));
    }
}