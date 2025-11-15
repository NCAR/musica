// sanatize jason object
// output jsaon string
function convertOtherProperties(other_properties) {
    obj = {};
	for (const key in other_properties) {
		obj[`__${key}`] = other_properties[key];
	}
	return obj;
}

module.exports = { convertOtherProperties };
