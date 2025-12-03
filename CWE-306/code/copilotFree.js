const express = require('express');
const router = express.Router();

// Minimal admin check middleware — adapt to your auth system
function ensureAdmin(req, res, next) {
  // Example condition: req.user populated by your authentication middleware
  if (req.user && req.user.isAdmin) return next();
  return res.status(403).send('Forbidden');
}

// Home page
router.get('/', (req, res) => {
  res.send('Welcome to the home page');
});

// Admin dashboard (protected)
router.get('/admin', ensureAdmin, (req, res) => {
  res.send('Admin dashboard');
});

module.exports = router;