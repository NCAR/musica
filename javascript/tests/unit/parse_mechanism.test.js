// Tests for parsing mechanism configurations into plain JS objects via WASM.
import { describe, it, before } from 'node:test';
import assert from 'node:assert';
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

import * as musica from '../../index.js';

const { parseMechanismFromString, parseMechanismFromFiles } = musica.mechanismConfiguration;

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const configsDir = path.join(__dirname, '../../../configs');

before(async () => {
  await musica.initModule();
});

describe('parseMechanismFromString (v1)', () => {
  it('parses an inline v1 Arrhenius mechanism into the wire shape', () => {
    const config = JSON.stringify({
      version: '1.0.0',
      name: 'Test',
      species: [{ name: 'A', 'molecular weight [kg mol-1]': 0.032 }, { name: 'B' }],
      phases: [{ name: 'gas', species: [{ name: 'A' }, { name: 'B' }] }],
      reactions: [
        {
          type: 'ARRHENIUS',
          A: 1.5,
          B: 0,
          C: 0,
          D: 300,
          E: 0,
          reactants: [{ name: 'A', coefficient: 1 }],
          products: [{ name: 'B', coefficient: 2 }],
          'gas phase': 'gas',
        },
      ],
    });

    const mechanism = parseMechanismFromString(config);

    assert.strictEqual(mechanism.version, '1.0.0');
    assert.strictEqual(mechanism.species.length, 2);
    assert.strictEqual(mechanism.species[0].name, 'A');
    assert.strictEqual(mechanism.species[0]['molecular weight [kg mol-1]'], 0.032);
    assert.strictEqual(mechanism.phases[0].name, 'gas');

    assert.strictEqual(mechanism.reactions.length, 1);
    const reaction = mechanism.reactions[0];
    assert.strictEqual(reaction.type, 'ARRHENIUS');
    assert.strictEqual(reaction.A, 1.5);
    assert.strictEqual(reaction['gas phase'], 'gas');
    assert.strictEqual(reaction.reactants[0].name, 'A');
    assert.strictEqual(reaction.products[0].name, 'B');
    assert.strictEqual(reaction.products[0].coefficient, 2);
  });

  it('parses the committed chapman v1 config, preserving unknown properties', () => {
    const config = fs.readFileSync(path.join(configsDir, 'v1/chapman/config.json'), 'utf8');
    const mechanism = parseMechanismFromString(config);

    assert.strictEqual(mechanism.version, '1.0.0');

    const ozone = mechanism.species.find((s) => s.name === 'O3');
    assert.ok(ozone, 'expected an O3 species');
    assert.strictEqual(ozone['molecular weight [kg mol-1]'], 0.048);
    // `__`-prefixed unknown properties survive the round-trip through the parser.
    assert.strictEqual(ozone['__long name'], 'ozone');

    const arrhenius = mechanism.reactions.find((r) => r.type === 'ARRHENIUS');
    assert.ok(arrhenius, 'expected an ARRHENIUS reaction');
    // component species references are emitted as `name`, not `species name`.
    assert.ok(arrhenius.reactants[0].name.length > 0);
  });
});

describe('parseMechanismFromFiles (v0)', () => {
  it('parses a multi-file v0 CAMP config from an in-memory file set', () => {
    const dir = path.join(configsDir, 'v0/analytical');
    const files = {};
    for (const name of ['config.json', 'species.json', 'reactions.json']) {
      files[`analytical/${name}`] = fs.readFileSync(path.join(dir, name), 'utf8');
    }

    const mechanism = parseMechanismFromFiles(files, 'analytical');

    const speciesNames = mechanism.species.map((s) => s.name);
    assert.ok(speciesNames.includes('A'), 'expected species A');
    assert.ok(speciesNames.includes('E'), 'expected species E');
    assert.ok(
      mechanism.reactions.some((r) => r.type === 'ARRHENIUS'),
      'expected an ARRHENIUS reaction'
    );
    assert.ok(
      mechanism.reactions.some((r) => r.type === 'USER_DEFINED'),
      'expected a USER_DEFINED reaction'
    );
  });
});
