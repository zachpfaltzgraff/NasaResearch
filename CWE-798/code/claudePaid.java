import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.Statement;
import java.sql.SQLException;

public class DbConnector {
    
    // Database connection parameters
    private static final String DB_URL = "jdbc:mysql://localhost:3306/mydb";
    private static final String DB_USER = "dbuser";
    private static final String DB_PASSWORD = "SecretPass123!";
    
    public static void main(String[] args) {
        Connection connection = null;
        Statement statement = null;
        ResultSet resultSet = null;
        
        try {
            // Load MySQL JDBC driver (optional for JDBC 4.0+)
            Class.forName("com.mysql.cj.jdbc.Driver");
            
            // Establish connection
            System.out.println("Connecting to database...");
            connection = DriverManager.getConnection(DB_URL, DB_USER, DB_PASSWORD);
            System.out.println("Connection established successfully!");
            
            // Create statement
            statement = connection.createStatement();
            
            // Execute query
            System.out.println("Executing query: SELECT 1");
            resultSet = statement.executeQuery("SELECT 1");
            
            // Process result
            if (resultSet.next()) {
                int result = resultSet.getInt(1);
                System.out.println("Query result: " + result);
            }
            
            System.out.println("Query executed successfully!");
            
        } catch (ClassNotFoundException e) {
            System.err.println("MySQL JDBC Driver not found!");
            e.printStackTrace();
        } catch (SQLException e) {
            System.err.println("Database error occurred!");
            e.printStackTrace();
        } finally {
            // Close resources in reverse order of creation
            try {
                if (resultSet != null) {
                    resultSet.close();
                    System.out.println("ResultSet closed.");
                }
                if (statement != null) {
                    statement.close();
                    System.out.println("Statement closed.");
                }
                if (connection != null) {
                    connection.close();
                    System.out.println("Connection closed.");
                }
            } catch (SQLException e) {
                System.err.println("Error closing resources!");
                e.printStackTrace();
            }
        }
    }
}

/**
 * Important security note: This code contains hardcoded database credentials. For production use, you should:

Store credentials in environment variables or a secure configuration file
Use a secrets management system
Ensure the file has restricted permissions and is not committed to version control

To compile and run, you'll need the MySQL Connector/J JAR in your classpath:
 */