// routes.js
const express = require('express');
const router = express.Router();

// Example middleware to protect admin routes.
// Replace this with your real authentication/authorization logic.
function requireAdmin(req, res, next) {
  // Example condition: user must be logged in AND have admin flag
  if (req.user && req.user.isAdmin) {
    return next();
  }
  return res.status(403).send('Access denied');
}

// Home page route
router.get('/', (req, res) => {
  res.send('Welcome to the home page');
});

// Admin dashboard route (protected)
router.get('/admin', requireAdmin, (req, res) => {
  res.send('Welcome to the admin dashboard');
});

module.exports = router;
