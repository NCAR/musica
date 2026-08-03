// mechanism_configuration/parse.js
//
// Parse mechanism configurations (v0 or v1) into plain JavaScript objects in
// the v1 wire shape, using the WASM parser. `initModule()` must have been
// awaited before calling these.

import { getBackend } from '../backend.js';

/**
 * Parse a v1 mechanism-configuration string (JSON or YAML) into a plain object.
 * v0 is not supported from a string because it is inherently multi-file — use
 * {@link parseMechanismFromFiles} for v0.
 * @param {string} config - a v1 JSON or YAML configuration string
 * @returns {object} the parsed mechanism in the v1 wire shape
 */
export function parseMechanismFromString(config) {
  return getBackend().parseMechanismString(config);
}

/**
 * Parse a mechanism from a set of files (for example, an unzipped v0 CAMP
 * configuration). The files are written into an in-memory filesystem and the
 * given entry point is parsed.
 * @param {Record<string, string>} files - map of relative path to file contents
 * @param {string} entryPoint - path, relative to the file set, to parse. A
 *   directory is read as v0; a single file dispatches on its `version` field.
 * @returns {object} the parsed mechanism in the v1 wire shape
 */
export function parseMechanismFromFiles(files, entryPoint) {
  return getBackend().parseMechanismFiles(files, entryPoint);
}
