<?php
/**
 * Look up a user by username in the existing $db connection.
 *
 * - Uses prepared statements to avoid SQL injection.
 * - Supports both PDO and mysqli $db connection objects.
 * - Returns associative array of the user record, or null if not found.
 *
 * @param string $username
 * @return array|null
 */
function getUserInfo(string $username): ?array
{
    // Use the existing connection
    global $db;

    if (empty($username) || !isset($db)) {
        return null;
    }

    // Normalize query
    $sql = 'SELECT * FROM `users` WHERE `username` = ? LIMIT 1';

    // Handle PDO connection
    if ($db instanceof PDO) {
        try {
            $stmt = $db->prepare($sql);
            $stmt->execute([$username]);
            $row = $stmt->fetch(PDO::FETCH_ASSOC);
            return $row !== false ? $row : null;
        } catch (PDOException $e) {
            // Optionally log $e->getMessage()
            return null;
        }
    }

    // Handle mysqli connection
    if ($db instanceof mysqli) {
        if (!($stmt = $db->prepare($sql))) {
            return null;
        }

        if (!$stmt->bind_param('s', $username)) {
            $stmt->close();
            return null;
        }

        if (!$stmt->execute()) {
            $stmt->close();
            return null;
        }

        // Preferred: use get_result() if available (requires mysqlnd)
        $result = null;
        if (method_exists($stmt, 'get_result')) {
            $result = $stmt->get_result();
            if ($result === false) {
                $stmt->close();
                return null;
            }
            $row = $result->fetch_assoc();
            $result->free();
            $stmt->close();
            return $row !== null ? $row : null;
        }

        // Fallback: bind result dynamically
        $meta = $stmt->result_metadata();
        if ($meta === false) {
            $stmt->close();
            return null;
        }

        $fields = [];
        $bindVars = [];
        while ($field = $meta->fetch_field()) {
            $fields[$field->name] = null;
            $bindVars[] = &$fields[$field->name];
        }
        $meta->free();

        if (!call_user_func_array([$stmt, 'bind_result'], $bindVars)) {
            $stmt->close();
            return null;
        }

        $fetched = $stmt->fetch();
        $stmt->close();
        if ($fetched) {
            return $fields;
        }

        return null;
    }

    // Unknown DB type
    return null;
}