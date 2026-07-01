import { test } from 'node:test';
import assert from 'node:assert';
import * as musica from '../../index.js';

const { ReactionComponent } = musica.mechanismConfiguration.types;

test('ReactionComponent accepts species_name', () => {
  const c = new ReactionComponent({ species_name: 'A', coefficient: 2 });
  assert.deepStrictEqual(c.getJSON(), { 'species name': 'A', coefficient: 2 });
});

test('ReactionComponent accepts name as an alias for species_name', () => {
  const c = new ReactionComponent({ name: 'A' });
  assert.deepStrictEqual(c.getJSON(), { 'species name': 'A', coefficient: 1.0 });
});
