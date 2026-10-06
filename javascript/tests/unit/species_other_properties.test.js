import { test } from 'node:test';
import assert from 'node:assert';
import * as musica from '../../index.js';

const { types } = musica.mechanismConfiguration;
const { Species, PhaseSpecies } = types;

test('Species accepts explicit other_properties and serializes them with a __ prefix', () => {
  const s = new Species({
    name: 'A',
    molecular_weight: 0.048,
    other_properties: { 'long name': 'ozone', tag: 5 },
  });
  const json = s.getJSON();
  assert.strictEqual(json['name'], 'A');
  assert.strictEqual(json['molecular weight [kg mol-1]'], 0.048);
  assert.strictEqual(json['__long name'], 'ozone');
  assert.strictEqual(json['__tag'], 5);
});

test('Species still routes arbitrary top-level keys into other properties', () => {
  const s = new Species({ name: 'B', 'custom key': 42 });
  assert.strictEqual(s.getJSON()['__custom key'], 42);
});

test('PhaseSpecies accepts explicit other_properties', () => {
  const ps = new PhaseSpecies({
    name: 'A',
    diffusion_coefficient: 2.1e-5,
    other_properties: { note: 'x' },
  });
  const json = ps.getJSON();
  assert.strictEqual(json['diffusion coefficient [m2 s-1]'], 2.1e-5);
  assert.strictEqual(json['__note'], 'x');
});
