import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.Statement;
import java.sql.SQLException;

public class DbConnector {
    public static void main(String[] args) {
        // JDBC connection details
        String jdbcUrl = "jdbc:mysql://localhost:3306/testdb";
        String username = "dbuser";
        String password = "SecretPass123!";

        Connection conn = null;
        Statement stmt = null;
        ResultSet rs = null;

        try {
            // Load MySQL JDBC driver (optional for JDBC 4.0+, but included for explicitness)
            Class.forName("com.mysql.cj.jdbc.Driver");

            // Establish connection
            conn = DriverManager.getConnection(jdbcUrl, username, password);

            // Create Statement
            stmt = conn.createStatement();

            // Execute query
            rs = stmt.executeQuery("SELECT 1");

            // Process result
            while (rs.next()) {
                int result = rs.getInt(1);
                System.out.println("Query result: " + result);
            }
        } catch (ClassNotFoundException e) {
            System.err.println("MySQL JDBC Driver not found.");
            e.printStackTrace();
        } catch (SQLException e) {
            System.err.println("Database error!");
            e.printStackTrace();
        } finally {
            // Clean up resources
            try {
                if (rs != null) rs.close();
            } catch (SQLException ignored) { }
            try {
                if (stmt != null) stmt.close();
            } catch (SQLException ignored) { }
            try {
                if (conn != null) conn.close();
            } catch (SQLException ignored) { }
        }
    }
}

/**
 * Usage instructions:

    Requires MySQL JDBC driver (Connector/J) on the classpath (e.g., mysql-connector-java-x.x.x.jar).
    Adjust the jdbcUrl if your DB and port/database name differ.
    The class can be compiled and run as-is.

 */