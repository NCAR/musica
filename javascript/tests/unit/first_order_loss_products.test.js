import { test } from 'node:test';
import assert from 'node:assert';
import * as musica from '../../index.js';

const { types, reactionTypes } = musica.mechanismConfiguration;
const { ReactionComponent } = types;

test('FirstOrderLoss omits products when none are given', () => {
  const fol = new reactionTypes.FirstOrderLoss({
    name: 'my first order loss',
    scaling_factor: 12.3,
    gas_phase: 'gas',
    reactants: [new ReactionComponent({ species_name: 'A' })],
  });
  const json = fol.getJSON();
  assert.strictEqual(json['type'], 'FIRST_ORDER_LOSS');
  assert.strictEqual(json['reactants'][0]['species name'], 'A');
  assert.ok(
    !('products' in json),
    'products key should be absent when no products are provided',
  );
});

test('FirstOrderLoss emits products (with coefficients) when provided', () => {
  const fol = new reactionTypes.FirstOrderLoss({
    name: 'my first order loss',
    scaling_factor: 12.3,
    gas_phase: 'gas',
    reactants: [new ReactionComponent({ species_name: 'A' })],
    products: [
      new ReactionComponent({ species_name: 'B' }),
      new ReactionComponent({ species_name: 'C', coefficient: 2.0 }),
    ],
  });
  const json = fol.getJSON();
  assert.strictEqual(json['products'].length, 2);
  assert.deepStrictEqual(json['products'][0], {
    'species name': 'B',
    coefficient: 1.0,
  });
  assert.deepStrictEqual(json['products'][1], {
    'species name': 'C',
    coefficient: 2.0,
  });
});
