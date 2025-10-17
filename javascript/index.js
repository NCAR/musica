const addon = require('../build/Release/musica-addon.node');
const types = require('./types.js');

Object.assign(addon, { types });
module.exports = addon;
