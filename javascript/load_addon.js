const path = require('path');

module.exports = require('bindings')({
  bindings: 'musica-addon.node',
  module_root: path.join(__dirname, '..'),
  try: [
    ['module_root', 'build-addon', 'Release', 'musica-addon.node']
  ]
});
