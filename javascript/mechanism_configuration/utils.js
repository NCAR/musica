// sanitize JSON object
// output JSON string
export function convertOtherProperties(other_properties) {
  let obj = {};
	for (const key in other_properties) {
		obj[`__${key}`] = other_properties[key];
	}
	return obj;
}

