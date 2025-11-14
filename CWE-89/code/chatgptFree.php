function getUserInfo($username) {
    global $db; // Use the existing database connection

    // Prepare the SQL statement
    $stmt = $db->prepare("SELECT * FROM users WHERE username = :username LIMIT 1");
    if (!$stmt) {
        // Handle error if prepare fails
        return null;
    }

    // Bind the parameter
    $stmt->bindParam(':username', $username, PDO::PARAM_STR);

    // Execute the statement
    $stmt->execute();

    // Fetch the user record as an associative array
    $user = $stmt->fetch(PDO::FETCH_ASSOC);

    // Return user or null if not found
    return $user ? $user : null;
}
