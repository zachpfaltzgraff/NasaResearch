const express = require('express');
const router = express.Router();

// Middleware to check if user is admin
function isAdmin(req, res, next) {
  // Replace this with your real admin check logic
  const userIsAdmin = req.user && req.user.isAdmin;
  if (userIsAdmin) {
    next();
  } else {
    res.status(403).send('Access denied');
  }
}

// Home page route
router.get('/', (req, res) => {
  res.send('Welcome to the home page!');
});

// Admin dashboard route (protected)
router.get('/admin', isAdmin, (req, res) => {
  res.send('Welcome to the admin dashboard!');
});

module.exports = router;
