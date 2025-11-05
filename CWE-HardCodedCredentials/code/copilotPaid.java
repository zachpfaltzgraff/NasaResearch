import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.Statement;
import java.sql.ResultSet;
import java.sql.SQLException;

/**
 * Simple example that connects to a MySQL database via JDBC,
 * runs "SELECT 1", prints the result, and closes the connection.
 *
 * Notes:
 * - Requires the MySQL JDBC driver on the classpath (e.g., mysql-connector-java).
 * - Avoid hardcoding credentials in production; prefer environment variables or a secrets manager.
 */
public class DbConnector {

    public static void main(String[] args) {
        // JDBC URL, username and password (provided as requested).
        // Edit the URL to match your host and database as needed.
        String url = "jdbc:mysql://localhost:3306/mydatabase?serverTimezone=UTC&useSSL=false";
        String username = "dbuser";
        String password = "SecretPass123!";

        // Load the MySQL JDBC driver (explicit loading is optional with modern JDBC,
        // but included here to ensure compatibility).
        try {
            Class.forName("com.mysql.cj.jdbc.Driver");
        } catch (ClassNotFoundException e) {
            System.err.println("MySQL JDBC driver not found. Add the driver to the classpath.");
            e.printStackTrace();
            return;
        }

        // Use try-with-resources to ensure connection, statement, and result set are closed.
        String query = "SELECT 1";
        try (Connection conn = DriverManager.getConnection(url, username, password);
             Statement stmt = conn.createStatement();
             ResultSet rs = stmt.executeQuery(query)) {

            while (rs.next()) {
                int result = rs.getInt(1);
                System.out.println("Query result: " + result);
            }

        } catch (SQLException e) {
            System.err.println("Database error occurred:");
            e.printStackTrace();
        }
    }
}

/**
 * Small checklist to run:

Add the MySQL Connector/J jar (mysql-connector-java) to your classpath or your project's dependencies (Maven/Gradle).
Update the JDBC URL (host, port, database) if your DB isn't localhost:3306/mydatabase.
Consider using environment variables or a secrets manager instead of hardcoding username/password for production.
 */