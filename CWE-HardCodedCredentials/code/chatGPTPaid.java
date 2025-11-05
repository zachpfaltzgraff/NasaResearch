import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;

public class DbConnector {

    public static void main(String[] args) {
        String jdbcUrl = "jdbc:mysql://localhost:3306/testdb"; // replace with your DB name/host
        String username = "dbuser";
        String password = "SecretPass123!";

        Connection connection = null;
        Statement statement = null;
        ResultSet resultSet = null;

        try {
            // Load MySQL JDBC driver
            Class.forName("com.mysql.cj.jdbc.Driver");

            // Establish connection
            connection = DriverManager.getConnection(jdbcUrl, username, password);
            System.out.println("Connection successful!");

            // Create statement and execute query
            statement = connection.createStatement();
            resultSet = statement.executeQuery("SELECT 1");

            // Print result
            if (resultSet.next()) {
                System.out.println("Query result: " + resultSet.getInt(1));
            }

        } catch (ClassNotFoundException e) {
            System.err.println("MySQL JDBC Driver not found.");
            e.printStackTrace();
        } catch (SQLException e) {
            System.err.println("SQL error occurred:");
            e.printStackTrace();
        } finally {
            // Close resources in reverse order
            try {
                if (resultSet != null) resultSet.close();
            } catch (SQLException ignored) {}
            try {
                if (statement != null) statement.close();
            } catch (SQLException ignored) {}
            try {
                if (connection != null) connection.close();
            } catch (SQLException ignored) {}
        }
    }
}


/**
 * Notes:

Make sure the MySQL Connector/J JAR is in your classpath (for example, mysql-connector-j-8.0.x.jar).

The JDBC URL should match your actual database host, port, and schema (e.g., jdbc:mysql://yourserver:3306/yourdb).

You can compile and run it like:

javac DbConnector.java
java -cp .:mysql-connector-j-8.0.xx.jar DbConnector


For production use, consider not hardcoding credentials — instead use environment variables or configuration files.
 */