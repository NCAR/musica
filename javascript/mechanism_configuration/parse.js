// mechanism_configuration/parse.js
//
// Parse mechanism configurations (v0 or v1) into plain JavaScript objects in
// the v1 wire shape, using the WASM parser. `initModule()` must have been
// awaited before calling these.

import { getBackend } from '../backend.js';

/**
 * @typedef {object} ParseError
 * @property {string} code - the error code, for example `UnknownType`
 * @property {string} message - the message, with the file, line, and column when known
 * @property {string|null} path - a JSON Pointer to the item that causes the error, or null
 */

/**
 * @typedef {object} ParseResult
 * @property {object|null} mechanism - as much of the mechanism as the parser can build, in the
 *   v1 wire shape. It is null only when the parser cannot read the configuration, for example
 *   when the version is missing or not supported. An item that cannot be built is left out,
 *   with an `ItemLeftOut` error.
 * @property {ParseError[]} errors - all the errors. The mechanism is valid only when this is empty.
 */

/**
 * Parse a v1 mechanism-configuration string (JSON or YAML).
 * v0 is not supported from a string because it is inherently multi-file - use
 * {@link parseMechanismFromFiles} for v0.
 * @param {string} config - a v1 JSON or YAML configuration string
 * @returns {ParseResult} the mechanism and all the errors
 */
export function parseMechanismFromString(config) {
  return getBackend().parseMechanismString(config);
}

/**
 * Parse a mechanism from a set of files (for example, an unzipped v0 CAMP
 * configuration, or a v1.1 configuration with file lists). The files are written
 * into an in-memory filesystem and the given entry point is parsed. The error
 * messages give the file paths relative to the file set.
 * @param {Record<string, string>} files - map of relative path to file contents
 * @param {string} entryPoint - path, relative to the file set, to parse. A
 *   directory is read as v0; a single file dispatches on its `version` field.
 * @returns {ParseResult} the mechanism and all the errors
 */
export function parseMechanismFromFiles(files, entryPoint) {
  return getBackend().parseMechanismFiles(files, entryPoint);
}
