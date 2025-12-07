// Mechanisms API Routes
// Provides information about available chemical mechanisms

import express from 'express';
import path from 'path';
import fs from 'fs';
import { fileURLToPath } from 'url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const router = express.Router();

// Path to the MUSICA configs directory
const CONFIGS_BASE_PATH = path.join(__dirname, '../../../configs/v1');

// Available mechanisms with metadata
const MECHANISMS = {
    ts1: {
        id: 'ts1',
        name: 'TS1',
        description: '209 species tropospheric mechanism',
        species: 209,
        reactions: 512,
        configPath: path.join(CONFIGS_BASE_PATH, 'ts1/config.json'),
    },
    chapman: {
        id: 'chapman',
        name: 'Chapman',
        description: 'Stratospheric oxygen chemistry',
        species: 8,
        reactions: 12,
        configPath: path.join(CONFIGS_BASE_PATH, 'chapman/config.json'),
    },
    analytical: {
        id: 'analytical',
        name: 'Analytical',
        description: 'Simple 3-species test mechanism (A→B→C)',
        species: 3,
        reactions: 3,
        configPath: path.join(CONFIGS_BASE_PATH, 'analytical/config.json'),
    },
};

// GET /api/mechanisms - List all available mechanisms
router.get('/', (req, res) => {
    try {
        const mechanismsList = Object.values(MECHANISMS).map(mech => ({
            id: mech.id,
            name: mech.name,
            description: mech.description,
            species: mech.species,
            reactions: mech.reactions,
        }));

        res.json({
            success: true,
            mechanisms: mechanismsList,
        });
    } catch (error) {
        console.error('Error listing mechanisms:', error);
        res.status(500).json({
            success: false,
            error: 'Failed to list mechanisms',
            message: error.message,
        });
    }
});

// GET /api/mechanisms/:id - Get specific mechanism details
router.get('/:id', (req, res) => {
    try {
        const { id } = req.params;
        const mechanism = MECHANISMS[id];

        if (!mechanism) {
            return res.status(404).json({
                success: false,
                error: 'Mechanism not found',
                message: `No mechanism found with id: ${id}`,
            });
        }

        // Check if config file exists
        const configExists = fs.existsSync(mechanism.configPath);

        res.json({
            success: true,
            mechanism: {
                ...mechanism,
                configExists,
            },
        });
    } catch (error) {
        console.error('Error getting mechanism:', error);
        res.status(500).json({
            success: false,
            error: 'Failed to get mechanism',
            message: error.message,
        });
    }
});

// GET /api/mechanisms/:id/species - Get species list for a mechanism
router.get('/:id/species', (req, res) => {
    try {
        const { id } = req.params;
        const mechanism = MECHANISMS[id];

        if (!mechanism) {
            return res.status(404).json({
                success: false,
                error: 'Mechanism not found',
            });
        }

        // Load species from config (simplified - would need to read actual config)
        // For now, return sample species based on mechanism
        let species = [];

        if (id === 'chapman') {
            species = ['O2', 'O', 'O1D', 'O3', 'M'];
        } else if (id === 'ts1') {
            species = ['O3', 'NO2', 'NO', 'CO', 'CH4', 'HCHO', 'OH', 'HO2'];
        }

        res.json({
            success: true,
            mechanism: id,
            species,
        });
    } catch (error) {
        console.error('Error getting species:', error);
        res.status(500).json({
            success: false,
            error: 'Failed to get species',
            message: error.message,
        });
    }
});

export default router;
