import java.io.*;
import java.util.Scanner;

// Serializable User class (must match the one used to create the .ser file)
class User implements Serializable {
    private static final long serialVersionUID = 1L;

    String name;
    String birthday;
    String idNumber;
    String pinCode;

    public User(String name, String birthday, String idNumber, String pinCode) {
        this.name = name;
        this.birthday = birthday;
        this.idNumber = idNumber;
        this.pinCode = pinCode;
    }
}

public class SerFileInspector {

    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);

        System.out.print("Enter path to .ser file: ");
        String filePath = scanner.nextLine();

        inspectSerFile(filePath);
    }

    public static void inspectSerFile(String filePath) {
        try (ObjectInputStream ois = new ObjectInputStream(new FileInputStream(filePath))) {

            Object obj = ois.readObject();

            if (obj instanceof User) {
                User user = (User) obj;

                System.out.println("\n=== USER DATA ===");
                System.out.println("Name: " + user.name);
                System.out.println("Birthday: " + user.birthday);
                System.out.println("ID Number: " + user.idNumber);
                System.out.println("Pin Code: " + user.pinCode);

                // Extra validation
                validateUser(user);

            } else {
                System.out.println("Error: Object is not of type User.");
                System.out.println("Actual type: " + obj.getClass().getName());
            }

        } catch (FileNotFoundException e) {
            System.out.println("Error: File not found.");
        } catch (InvalidClassException e) {
            System.out.println("Error: Class version mismatch.");
            System.out.println("Details: " + e.getMessage());
        } catch (StreamCorruptedException e) {
            System.out.println("Error: Corrupted .ser file.");
        } catch (IOException e) {
            System.out.println("IO Error: " + e.getMessage());
        } catch (ClassNotFoundException e) {
            System.out.println("Error: Class definition not found.");
        }
    }

    private static void validateUser(User user) {
        System.out.println("\n=== VALIDATION CHECKS ===");

        if (user.name == null || user.name.isEmpty()) {
            System.out.println("Warning: Name is missing.");
        }

        if (user.birthday == null || user.birthday.isEmpty()) {
            System.out.println("Warning: Birthday is missing.");
        }

        if (user.idNumber == null || user.idNumber.isEmpty()) {
            System.out.println("Warning: ID Number is missing.");
        }

        if (user.pinCode == null || user.pinCode.length() != 6) {
            System.out.println("Warning: Pin Code is invalid (must be 6 digits).");
        } else {
            System.out.println("Pin Code format looks valid.");
        }
    }
}