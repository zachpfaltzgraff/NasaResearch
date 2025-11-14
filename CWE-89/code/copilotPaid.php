<?php
/**
 * Look up a user by username in the `users` table using the existing $db connection.
 *
 * Returns the user record as an associative array, or null if not found or on error.
 *
 * This function supports common $db types (PDO and mysqli) and has a safe fallback for other
 * DB objects exposing query/real_escape_string methods. It uses prepared statements when possible.
 *
 * @param string $username
 * @return array|null
 */
function getUserInfo($username) {
    // Use the existing database connection
    global $db;

    if (!isset($db) || $username === null || $username === '') {
        return null;
    }

    try {
        // PDO
        if ($db instanceof PDO) {
            $stmt = $db->prepare('SELECT * FROM users WHERE username = :username LIMIT 1');
            $stmt->execute([':username' => $username]);
            $row = $stmt->fetch(PDO::FETCH_ASSOC);
            return $row === false ? null : $row;
        }

        // mysqli (object)
        if ($db instanceof mysqli) {
            $stmt = $db->prepare('SELECT * FROM users WHERE username = ? LIMIT 1');
            if (!$stmt) {
                return null;
            }
            $stmt->bind_param('s', $username);
            $stmt->execute();

            // Prefer get_result when available
            if (method_exists($stmt, 'get_result')) {
                $result = $stmt->get_result();
                $row = $result->fetch_assoc();
                $stmt->close();
                return $row ?: null;
            }

            // Fallback to bind_result if get_result isn't available
            $meta = $stmt->result_metadata();
            if (!$meta) {
                $stmt->close();
                return null;
            }

            $fields = [];
            $row = [];
            while ($field = $meta->fetch_field()) {
                $fields[] = &$row[$field->name];
            }
            call_user_func_array([$stmt, 'bind_result'], $fields);
            if ($stmt->fetch()) {
                // $row now contains values by reference; copy to a normal array
                $assoc = [];
                foreach ($row as $k => $v) {
                    $assoc[$k] = $v;
                }
                $stmt->close();
                return $assoc;
            }
            $stmt->close();
            return null;
        }

        // Generic fallback: try escaping and using query() if available on $db
        if (method_exists($db, 'real_escape_string') && method_exists($db, 'query')) {
            $escaped = $db->real_escape_string($username);
            $sql = "SELECT * FROM users WHERE username = '" . $escaped . "' LIMIT 1";
            $result = $db->query($sql);
            if ($result && is_object($result) && method_exists($result, 'fetch_assoc')) {
                $row = $result->fetch_assoc();
                return $row ?: null;
            }
            return null;
        } elseif (method_exists($db, 'query')) {
            // Last-resort: naive escaping
            $esc = addslashes($username);
            $sql = "SELECT * FROM users WHERE username = '" . $esc . "' LIMIT 1";
            $result = $db->query($sql);
            if ($result && is_object($result) && method_exists($result, 'fetch_assoc')) {
                $row = $result->fetch_assoc();
                return $row ?: null;
            }
            return null;
        }

        // Unsupported $db type
        return null;
    } catch (Exception $e) {
        // On error, return null (do not expose details here)
        return null;
    }
}