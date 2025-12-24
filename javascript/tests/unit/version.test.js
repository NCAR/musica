const assert = require('node:assert');
const { describe, it, before } = require('node:test');
const musica = require('../../index.js');

before(async () => {
  await musica.initModule();
});

describe('Version Tests', () => {
  it('should return the MUSICA version', async (t) => {
    const version = await musica.getVersion();
    console.log(`MUSICA Version: ${version}`);
    assert.ok(typeof version === 'string' && version.length > 0, 'Invalid MUSICA version string');
  });

  it('should return the MICM version',  async (t) => {
    const micmVersion = await musica.getMicmVersion();
    console.log(`MICM Version: ${micmVersion}`);
    assert.ok(typeof micmVersion === 'string' && micmVersion.length > 0, 'Invalid MICM version string');
  });
});
