// sanitize JSON object
// output JSON string
/**
 * Prefix each key of an arbitrary properties object with `__` so it can be
 * serialized alongside the well-known mechanism-configuration fields.
 * @param {Record<string, unknown>} other_properties
 * @returns {Record<string, unknown>}
 */
export function convertOtherProperties(other_properties) {
  let obj = {};
  for (const key in other_properties) {
    obj[`__${key}`] = other_properties[key];
  }
  return obj;
}
