import assert from 'node:assert';
import { describe, it } from 'node:test';
import { types, reactionTypes } from '../../mechanism_configuration/index.js';

const { ReactionComponent } = types;

const reactants = [new ReactionComponent({ name: 'A' })];
const products = [new ReactionComponent({ name: 'B' })];
const common = { reactants, products, name: 'r', gas_phase: 'gas' };

describe('ReactionComponent.coefficient preserves an explicit 0', () => {
  it('keeps 0 rather than defaulting to 1.0', () => {
    const component = new ReactionComponent({ name: 'A', coefficient: 0 });
    assert.strictEqual(component.getJSON()['coefficient'], 0);
  });

  it('still defaults to 1.0 when omitted', () => {
    const component = new ReactionComponent({ name: 'A' });
    assert.strictEqual(component.getJSON()['coefficient'], 1.0);
  });
});

describe('Arrhenius/TaylorSeries preserve an explicit 0 for A and D', () => {
  for (const ClassName of ['Arrhenius', 'TaylorSeries']) {
    describe(ClassName, () => {
      it('keeps A: 0 rather than defaulting to 1.0', () => {
        const instance = new reactionTypes[ClassName]({ ...common, A: 0 });
        assert.strictEqual(instance.getJSON()['A'], 0);
      });

      it('keeps D: 0 rather than defaulting to 300.0', () => {
        const instance = new reactionTypes[ClassName]({ ...common, D: 0 });
        assert.strictEqual(instance.getJSON()['D'], 0);
      });

      it('still defaults A and D when omitted', () => {
        const instance = new reactionTypes[ClassName](common);
        assert.strictEqual(instance.getJSON()['A'], 1.0);
        assert.strictEqual(instance.getJSON()['D'], 300.0);
      });
    });
  }
});

describe('scaling_factor: 0 is preserved, not replaced with 1.0', () => {
  const cases = [
    ['Emission', { products, name: 'r', gas_phase: 'gas' }],
    ['FirstOrderLoss', { reactants, name: 'r', gas_phase: 'gas' }],
    ['Photolysis', common],
    ['UserDefined', common],
  ];

  for (const [ClassName, params] of cases) {
    describe(ClassName, () => {
      it('keeps scaling_factor: 0', () => {
        const instance = new reactionTypes[ClassName]({ ...params, scaling_factor: 0 });
        assert.strictEqual(instance.getJSON()['scaling factor'], 0);
      });

      it('still defaults scaling_factor to 1.0 when omitted', () => {
        const instance = new reactionTypes[ClassName](params);
        assert.strictEqual(instance.getJSON()['scaling factor'], 1.0);
      });
    });
  }
});

describe('Surface preserves reaction_probability: 0', () => {
  const params = {
    gas_phase_species: new ReactionComponent({ name: 'A' }),
    gas_phase_products: products,
    name: 'r',
    gas_phase: 'gas',
  };

  it('keeps reaction_probability: 0', () => {
    const instance = new reactionTypes.Surface({ ...params, reaction_probability: 0 });
    assert.strictEqual(instance.getJSON()['reaction probability'], 0);
  });

  it('still defaults reaction_probability to 1.0 when omitted', () => {
    const instance = new reactionTypes.Surface(params);
    assert.strictEqual(instance.getJSON()['reaction probability'], 1.0);
  });
});

describe('Troe/TernaryChemicalActivation preserve explicit 0 for k0_A, kinf_A, Fc, and N', () => {
  for (const ClassName of ['Troe', 'TernaryChemicalActivation']) {
    describe(ClassName, () => {
      it('keeps k0_A: 0, kinf_A: 0, Fc: 0, N: 0', () => {
        const instance = new reactionTypes[ClassName]({
          ...common,
          k0_A: 0,
          kinf_A: 0,
          Fc: 0,
          N: 0,
        });
        const json = instance.getJSON();
        assert.strictEqual(json['k0_A'], 0);
        assert.strictEqual(json['kinf_A'], 0);
        assert.strictEqual(json['Fc'], 0);
        assert.strictEqual(json['N'], 0);
      });

      it('still defaults k0_A, kinf_A, Fc, and N when omitted', () => {
        const instance = new reactionTypes[ClassName](common);
        const json = instance.getJSON();
        assert.strictEqual(json['k0_A'], 1.0);
        assert.strictEqual(json['kinf_A'], 1.0);
        assert.strictEqual(json['Fc'], 0.6);
        assert.strictEqual(json['N'], 1.0);
      });
    });
  }
});

describe('Tunneling preserves an explicit 0 for A', () => {
  it('keeps A: 0 rather than defaulting to 1.0', () => {
    const instance = new reactionTypes.Tunneling({ ...common, A: 0 });
    assert.strictEqual(instance.getJSON()['A'], 0);
  });

  it('still defaults A to 1.0 when omitted', () => {
    const instance = new reactionTypes.Tunneling(common);
    assert.strictEqual(instance.getJSON()['A'], 1.0);
  });
});

describe('LambdaRateConstant preserves an explicit empty lambda_function string', () => {
  it('keeps an explicit empty string rather than the default placeholder lambda', () => {
    const instance = new reactionTypes.LambdaRateConstant({ ...common, lambda_function: '' });
    assert.strictEqual(instance.getJSON()['lambda function'], '');
  });

  it('still defaults to the placeholder lambda when omitted', () => {
    const instance = new reactionTypes.LambdaRateConstant(common);
    assert.strictEqual(
      instance.getJSON()['lambda function'],
      '[](double T, double P, double air_density) { return 0.0; }'
    );
  });
});

describe('FirstOrderLoss.products still defaults to [] when omitted', () => {
  it('omits the products key when no products are given', () => {
    const instance = new reactionTypes.FirstOrderLoss({
      reactants,
      name: 'r',
      gas_phase: 'gas',
    });
    assert.ok(!('products' in instance.getJSON()));
  });
});
