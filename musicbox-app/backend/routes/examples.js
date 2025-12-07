// Examples API Routes
// Provides pre-configured example simulations

import express from 'express';
import path from 'path';
import fs from 'fs';
import { fileURLToPath } from 'url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const router = express.Router();

// Path to backend configs directory (uses configs from project root)
const EXAMPLES_BASE_PATH = path.join(__dirname, '../../../configs/v1');

// Available examples metadata
// NOTE: Only v1 format mechanisms are supported
// Examples are now loaded from configs/v1/<mechanism>/example.json
const EXAMPLES = {
    chapman: {
        id: 'chapman',
        name: 'Chapman Mechanism',
        description: 'Stratospheric oxygen chemistry with photolysis',
        mechanism: 'chapman',
        file: 'chapman/example.json',
    },
    chapman_evolving: {
        id: 'chapman_evolving',
        name: 'Chapman with Evolving Conditions',
        description: 'Chapman mechanism with time-varying temperature (6 hour day-night cycle)',
        mechanism: 'chapman',
        file: 'chapman/example_evolving.json',
    },
    ts1: {
        id: 'ts1',
        name: 'TS1 Mechanism',
        description: '209 species tropospheric mechanism',
        mechanism: 'ts1',
        file: 'ts1/example.json',
    },
    ts1_evolving: {
        id: 'ts1_evolving',
        name: 'TS1 with Diurnal Variation',
        description: 'TS1 with realistic 24-hour temperature and pressure changes',
        mechanism: 'ts1',
        file: 'ts1/example_evolving.json',
    },
    full_configuration: {
        id: 'full_configuration',
        name: 'Full Configuration Test',
        description: 'Comprehensive test with all reaction types (ARRHENIUS, TROE, PHOTOLYSIS, SURFACE, etc.)',
        mechanism: 'full_configuration',
        file: 'full_configuration/example.json',
    },
    /* DISABLED: No v1 version available
    analytical: {
        id: 'analytical',
        name: 'Analytical Mechanism',
        description: 'Simple 3-species test mechanism (A→B→C)',
        mechanism: 'analytical',
        file: 'analytical/example.json',
    },
    */
};

// GET /api/examples - List all available examples
router.get('/', (req, res) => {
    try {
        const examplesList = Object.values(EXAMPLES).map(ex => ({
            id: ex.id,
            name: ex.name,
            description: ex.description,
            mechanism: ex.mechanism,
        }));

        res.json({
            success: true,
            examples: examplesList,
        });
    } catch (error) {
        console.error('Error listing examples:', error);
        res.status(500).json({
            success: false,
            error: 'Failed to list examples',
            message: error.message,
        });
    }
});

// GET /api/examples/:id - Load specific example configuration
router.get('/:id', (req, res) => {
    try {
        const { id } = req.params;
        const example = EXAMPLES[id];

        if (!example) {
            return res.status(404).json({
                success: false,
                error: 'Example not found',
                message: `No example found with id: ${id}`,
            });
        }

        const examplePath = path.join(EXAMPLES_BASE_PATH, example.file);

        if (!fs.existsSync(examplePath)) {
            return res.status(404).json({
                success: false,
                error: 'Example file not found',
                message: `Example file does not exist: ${example.file}`,
            });
        }

        const exampleData = JSON.parse(fs.readFileSync(examplePath, 'utf8'));

        res.json({
            success: true,
            example: exampleData,
        });
    } catch (error) {
        console.error('Error loading example:', error);
        res.status(500).json({
            success: false,
            error: 'Failed to load example',
            message: error.message,
        });
    }
});

export default router;
