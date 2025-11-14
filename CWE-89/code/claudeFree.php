function getUserInfo($username) {
    global $db;
    
    // Prepare statement to prevent SQL injection
    $stmt = $db->prepare("SELECT * FROM users WHERE username = ?");
    $stmt->bind_param("s", $username);
    $stmt->execute();
    
    $result = $stmt->get_result();
    
    // Fetch user record as associative array
    $user = $result->fetch_assoc();
    
    $stmt->close();
    
    // Returns associative array if found, null if not
    return $user;
}