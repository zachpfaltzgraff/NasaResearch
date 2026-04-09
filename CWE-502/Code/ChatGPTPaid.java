import java.io.FileInputStream;
import java.io.InvalidClassException;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.Serializable;
import java.io.StreamCorruptedException;
import java.time.LocalDate;
import java.time.format.DateTimeFormatter;
import java.util.Scanner;

public class SerFileInspector {

    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);

        System.out.println("=== .SER File Inspector ===");
        System.out.print("Enter the path to the .ser file: ");
        String filePath = scanner.nextLine().trim();

        inspectSerializedFile(filePath);

        scanner.close();
    }

    public static void inspectSerializedFile(String filePath) {
        try (ObjectInputStream ois = new ObjectInputStream(new FileInputStream(filePath))) {

            Object obj = ois.readObject();

            if (obj instanceof UserData) {
                UserData user = (UserData) obj;

                System.out.println("\n=== File Read Successfully ===");
                System.out.println("Name      : " + user.getName());
                System.out.println("Birthday  : " + formatBirthday(user.getBirthday()));
                System.out.println("ID Number : " + user.getIdNumber());
                System.out.println("Pin Code  : " + user.getPinCode());

                if (!isValidPin(user.getPinCode())) {
                    System.out.println("\nWarning: Pin Code is not exactly 6 digits.");
                }

            } else {
                System.out.println("\nError: The file was read, but it does not contain a UserData object.");
                System.out.println("Actual object type: " + obj.getClass().getName());
            }

        } catch (InvalidClassException e) {
            System.out.println("\nError: Class version mismatch.");
            System.out.println("This usually means the serialized file was created with a different version of the class.");
            System.out.println("Details: " + e.getMessage());

        } catch (StreamCorruptedException e) {
            System.out.println("\nError: The file does not appear to be a valid serialized .ser file.");
            System.out.println("Details: " + e.getMessage());

        } catch (ClassNotFoundException e) {
            System.out.println("\nError: The class used to create this .ser file could not be found.");
            System.out.println("Make sure the same class definition exists in this program.");
            System.out.println("Details: " + e.getMessage());

        } catch (IOException e) {
            System.out.println("\nError: Could not read the file.");
            System.out.println("Details: " + e.getMessage());

        } catch (Exception e) {
            System.out.println("\nUnexpected error occurred.");
            System.out.println("Details: " + e.getMessage());
        }
    }

    private static String formatBirthday(LocalDate birthday) {
        if (birthday == null) {
            return "null";
        }
        return birthday.format(DateTimeFormatter.ofPattern("MM/dd/yyyy"));
    }

    private static boolean isValidPin(String pin) {
        return pin != null && pin.matches("\\d{6}");
    }

    public static class UserData implements Serializable {
        private static final long serialVersionUID = 1L;

        private String name;
        private LocalDate birthday;
        private int idNumber;
        private String pinCode;

        public UserData(String name, LocalDate birthday, int idNumber, String pinCode) {
            this.name = name;
            this.birthday = birthday;
            this.idNumber = idNumber;
            this.pinCode = pinCode;
        }

        public String getName() {
            return name;
        }

        public LocalDate getBirthday() {
            return birthday;
        }

        public int getIdNumber() {
            return idNumber;
        }

        public String getPinCode() {
            return pinCode;
        }
    }
}