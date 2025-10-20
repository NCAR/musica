function isScalarNumber(x) {
	return (
		(typeof x === 'number' || x instanceof Number) &&
		!Number.isNaN(x) &&
		!(typeof x === 'boolean')
	);
}
module.exports = { isScalarNumber };
