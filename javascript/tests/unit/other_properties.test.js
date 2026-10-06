import { test, before } from 'node:test';
import assert from 'node:assert';
import * as musica from '../../index.js';

const { types, reactionTypes, Mechanism } = musica.mechanismConfiguration;
const { Species, Phase, ReactionComponent } = types;
const { Arrhenius } = reactionTypes;

before(async () => {
  await musica.initModule();
});

test('Phase accepts explicit other_properties and arbitrary top-level keys', () => {
  const phase = new Phase({
    name: 'gas',
    species: [new Species({ name: 'A' })],
    other_properties: { 'long name': 'gas phase' },
    tag: 5,
  });
  const json = phase.getJSON();
  assert.strictEqual(json['__long name'], 'gas phase');
  assert.strictEqual(json['__tag'], 5);
  assert.ok(!('__other_properties' in json));
});

test('ReactionComponent accepts explicit other_properties and arbitrary top-level keys', () => {
  const rc = new ReactionComponent({
    name: 'A',
    coefficient: 2.0,
    other_properties: { note: 'x' },
    tag: 5,
  });
  const json = rc.getJSON();
  assert.strictEqual(json['coefficient'], 2.0);
  assert.strictEqual(json['__note'], 'x');
  assert.strictEqual(json['__tag'], 5);
  assert.ok(!('__other_properties' in json));
});

test('Reactions accept explicit other_properties', () => {
  const r = new Arrhenius({
    name: 'r1',
    reactants: [new ReactionComponent({ name: 'A' })],
    products: [new ReactionComponent({ name: 'B' })],
    other_properties: { source: 'JPL' },
    tag: 5,
  });
  const json = r.getJSON();
  assert.strictEqual(json['__source'], 'JPL');
  assert.strictEqual(json['__tag'], 5);
  assert.ok(!('__other_properties' in json));
});

test('Mechanism serializes other properties on nested objects and ignores its own extra keys', () => {
  const A = new Species({ name: 'A', note: 'species' });
  const B = new Species({ name: 'B' });
  const mechanism = new Mechanism({
    name: 'm',
    version: '1.0.0',
    species: [A, B],
    phases: [new Phase({ name: 'gas', species: [A, B], note: 'phase' })],
    reactions: [
      new Arrhenius({
        reactants: [new ReactionComponent({ name: 'A', note: 'reactant' })],
        products: [new ReactionComponent({ name: 'B' })],
        gas_phase: 'gas',
        note: 'reaction',
      }),
    ],
    // mechanism_configuration does not keep other properties on the mechanism itself
    other_properties: { author: 'someone' },
    tag: 5,
  });
  const json = mechanism.getJSON();
  assert.deepStrictEqual(
    Object.keys(json).filter((key) => key.startsWith('__')),
    []
  );
  assert.strictEqual(json['species'][0]['__note'], 'species');
  assert.strictEqual(json['phases'][0]['__note'], 'phase');
  assert.strictEqual(json['reactions'][0]['__note'], 'reaction');
  assert.strictEqual(json['reactions'][0]['reactants'][0]['__note'], 'reactant');

  // The C++ parser must accept a mechanism that has other properties on every nested object
  const micm = musica.MICM.fromMechanism(mechanism);
  assert.ok(micm);
});
