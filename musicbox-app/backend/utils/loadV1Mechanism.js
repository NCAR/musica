// Utility to load v1 mechanism configs as JSON strings for C++ parser
// v1 configs are passed directly to C++ - no JavaScript parsing needed

import fs from 'fs';
import path from 'path';

/**
 * Load a v1 mechanism configuration as a JSON string
 * The C++ parser handles all reaction types (ARRHENIUS, PHOTOLYSIS, TROE, SURFACE, USER_DEFINED)
 *
 * @param {string} configPath - Path to v1 mechanism config directory
 * @returns {string} JSON string of the mechanism configuration
 */
function loadV1MechanismAsString(configPath) {
  // Read config.json (or try alternative names)
  let configFile = path.join(configPath, 'config.json');

  if (!fs.existsSync(configFile)) {
    // Try alternative file names
    const dirName = path.basename(configPath);
    const altFile = path.join(configPath, `${dirName}.json`);
    if (fs.existsSync(altFile)) {
      configFile = altFile;
    } else {
      throw new Error(`Config file not found in: ${configPath}`);
    }
  }

  // Return the raw JSON string for C++ parser
  return fs.readFileSync(configFile, 'utf8');
}

export { loadV1MechanismAsString };
