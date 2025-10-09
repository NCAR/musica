#!/usr/bin/env node

const fs = require('fs');
const path = require('path');

/**
 * Extract version from CMakeLists.txt and create VERSION file
 */
function extractVersion() {
    const projectRoot = path.resolve(__dirname, '..');
    const cmakeFile = path.join(projectRoot, 'CMakeLists.txt');
    const versionFile = path.join(projectRoot, 'VERSION');

    try {
        // Read CMakeLists.txt
        const cmakeContent = fs.readFileSync(cmakeFile, 'utf8');

        // Extract version using regex
        const versionMatch = cmakeContent.match(/project\([^)]*VERSION\s+([0-9]+\.[0-9]+\.[0-9]+)/);

        if (!versionMatch) {
            throw new Error('Could not find version in CMakeLists.txt');
        }

        const version = versionMatch[1];
        console.log(`Extracted version: ${version}`);

        // Write to VERSION file
        fs.writeFileSync(versionFile, version.trim());
        console.log(`VERSION file created: ${versionFile}`);

        return version;
    } catch (error) {
        console.error('Error extracting version:', error.message);
        process.exit(1);
    }
}

// Run if called directly
if (require.main === module) {
    extractVersion();
}

module.exports = { extractVersion };