const express = require('express');
const router = express.Router();

// --- Middleware for Admin Protection ---
// NOTE: In a real application, you would implement proper
// authentication and authorization logic here (e.g., checking for
// a valid session, user role, or JWT). This is a simple placeholder.
function isAdmin(req, res, next) {
    // For demonstration, let's assume the user must pass a specific query parameter
    // to be considered an admin. Replace this with your actual logic.
    if (req.query.user === 'admin') {
        next(); // User is admin, proceed to the route handler
    } else {
        res.status(403).send('Access Denied: You must be an admin.');
    }
}

// --- Public Routes ---

/**
 * @route GET /
 * @desc Home page route
 */
router.get('/', (req, res) => {
    res.send('Welcome to the Home Page!');
});


// --- Protected Routes ---

/**
 * @route GET /admin
 * @desc Admin Dashboard route
 * @access Private (Requires 'admin' privileges)
 */
router.get('/admin', isAdmin, (req, res) => {
    res.send('Welcome to the **Admin Dashboard**! (Access granted)');
});


module.exports = router;