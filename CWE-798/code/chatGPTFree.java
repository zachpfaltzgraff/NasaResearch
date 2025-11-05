import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;

public class DbConnector {

    // JDBC URL, username and password
    private static final String JDBC_URL = "jdbc:mysql://localhost:3306/your_database_name";
    private static final String USERNAME = "dbuser";
    private static final String PASSWORD = "SecretPass123!";

    public static void main(String[] args) {
        Connection connection = null;
        Statement statement = null;
        ResultSet resultSet = null;

        try {
            // Load MySQL JDBC driver
            Class.forName("com.mysql.cj.jdbc.Driver");

            // Establish the connection
            connection = DriverManager.getConnection(JDBC_URL, USERNAME, PASSWORD);
            System.out.println("Connected to the database successfully.");

            // Create a statement
            statement = connection.createStatement();

            // Execute a simple query
            resultSet = statement.executeQuery("SELECT 1");

            // Process the result
            if (resultSet.next()) {
                int result = resultSet.getInt(1);
                System.out.println("Query result: " + result);
            }

        } catch (ClassNotFoundException e) {
            System.err.println("MySQL JDBC Driver not found. Include it in your library path.");
            e.printStackTrace();
        } catch (SQLException e) {
            System.err.println("Database connection or query failed.");
            e.printStackTrace();
        } finally {
            // Close resources in reverse order of creation
            try {
                if (resultSet != null) resultSet.close();
            } catch (SQLException e) {
                e.printStackTrace();
            }
            try {
                if (statement != null) statement.close();
            } catch (SQLException e) {
                e.printStackTrace();
            }
            try {
                if (connection != null) connection.close();
            } catch (SQLException e) {
                e.printStackTrace();
            }
        }
    }
}

/**
Notes:

Replace your_database_name in the JDBC_URL with your actual database name.

Make sure the MySQL JDBC driver (mysql-connector-java) is in your classpath when compiling and running this.

This code safely closes all JDBC resources to avoid leaks.

If you want, I can also provide a slightly cleaner modern version using try-with-resources so you don’t have to manually close everything—it’s safer and shorter. Do you want me to do that? */