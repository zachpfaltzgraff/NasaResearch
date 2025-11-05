import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;

public class DbConnector {
    
    // Database connection parameters
    private static final String JDBC_URL = "jdbc:mysql://localhost:3306/myDatabase";
    private static final String USERNAME = "dbuser";
    private static final String PASSWORD = "SecretPass123!";
    
    public static void main(String[] args) {
        Connection connection = null;
        Statement statement = null;
        ResultSet resultSet = null;
        
        try {
            // Load MySQL JDBC Driver
            Class.forName("com.mysql.cj.jdbc.Driver");
            
            // Establish connection
            System.out.println("Connecting to database...");
            connection = DriverManager.getConnection(JDBC_URL, USERNAME, PASSWORD);
            System.out.println("Connection successful!");
            
            // Create statement
            statement = connection.createStatement();
            
            // Execute query
            System.out.println("Executing query: SELECT 1");
            resultSet = statement.executeQuery("SELECT 1");
            
            // Print result
            if (resultSet.next()) {
                int result = resultSet.getInt(1);
                System.out.println("Query result: " + result);
            }
            
        } catch (ClassNotFoundException e) {
            System.err.println("MySQL JDBC Driver not found!");
            e.printStackTrace();
        } catch (SQLException e) {
            System.err.println("Database connection or query failed!");
            e.printStackTrace();
        } finally {
            // Close resources in reverse order
            try {
                if (resultSet != null) resultSet.close();
                if (statement != null) statement.close();
                if (connection != null) {
                    connection.close();
                    System.out.println("Connection closed.");
                }
            } catch (SQLException e) {
                e.printStackTrace();
            }
        }
    }
}

/**
 * Security Warning: Hardcoding database credentials in source code is a significant security risk and is considered a bad practice because:

Credentials are visible to anyone with access to the code
They may be accidentally committed to version control
They're difficult to change without recompiling

For production use, please use environment variables, configuration files (not in version control), or a secrets management system.

Update the JDBC_URL to match your database server (currently set to localhost:3306/myDatabase)
Compile: javac -cp .:mysql-connector-j-8.0.33.jar DbConnector.java
Run: java -cp .:mysql-connector-j-8.0.33.jar DbConnector

For production environments, consider refactoring to use:

Environment variables: System.getenv("DB_PASSWORD")
Properties files outside the codebase
Connection pooling (HikariCP, Apache DBCP)
Try-with-resources for automatic resource management
 */