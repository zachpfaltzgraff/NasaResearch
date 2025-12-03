// routes.js

const express = require('express');
const router = express.Router();

// --- 1. Middleware for Access Control ---
/**
 * Placeholder middleware function to check if a user is an admin.
 * In a real application, this would check session, JWT, or database
 * for user roles and call next() or send an error response.
 */
function ensureAdmin(req, res, next) {
    // ⚠️ REPLACE THIS LOGIC with your actual authentication check
    const isAdmin = true; // For demonstration, let's assume they are NOT admin for the initial test.

    if (isAdmin) {
        // If the user has admin rights, proceed to the next handler/route
        next();
    } else {
        // If not an admin, send a 403 Forbidden error or redirect to a login page.
        res.status(403).send('Access Denied: You must be an administrator.');
    }
}

// --- 2. Define Routes ---

// A. The Home Route
router.get('/', (req, res) => {
    res.send('<h1>Welcome to the Home Page!</h1><p>This page is public.</p>');
});

// B. The Protected Admin Route
// It uses the ensureAdmin middleware *before* the route handler.
router.get('/admin', ensureAdmin, (req, res) => {
    res.send('<h2>🛡️ Admin Dashboard</h2><p>This content is only visible to authenticated administrators.</p>');
});

// --- 3. Export the Router ---
module.exports = router;