// linear interpolation for evolving conditions
// interpolates values between time points for temp, pressure, etc

function linearInterpolate(x, x0, x1, y0, y1) {
    // handle edge cases
    if (x1 === x0) return y0;

    // linear interp: y = y0 + (x - x0) * (y1 - y0) / (x1 - x0)
    const t = (x - x0) / (x1 - x0);
    return y0 + t * (y1 - y0);
}

// step interpolation - hold previous value until next time point
function stepInterpolate(currentTime, times, values) {
    // find last time point before or at current time
    for (let i = times.length - 1; i >= 0; i--) {
        if (currentTime >= times[i]) {
            return values[i];
        }
    }
    // before first point? use first value
    return values[0];
}

// get interpolated value from time-series data
function interpolateValue(currentTime, times, values, defaultValue, method = 'linear') {
    // validate inputs
    if (!times || !values || times.length === 0 || values.length === 0) {
        return defaultValue;
    }

    if (times.length !== values.length) {
        console.warn('Interpolation warning: times and values arrays have different lengths');
        return defaultValue;
    }

    // only one point? use as constant
    if (times.length === 1) {
        return values[0];
    }

    // before first time point
    if (currentTime <= times[0]) {
        return values[0];
    }

    // after last time point
    if (currentTime >= times[times.length - 1]) {
        return values[values.length - 1];
    }

    // use step if requested
    if (method === 'step') {
        return stepInterpolate(currentTime, times, values);
    }

    // linear interpolation (default)
    // find surrounding time points
    for (let i = 0; i < times.length - 1; i++) {
        if (currentTime >= times[i] && currentTime <= times[i + 1]) {
            return linearInterpolate(
                currentTime,
                times[i],
                times[i + 1],
                values[i],
                values[i + 1]
            );
        }
    }

    // fallback (shouldn't reach here)
    return defaultValue;
}

// validate evolving conditions data
function validateEvolvingConditions(evolvingConditions) {
    const errors = [];

    if (!evolvingConditions) {
        return { isValid: false, errors: ['Evolving conditions object is null or undefined'] };
    }

    const { times, temperature, pressure } = evolvingConditions;

    // check arrays exist
    if (!Array.isArray(times)) {
        errors.push('times must be an array');
    }
    if (!Array.isArray(temperature)) {
        errors.push('temperature must be an array');
    }
    if (!Array.isArray(pressure)) {
        errors.push('pressure must be an array');
    }

    if (errors.length > 0) {
        return { isValid: false, errors };
    }

    // check lengths match
    if (times.length !== temperature.length) {
        errors.push(`times length (${times.length}) does not match temperature length (${temperature.length})`);
    }
    if (times.length !== pressure.length) {
        errors.push(`times length (${times.length}) does not match pressure length (${pressure.length})`);
    }

    // need at least one point
    if (times.length === 0) {
        errors.push('Evolving conditions must have at least one time point');
    }

    // check times are sorted
    for (let i = 1; i < times.length; i++) {
        if (times[i] < times[i - 1]) {
            errors.push('Time points must be sorted in ascending order');
            break;
        }
    }

    // validate values are numbers
    times.forEach((t, i) => {
        if (typeof t !== 'number' || isNaN(t)) {
            errors.push(`Invalid time at index ${i}: ${t}`);
        }
    });
    temperature.forEach((t, i) => {
        if (typeof t !== 'number' || isNaN(t)) {
            errors.push(`Invalid temperature at index ${i}: ${t}`);
        }
    });
    pressure.forEach((p, i) => {
        if (typeof p !== 'number' || isNaN(p)) {
            errors.push(`Invalid pressure at index ${i}: ${p}`);
        }
    });

    return {
        isValid: errors.length === 0,
        errors
    };
}

export {
    linearInterpolate,
    interpolateValue,
    validateEvolvingConditions
};
