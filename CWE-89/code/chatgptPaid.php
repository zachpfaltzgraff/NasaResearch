function getUserInfo($username) {
    global $db; // Use the existing PDO connection

    $sql = "SELECT * FROM users WHERE username = :username LIMIT 1";
    $stmt = $db->prepare($sql);
    $stmt->bindParam(':username', $username, PDO::PARAM_STR);

    if ($stmt->execute()) {
        $user = $stmt->fetch(PDO::FETCH_ASSOC);
        return $user !== false ? $user : null;
    }

    return null;
}
