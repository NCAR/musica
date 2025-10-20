const fs = require('fs');
const path = require('path');

const dir = __dirname;
const skip = new Set(['musica.js', 'index.js']);

const merged = {};
for (const file of fs.readdirSync(dir)) {
	if (!file.endsWith('.js') || skip.has(file)) continue;
	const mod = require(path.join(dir, file));
	if (mod && typeof mod === 'object') Object.assign(merged, mod);
}
module.exports = merged;
