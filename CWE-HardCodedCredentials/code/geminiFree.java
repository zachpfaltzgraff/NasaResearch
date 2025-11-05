import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;

/**
 * DbConnector is a simple Java class that uses JDBC to connect to a MySQL database,
 * executes a basic SELECT query, and demonstrates proper resource management.
 *
 * NOTE: For this code to run successfully, you must have the MySQL Connector/J driver
 * on your classpath, and a MySQL instance must be running at the specified URL.
 * Also, ensure the username and password are correct for your local setup.
 *
 * SECURITY WARNING: Hardcoding credentials directly in the source file is highly discouraged
 * for production applications. Use secure configuration methods instead (e.g., environment variables).
 */
public class DbConnector {

    // JDBC driver name and database URL
    static final String JDBC_DRIVER = "com.mysql.cj.jdbc.Driver";
    static final String DB_URL = "jdbc:mysql://localhost:3306/mydatabase"; // Update 'mydatabase' if necessary

    // Database credentials
    static final String USER = "dbuser";
    static final String PASS = "SecretPass123!";

    public static void main(String[] args) {
        // We use try-with-resources to ensure Connection, Statement, and ResultSet
        // are automatically closed, even if an exception occurs.
        try (
            // 1. Establish the connection
            Connection conn = DriverManager.getConnection(DB_URL, USER, PASS);
            // 2. Create a statement object
            Statement stmt = conn.createStatement();
        ) {
            System.out.println("Attempting to load JDBC driver...");
            // Optional: Explicitly load the driver (mostly for older JDBC versions, but safe to include)
            Class.forName(JDBC_DRIVER);
            System.out.println("Driver loaded successfully.");


            System.out.println("Executing query: SELECT 1...");
            String sql = "SELECT 1";

            // 3. Execute the query and get the ResultSet
            try (ResultSet rs = stmt.executeQuery(sql)) {
                // 4. Process the result set
                if (rs.next()) {
                    int result = rs.getInt(1); // Get the value of the first column
                    System.out.println("\nQuery Result:");
                    System.out.println("------------------------------------");
                    System.out.println("Value from SELECT 1 query: " + result);
                    System.out.println("------------------------------------");
                }
            } // ResultSet 'rs' is closed here automatically

        } catch (ClassNotFoundException e) {
            // Handles cases where the MySQL JDBC driver (Connector/J) is not found
            System.err.println("ERROR: MySQL JDBC Driver not found. Ensure the JAR file is included in your classpath.");
            e.printStackTrace();
        } catch (SQLException e) {
            // Handles connection errors, SQL syntax errors, authentication errors, etc.
            System.err.println("ERROR: Could not connect to or query the database.");
            System.err.println("Check if MySQL is running, the URL is correct, and credentials are valid.");
            e.printStackTrace();
        } finally {
            // Note: Resources are automatically closed by try-with-resources,
            // so we only print a final message here for confirmation.
            System.out.println("\nDatabase connection and resources were successfully closed.");
        }
    }
}

/**
 * Note on Security: I have included the credentials you provided (dbuser and SecretPass123!) as requested. 
 * However, in any real company application, these credentials should never be hardcoded directly into the source file.
 *  They should be loaded securely from environment variables, a vault, or a dedicated configuration file.
 */