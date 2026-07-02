import assert from 'node:assert';
import { describe, it } from 'node:test';
import { types, reactionTypes } from '../../mechanism_configuration/index.js';

const { ReactionComponent } = types;

// Expected mechanism-configuration type string for each reaction class.
const EXPECTED_TYPES = {
  Arrhenius: 'ARRHENIUS',
  Branched: 'BRANCHED_NO_RO2',
  Emission: 'EMISSION',
  FirstOrderLoss: 'FIRST_ORDER_LOSS',
  Photolysis: 'PHOTOLYSIS',
  Surface: 'SURFACE',
  TaylorSeries: 'TAYLOR_SERIES',
  Troe: 'TROE',
  TernaryChemicalActivation: 'TERNARY_CHEMICAL_ACTIVATION',
  Tunneling: 'TUNNELING',
  UserDefined: 'USER_DEFINED',
  LambdaRateConstant: 'LAMBDA_RATE_CONSTANT',
};

// Minimal-but-serializable constructor params for each reaction class.
const reactants = [new ReactionComponent({ name: 'A' })];
const products = [new ReactionComponent({ name: 'B' })];
const PARAMS = {
  Arrhenius: { reactants, products, name: 'r', gas_phase: 'gas' },
  Branched: {
    reactants,
    nitrate_products: products,
    alkoxy_products: products,
    name: 'r',
    gas_phase: 'gas',
  },
  Emission: { products, name: 'r', gas_phase: 'gas' },
  FirstOrderLoss: { reactants, name: 'r', gas_phase: 'gas' },
  Photolysis: { reactants, products, name: 'r', gas_phase: 'gas' },
  Surface: {
    gas_phase_species: new ReactionComponent({ name: 'A' }),
    gas_phase_products: products,
    name: 'r',
    gas_phase: 'gas',
  },
  TaylorSeries: { reactants, products, name: 'r', gas_phase: 'gas' },
  Troe: { reactants, products, name: 'r', gas_phase: 'gas' },
  TernaryChemicalActivation: { reactants, products, name: 'r', gas_phase: 'gas' },
  Tunneling: { reactants, products, name: 'r', gas_phase: 'gas' },
  UserDefined: { reactants, products, name: 'r', gas_phase: 'gas' },
  LambdaRateConstant: { reactants, products, name: 'r', gas_phase: 'gas' },
};

describe('reaction type `.type` is accessible', () => {
  it('exposes the same set of classes and expectations', () => {
    assert.deepStrictEqual(
      Object.keys(reactionTypes).sort(),
      Object.keys(EXPECTED_TYPES).sort(),
      'EXPECTED_TYPES is out of sync with the exported reactionTypes'
    );
  });

  for (const [className, expectedType] of Object.entries(EXPECTED_TYPES)) {
    describe(className, () => {
      const ReactionClass = reactionTypes[className];

      it('exposes the type as a static property (no instantiation required)', () => {
        assert.strictEqual(ReactionClass.type, expectedType);
      });

      it('exposes the type as an instance property', () => {
        const instance = new ReactionClass(PARAMS[className]);
        assert.strictEqual(instance.type, expectedType);
        assert.strictEqual(instance.type, ReactionClass.type);
      });

      it('serializes the same type string in getJSON()', () => {
        const instance = new ReactionClass(PARAMS[className]);
        assert.strictEqual(instance.getJSON()['type'], instance.type);
      });
    });
  }
});
