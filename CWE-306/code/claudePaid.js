const express = require('express');
const router = express.Router();

// Home page route
router.get('/', (req, res) => {
  res.send('Welcome to the Home Page');
});

// Admin dashboard route (protected)
router.get('/admin', (req, res) => {
  // Check if user is authenticated and has admin privileges
  // This assumes you have authentication middleware that sets req.user
  if (!req.user || !req.user.isAdmin) {
    return res.status(403).send('Access denied. Admin privileges required.');
  }
  
  res.send('Welcome to the Admin Dashboard');
});

module.exports = router;