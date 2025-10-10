#!/usr/bin/env node

const fs = require('fs');
const path = require('path');

// Extract version from CMakeLists.txt
function extractVersionFromCMake() {
    const projectRoot = path.resolve(__dirname, '..');
    const cmakeFile = path.join(projectRoot, 'CMakeLists.txt');

    try {
        const content = fs.readFileSync(cmakeFile, 'utf8');

        // Match: project(musica-distribution VERSION)
        const match = content.match(/project\s*\(\s*musica-distribution\s+VERSION\s+([\d.]+)\s*\)/);

        if (!match) {
            throw new Error('Could not find version in CMakeLists.txt. Expected format: project(musica-distribution VERSION x.y.z)');
        }

        return match[1];
    } catch (error) {
        throw new Error(`Could not read CMakeLists.txt: ${error.message}`);
    }
}

// Alias for extractVersionFromCMake
function readVersion() {
    return extractVersionFromCMake();
}

// Update package.json version to match CMakeLists.txt
function updatePackageJson() {
    const projectRoot = path.resolve(__dirname, '..');
    const packageFile = path.join(projectRoot, 'package.json');
    const version = extractVersionFromCMake();

    try {
        const packageJson = JSON.parse(fs.readFileSync(packageFile, 'utf8'));

        if (packageJson.version === version) {
            console.log(`package.json is already at version ${version}`);
            return version;
        }

        const oldVersion = packageJson.version;
        packageJson.version = version;

        fs.writeFileSync(packageFile, JSON.stringify(packageJson, null, 2) + '\n');
        console.log(`Updated package.json version: ${oldVersion} -> ${version}`);
        return version;
    } catch (error) {
        throw new Error(`Could not update package.json: ${error.message}`);
    }
}

// Update package-lock version to match package.json
function updatePackageLock() {
    const projectRoot = path.resolve(__dirname, '..');
    const packageLockFile = path.join(projectRoot, 'package-lock.json');
    //check if package-lock.json exists
    if (!fs.existsSync(packageLockFile)){
        console.log('package-lock.json not found, skipping..');
        return;
    }
    const version = extractVersionFromCMake();

    try {
        const packageLock = JSON.parse(fs.readFileSync(packageLockFile, 'utf8'));
        if (packageLock.version === version) {
            console.log(`package-lock is already at version ${version}`);
            return version;
        }
        const oldVersion = packageLock.version;
        packageLock.version = version;
        //update the root package entry if it exists
        if (packageLock.packages && packageLock.packages['']) {
            packageLock.packages[''].version = version;
        }
        fs.writeFileSync(packageLockFile, JSON.stringify(packageLock, null, 2) + '\n');
        console.log(`Upadted package-lock version: ${oldVersion} --> ${version}`);
        return version;
    } catch (error) {
        throw new Error(`Could not update package-lock.json: ${error.message}`);
    }
}

// Sync package.json and package-lock.json version with CMakeLists.txt
function syncVersion() {
    try {
        const version = extractVersionFromCMake();
        console.log(`CMakeLists.txt version: ${version}`);

        updatePackageJson();
        updatePackageLock();

        console.log('Version sync complete!');
        return version;
    } catch (error) {
        console.error('Error syncing version:', error.message);
        process.exit(1);
    }
}

// Alias for syncVersion 
function syncAllVersions() {
    return syncVersion();
}

// CLI interface
if (require.main === module) {
    const command = process.argv[2];

    switch (command) {
        case 'read':
        case 'extract':
            console.log(extractVersionFromCMake());
            break;
        case 'sync':
        case 'update':
        case 'update-package':
            syncVersion();
            break;
        default:
            console.log('Usage: node extract_version.js <command>');
            console.log('');
            console.log('Commands:');
            console.log('  read, extract      - Read version from CMakeLists.txt');
            console.log('  sync, update       - Update package.json and package-lock.json to match CMakeLists.txt');
            console.log('  update-package     - Alias for sync');
            console.log('');
            console.log('Examples:');
            console.log('  node scripts/extract_version.js read');
            console.log('  node scripts/extract_version.js sync');
            console.log('  npm run sync-version');
            break;
    }
}

module.exports = {
    extractVersionFromCMake,
    readVersion,
    updatePackageJson,
    updatePackageLock,
    syncVersion,
    syncAllVersions,
};