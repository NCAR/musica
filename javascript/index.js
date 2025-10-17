const addon = require('../build/Release/musica-addon.node');
const types = require('./types.js');

module.exports = { ...addon, types };
