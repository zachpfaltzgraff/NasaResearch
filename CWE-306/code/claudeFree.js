const express = require('express');
const router = express.Router();

// Middleware to check if user is admin
const isAdmin = (req, res, next) => {
  if (req.session && req.session.isAdmin) {
    next();
  } else {
    res.status(403).send('Access denied');
  }
};

// Home page route
router.get('/', (req, res) => {
  res.send('Welcome to the home page');
});

// Admin dashboard route (protected)
router.get('/admin', isAdmin, (req, res) => {
  res.send('Admin Dashboard');
});

module.exports = router;