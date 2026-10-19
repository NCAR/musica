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

  it('keeps the type and structure of unknown properties at every level', () => {
    const config = JSON.stringify({
      version: '1.0.0',
      name: 'Other properties',
      __author: 'someone',
      __nested: { list: ['x'], map: { key: 'value' } },
      species: [
        {
          name: 'A',
          '__quoted number': '0.5',
          '__plain number': 0.5,
          __count: 3,
          __flag: false,
          __nothing: null,
        },
        { name: 'B' },
      ],
      phases: [
        {
          name: 'gas',
          species: [{ name: 'A', __note: { a: [1, 2] } }, { name: 'B' }],
          __info: { key: 'phase' },
        },
      ],
      reactions: [
        {
          type: 'ARRHENIUS',
          A: 1.5,
          reactants: [{ name: 'A', __tags: [1, 'two', 3.5] }],
          products: [{ name: 'B' }],
          'gas phase': 'gas',
          __info: { key: 'reaction' },
        },
      ],
    });

    const mechanism = parseMechanismFromString(config);

    assert.strictEqual(mechanism.__author, 'someone');
    assert.deepStrictEqual(mechanism.__nested, {
      list: ['x'],
      map: { key: 'value' },
    });

    const A = mechanism.species[0];
    assert.strictEqual(A['__quoted number'], '0.5');
    assert.strictEqual(A['__plain number'], 0.5);
    assert.strictEqual(A.__count, 3);
    assert.strictEqual(A.__flag, false);
    assert.strictEqual(A.__nothing, null);

    assert.deepStrictEqual(mechanism.phases[0].__info, { key: 'phase' });
    assert.deepStrictEqual(mechanism.phases[0].species[0].__note, { a: [1, 2] });

    const reaction = mechanism.reactions[0];
    assert.deepStrictEqual(reaction.__info, { key: 'reaction' });
    assert.deepStrictEqual(reaction.reactants[0].__tags, [1, 'two', 3.5]);
  });

  it('round-trips mechanism other_properties through getJSON and the parser', () => {
    const { Mechanism, types, reactionTypes } = musica.mechanismConfiguration;
    const A = new types.Species({ name: 'A' });
    const B = new types.Species({ name: 'B' });
    const mechanism = new Mechanism({
      name: 'round trip',
      version: '1.0.0',
      species: [A, B],
      phases: [new types.Phase({ name: 'gas', species: [A, B] })],
      reactions: [
        new reactionTypes.Arrhenius({
          reactants: [new types.ReactionComponent({ name: 'A' })],
          products: [new types.ReactionComponent({ name: 'B' })],
          gas_phase: 'gas',
        }),
      ],
      other_properties: { nested: { list: ['x'] } },
    });

    const parsed = parseMechanismFromString(mechanism.getString());

    assert.deepStrictEqual(parsed.__nested, { list: ['x'] });
  });

  it('throws an Error that holds the parser message for an invalid config', () => {
    const config = JSON.stringify({
      version: '1.0.0',
      name: 'Invalid',
      species: [{ name: 'A' }],
      phases: [{ name: 'gas', species: [{ name: 'A' }] }],
      reactions: [
        {
          type: 'ARRHENIUS',
          reactants: [{ name: 'A' }],
          products: [{ name: 'UNDECLARED' }],
          'gas phase': 'gas',
        },
      ],
    });

    assert.throws(
      () => parseMechanismFromString(config),
      (error) => error instanceof Error && /UNDECLARED/.test(error.message)
    );
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
