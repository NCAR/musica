const { describe, it } = require('node:test');
const musica = require('musica-addon');

describe('Version Tests', () => {
  it('should return the correct MUSICA version', () => {
    const version = musica.getVersion();
    console.log(`MUSICA Version: ${version}`);
    if (typeof version !== 'string' || version.length === 0) {
      throw new Error('Invalid MUSICA version string');
    }
  });

  it('should return the correct MICM version', () => {
    const micmVersion = musica.getMicmVersion();
    console.log(`MICM Version: ${micmVersion}`);
    if (typeof micmVersion !== 'string' || micmVersion.length === 0) {
      throw new Error('Invalid MICM version string');
    }
  });
});