const fs = require('fs');
const path = require('path');

/* Read version from VERSION file */
function readVersion() {
    const projectRoot = path.resolve(__dirname, '..');
    const versionFile = path.join(projectRoot, 'VERSION');

    try {
        return fs.readFileSync(versionFile, 'utf8').trim();
    } catch (error) {
        throw new Error(`Could not read VERSION file: ${error.message}`);
    }
}

/* Update package.json version to match VERSION file */
function updatePackageJson() {
    const projectRoot = path.resolve(__dirname, '..');
    const packageFile = path.join(projectRoot, 'package.json');
    const version = readVersion();

    try {
        const packageJson = JSON.parse(fs.readFileSync(packageFile, 'utf8'));
        packageJson.version = version;

        fs.writeFileSync(packageFile, JSON.stringify(packageJson, null, 2) + '\n');
        console.log(`Updated package.json version to: ${version}`);
        return version;
    } catch (error) {
        throw new Error(`Could not update package.json: ${error.message}`);
    }
}



/* Sync all build system versions to match VERSION file */
function syncAllVersions() {
    try {
        const version = readVersion();
        console.log(`Syncing all build systems to version: ${version}`);

        updatePackageJson();

        console.log('All build systems synced successfully!');
        return version;
    } catch (error) {
        console.error('Error syncing versions:', error.message);
        process.exit(1);
    }
}

/* Test version consistency across all build systems */
function testVersionConsistency() {
    try {
        const version = readVersion();
        console.log(`Testing version consistency. Expected version: ${version}`);

        // Test package.json
        const projectRoot = path.resolve(__dirname, '..');
        const packageFile = path.join(projectRoot, 'package.json');
        if (fs.existsSync(packageFile)) {
            const packageJson = JSON.parse(fs.readFileSync(packageFile, 'utf8'));
            console.log(`package.json version: ${packageJson.version} ${packageJson.version === version ? '-> True' : '-> Fasle'}`);
        }

        // CMake and Python versions would need to be tested during their respective build processes
        console.log('Note: CMake and Python versions are validated during build time');

        return version;
    } catch (error) {
        console.error('Error testing version consistency:', error.message);
        process.exit(1);
    }
}

// CLI interface
if (require.main === module) {
    const command = process.argv[2];

    switch (command) {
        case 'read':
            console.log(readVersion());
            break;
        case 'sync':
            syncAllVersions();
            break;
        case 'test':
            testVersionConsistency();
            break;
        case 'update-package':
            updatePackageJson();
            break;
        default:
            console.log('Usage: node version_utils.js <command>');
            console.log('Commands:');
            console.log('  read              - Read current version');
            console.log('  sync              - Sync all build systems');
            console.log('  test              - Test version consistency');
            console.log('  update-package    - Update package.json only');
            break;
    }
}

module.exports = {
    readVersion,
    updatePackageJson,
    syncAllVersions,
    testVersionConsistency
};