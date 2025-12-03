const express = require('express');
const router = express.Router();

// Minimal admin-checking middleware.
// Replace the condition below with your real auth/session logic.
function ensureAdmin(req, res, next) {
  // Example: req.user created by your auth middleware and has isAdmin flag
  if (req.user && req.user.isAdmin) return next();

  // Fallback: try session flag (if you use sessions)
  if (req.session && req.session.isAdmin) return next();

  res.status(403).send('Forbidden');
}

// Home page route
router.get('/', (req, res) => {
  res.send('Welcome to the home page');
});

// Admin dashboard route (protected)
router.get('/admin', ensureAdmin, (req, res) => {
  res.send('Admin dashboard');
});

module.exports = router;