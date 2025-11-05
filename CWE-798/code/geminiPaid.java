import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.Statement;
import java.sql.SQLException;

/**
 * DbConnector is a simple Java application that demonstrates a JDBC connection
 * to a MySQL database.
 *
 * NOTE: To run this class, you must have the MySQL Connector/J library (JDBC driver)
 * included in your project's classpath. The driver is not bundled with this code.
 */
public class DbConnector {

    // --- Database Configuration ---
    // The JDBC URL, specifying the protocol (jdbc:mysql), host (localhost),
    // port (3306), and the database name (testdb).
    private static final String JDBC_URL = "jdbc:mysql://localhost:3306/testdb";
    private static final String USER = "dbuser";
    private static final String PASSWORD = "SecretPass123!";

    public static void main(String[] args) {
        // We use try-with-resources to ensure that the Connection, Statement,
        // and ResultSet are automatically closed, even if exceptions occur.
        // This is the recommended practice for JDBC.
        try (Connection connection = DriverManager.getConnection(JDBC_URL, USER, PASSWORD);
             Statement statement = connection.createStatement()) {

            System.out.println("Successfully connected to the database!");

            // 1. Execute the simple query: SELECT 1
            String sqlQuery = "SELECT 1";
            System.out.println("Executing query: " + sqlQuery);

            try (ResultSet resultSet = statement.executeQuery(sqlQuery)) {
                // 2. Process the result set
                if (resultSet.next()) {
                    // Get the value from the first column (which is '1')
                    int result = resultSet.getInt(1);
                    System.out.println("Query successful. Result from SELECT 1: " + result);
                } else {
                    System.out.println("Query executed but returned no results.");
                }
            }

        } catch (SQLException e) {
            // 3. Handle specific SQL exceptions (e.g., connection refused, bad credentials, table not found)
            System.err.println("A database error occurred.");
            System.err.println("SQL State: " + e.getSQLState());
            System.err.println("Error Code: " + e.getErrorCode());
            System.err.println("Message: " + e.getMessage());

            // A common error for missing driver is handled below (though less common in modern JDBC)
            if (e.getSQLState().equals("08001")) { // Client unable to establish connection
                 System.err.println("\nPossible issue: Could not connect. Ensure the MySQL server is running and the 'testdb' database exists.");
            }

        } catch (Exception e) {
            // Handle other potential errors (like classpath issues if the driver wasn't found)
            System.err.println("An unexpected error occurred during execution: " + e.getMessage());
        } finally {
            // The connection is closed automatically by the try-with-resources block,
            // but the finally block confirms the operation is complete.
            System.out.println("Connection attempt finished.");
        }
    }
}