import java.util.*;
import java.io.*;

public class UserLoginSystem {
    private static final Map<String, String> credentials = new HashMap<>();
    
    static {
        // Initialize with some default credentials
        credentials.put("admin", "password123");
        credentials.put("user1", "user1pass");
        credentials.put("guest", "guestpass");
    }
    
    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);
        boolean loggedIn = false;
        int maxAttempts = 3;
        int attempts = 0;
        
        System.out.println("=== Welcome to User Login System ===\n");
        
        try {
            while (!loggedIn && attempts < maxAttempts) {
                try {
                    System.out.print("Enter username: ");
                    String username = scanner.nextLine().trim();
                    
                    if (username.isEmpty()) {
                        System.out.println("Error: Username cannot be empty.\n");
                        continue;
                    }
                    
                    System.out.print("Enter password: ");
                    String password = scanner.nextLine().trim();
                    
                    if (password.isEmpty()) {
                        System.out.println("Error: Password cannot be empty.\n");
                        continue;
                    }
                    
                    // Attempt login
                    if (validateCredentials(username, password)) {
                        System.out.println("\n✓ Login successful! Welcome, " + username + "!\n");
                        loggedIn = true;
                    } else {
                        attempts++;
                        int remainingAttempts = maxAttempts - attempts;
                        
                        if (remainingAttempts > 0) {
                            System.out.println("✗ Invalid username or password.");
                            System.out.println("Remaining attempts: " + remainingAttempts + "\n");
                        } else {
                            System.out.println("✗ Invalid username or password.");
                            System.out.println("Account locked due to multiple failed login attempts.\n");
                        }
                    }
                    
                } catch (Exception e) {
                    System.out.println("Error during login: " + e.getMessage());
                    System.out.println("Please try again.\n");
                }
            }
            
            if (!loggedIn && attempts >= maxAttempts) {
                System.out.println("Login failed. Maximum attempts exceeded. Program terminated.");
            }
            
        } catch (InputMismatchException e) {
            System.out.println("Error: Invalid input format. " + e.getMessage());
        } catch (NoSuchElementException e) {
            System.out.println("Error: Input stream closed or unavailable.");
        } catch (Exception e) {
            System.out.println("An unexpected error occurred: " + e.getMessage());
            e.printStackTrace();
        } finally {
            scanner.close();
            System.out.println("Thank you for using the Login System. Goodbye!");
        }
    }
    
    /**
     * Validates the provided username and password against stored credentials.
     * 
     * @param username The username to validate
     * @param password The password to validate
     * @return true if credentials are valid, false otherwise
     * @throws Exception if an error occurs during validation
     */
    private static boolean validateCredentials(String username, String password) throws Exception {
        try {
            if (username == null || password == null) {
                throw new IllegalArgumentException("Username and password cannot be null");
            }
            
            // Check if username exists and password matches
            if (credentials.containsKey(username)) {
                String storedPassword = credentials.get(username);
                return storedPassword.equals(password);
            }
            
            return false;
            
        } catch (Exception e) {
            throw new Exception("Error validating credentials: " + e.getMessage());
        }
    }
    
    /**
     * Adds a new user to the credential store.
     * In a real application, this would be stored securely in a database.
     * 
     * @param username The new*
