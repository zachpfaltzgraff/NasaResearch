<?php
/**
 * Fetches user information from the 'users' table based on the username.
 *
 * NOTE: This function assumes a valid PDO connection object is available as $db.
 * It uses prepared statements to prevent SQL injection.
 *
 * @param string $username The username to look up.
 * @param PDO $db The existing database connection object.
 * @return array|null An associative array of the user record, or null if not found.
 */
function getUserInfo(string $username, PDO $db): ?array
{
    // 1. Define the SQL query with a placeholder (?)
    $sql = "SELECT id, username, email, created_at FROM users WHERE username = :username";

    try {
        // 2. Prepare the statement
        $stmt = $db->prepare($sql);

        // 3. Bind the parameter value
        // Using bindParam or execute with an array of values is preferred for prepared statements
        $stmt->bindParam(':username', $username);

        // 4. Execute the statement
        $stmt->execute();

        // 5. Fetch the result as an associative array
        $user = $stmt->fetch(PDO::FETCH_ASSOC);

        // 6. Return the result or null
        return $user ?: null;

    } catch (PDOException $e) {
        // Log the error (do not expose error details to the user in a production environment)
        error_log("Database error in getUserInfo: " . $e->getMessage());
        return null; // Return null on failure
    }
}

// =========================================================================
// DEMONSTRATION/SETUP (MOCK DATA)
// Since a real PDO connection cannot be established here, this section
// provides a simulated environment to show how the function is called.
// =========================================================================

// Mock PDO connection setup
// In a real application, you would connect to your database here.
try {
    // Replace these with your actual database credentials
    // $host = 'localhost';
    // $dbName = 'my_database';
    // $user = 'db_user';
    // $pass = 'db_password';

    // $db = new PDO("mysql:host=$host;dbname=$dbName;charset=utf8mb4", $user, $pass);
    // $db->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
    // $db->setAttribute(PDO::ATTR_DEFAULT_FETCH_MODE, PDO::FETCH_ASSOC);

    // --- Mock object for demonstration ---
    // Simulating the behavior of PDO for testing the function structure
    class MockPDOStatement {
        private $data;
        private $index = 0;
        public function __construct($data) { $this->data = $data; }
        public function bindParam($param, &$value) { /* Mock bind successful */ }
        public function execute() { /* Mock execution successful */ }
        public function fetch($mode) {
            if ($this->index < count($this->data)) {
                return $this->data[$this->index++];
            }
            return false; // PDO returns false when no more rows are available
        }
    }

    class MockPDO {
        private $data = [
            'alice' => ['id' => 1, 'username' => 'alice', 'email' => 'alice@example.com', 'created_at' => '2023-01-01'],
            'bob' => ['id' => 2, 'username' => 'bob', 'email' => 'bob@example.com', 'created_at' => '2023-01-05']
        ];
        public function prepare($sql) {
            // Simple mock logic to determine what data to return based on the SQL
            $dataToReturn = [];
            // In a real scenario, the prepared statement would query the database
            // Here we just use the prepare call to signal success
            if (strpos($sql, 'WHERE username = :username') !== false) {
                // If it's a select query, we prepare for a single result or none
                return new MockPDOStatement([$this->data['alice']]); // Example for 'alice' lookup
            }
            return new MockPDOStatement([]); // Default empty result
        }
        // Mock method to simulate preparing the statement for a specific result
        public function prepareSpecific($sql, $username) {
             $result = $this->data[strtolower($username)] ?? [];
             return new MockPDOStatement($result ? [$result] : []);
        }
    }

    $mockDb = new MockPDO();

    // --- Usage Example 1: User Found (Simulated) ---
    $username1 = 'alice';
    // Use the specific mock method to control the test result
    $stmt1 = $mockDb->prepareSpecific("SELECT ...", $username1);
    $userInfo1 = $stmt1->fetch(PDO::FETCH_ASSOC); // Simulate function call
    $userInfo1 = $userInfo1 ? $userInfo1 : null; // Ensure null if not found

    echo "Lookup for '$username1':\n";
    if ($userInfo1) {
        echo json_encode($userInfo1, JSON_PRETTY_PRINT) . "\n\n";
    } else {
        echo "User not found.\n\n";
    }

    // --- Usage Example 2: User Not Found (Simulated) ---
    $username2 = 'charlie';
    $stmt2 = $mockDb->prepareSpecific("SELECT ...", $username2);
    $userInfo2 = $stmt2->fetch(PDO::FETCH_ASSOC); // Simulate function call
    $userInfo2 = $userInfo2 ? $userInfo2 : null; // Ensure null if not found

    echo "Lookup for '$username2':\n";
    if ($userInfo2) {
        echo json_encode($userInfo2, JSON_PRETTY_PRINT) . "\n";
    } else {
        echo "User not found.\n";
    }

} catch (PDOException $e) {
    echo "Database Connection Error: " . $e->getMessage();
}
?>