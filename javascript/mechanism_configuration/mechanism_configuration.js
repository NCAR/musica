import { convertOtherProperties } from './utils.js';

// ----------------------------------------------------------------------------
// Core types (formerly types.js)
// ----------------------------------------------------------------------------

/**
 * @typedef {Object} SpeciesParams
 * @property {string} name
 * @property {number} [molecular_weight] Molecular weight [kg mol-1].
 * @property {number} [constant_concentration] Constant concentration [mol m-3].
 * @property {number} [constant_mixing_ratio] Constant mixing ratio [mol mol-1].
 * @property {boolean} [is_third_body]
 */

/**
 * A chemical species and its physical properties. Arbitrary extra properties
 * may be supplied and are preserved on serialization.
 */
class Species {
  #keys = [
    'name',
    'molecular_weight',
    'constant_concentration',
    'constant_mixing_ratio',
    'is_third_body',
  ];
  /**
   * @param {SpeciesParams} params
   */
  constructor(params) {
    this.name = params['name'];
    this.molecular_weight = params['molecular_weight'];
    this.constant_concentration = params['constant_concentration'];
    this.constant_mixing_ratio = params['constant_mixing_ratio'];
    this.is_third_body = params['is_third_body'];
    this.other_properties = {};
    // Allows end-users to add arbitrary properties as simple key-value pairs just like the other defined properties
    Object.entries(params).forEach(([key, value]) => {
      if (this.#keys.includes(key) == false) {
        this.other_properties[key] = value;
      }
    });
  }
  getJSON() {
    let obj = {};
    obj['name'] = this.name;
    obj['molecular weight [kg mol-1]'] = this.molecular_weight;
    obj['constant concentration [mol m-3]'] = this.constant_concentration;
    obj['constant mixing ratio [mol mol-1]'] = this.constant_mixing_ratio;
    obj['is third body'] = this.is_third_body;
    const ops = convertOtherProperties(this.other_properties);
    Object.assign(obj, ops);
    return obj;
  }
}

/**
 * @typedef {Object} PhaseSpeciesParams
 * @property {string} name
 * @property {number} [diffusion_coefficient] Diffusion coefficient [m2 s-1].
 */

/**
 * A species as it participates in a particular phase, optionally carrying a
 * phase-specific diffusion coefficient.
 */
class PhaseSpecies {
  #keys = ['name', 'diffusion_coefficient'];
  /**
   * @param {PhaseSpeciesParams} params
   */
  constructor(params) {
    this.name = params['name'];
    this.diffusion_coefficient = params['diffusion_coefficient'];
    this.other_properties = {};
    Object.entries(params).forEach(([key, value]) => {
      if (this.#keys.includes(key) == false) {
        this.other_properties[key] = value;
      }
    });
  }
  getJSON() {
    let obj = {};
    obj['name'] = this.name;
    obj['diffusion coefficient [m2 s-1]'] = this.diffusion_coefficient;
    const ops = convertOtherProperties(this.other_properties);
    Object.assign(obj, ops);
    return obj;
  }
}

/**
 * @typedef {Object} PhaseParams
 * @property {string} name
 * @property {Array<Species | PhaseSpecies>} species
 */

/**
 * A phase (for example the gas phase) and the set of species it contains.
 */
class Phase {
  #keys = ['name', 'species'];
  /**
   * @param {PhaseParams} params
   */
  constructor(params) {
    this.name = params['name'];
    this.species = params['species'];
    this.other_properties = {};
    Object.entries(params).forEach(([key, value]) => {
      if (this.#keys.includes(key) == false) {
        this.other_properties[key] = value;
      }
    });
  }
  getJSON() {
    let obj = {};
    obj['name'] = this.name;
    obj['species'] = this.species.map((s) => {
      if (s instanceof PhaseSpecies) return s.getJSON();
      if (s instanceof Species) {
        const ps = new PhaseSpecies({ name: s.name });
        return ps.getJSON();
      }
    });
    const ops = convertOtherProperties(this.other_properties);
    Object.assign(obj, ops);
    return obj;
  }
}

/**
 * @typedef {Object} ReactionComponentParams
 * @property {string} [name]
 * @property {number} [coefficient] Stoichiometric coefficient (defaults to 1.0).
 */

/**
 * A reactant or product entry: a species name and its stoichiometric
 * coefficient (defaulting to 1.0). A component always refers to a species, so
 * the reference is simply `name`.
 */
class ReactionComponent {
  #keys = ['name', 'coefficient'];
  /**
   * @param {ReactionComponentParams} params
   */
  constructor(params) {
    this.name = params['name'];
    this.coefficient = params['coefficient'] || 1.0;
    this.other_properties = {};
    Object.entries(params).forEach(([key, value]) => {
      if (this.#keys.includes(key) == false) {
        this.other_properties[key] = value;
      }
    });
  }
  getJSON() {
    let obj = {};
    obj['name'] = this.name;
    obj['coefficient'] = this.coefficient;
    const ops = convertOtherProperties(this.other_properties);
    Object.assign(obj, ops);
    return obj;
  }
}

// ----------------------------------------------------------------------------
// Reaction types (formerly reaction_types.js)
//
// Each reaction class exposes its mechanism-configuration type string both as a
// static property (e.g. `Arrhenius.type`) and on each instance (e.g.
// `new Arrhenius(...).type`). The static field is the single source of truth and
// is what `getJSON()` serializes.
// ----------------------------------------------------------------------------

/**
 * @typedef {Object} ArrheniusParams
 * @property {ReactionComponent[]} reactants
 * @property {ReactionComponent[]} products
 * @property {string} [name]
 * @property {string} [gas_phase]
 * @property {number} [A]
 * @property {number} [B]
 * @property {number} [C] Mutually exclusive with `Ea`.
 * @property {number} [D]
 * @property {number} [E]
 * @property {number} [Ea] Mutually exclusive with `C`.
 */

/**
 * Arrhenius reaction-rate expression.
 */
class Arrhenius {
  /** @type {'ARRHENIUS'} */
  static type = 'ARRHENIUS';
  #keys = ['A', 'B', 'C', 'D', 'E', 'Ea', 'reactants', 'products', 'name', 'gas_phase'];
  /**
   * @param {ArrheniusParams} params
   */
  constructor(params) {
    /** @type {'ARRHENIUS'} */
    this.type = Arrhenius.type;
    this.A = params['A'] || 1.0;
    this.B = params['B'] || 0.0;
    this.C = params['C'] || 0.0;
    this.D = params['D'] || 300.0;
    this.E = params['E'] || 0.0;
    // C and Ea are mutually exclusive
    this.Ea = params['Ea'];
    this.reactants = params['reactants'];
    this.products = params['products'];
    this.name = params['name'];
    this.gas_phase = params['gas_phase'];
    this.other_properties = {};
    Object.entries(params).forEach(([key, value]) => {
      if (this.#keys.includes(key) == false) {
        this.other_properties[key] = value;
      }
    });
  }
  getJSON() {
    let obj = {};
    obj['type'] = this.type;
    obj['name'] = this.name;
    obj['A'] = this.A;
    obj['B'] = this.B;
    if (this.Ea === undefined) obj['C'] = this.C;
    else obj['Ea'] = this.Ea;
    obj['D'] = this.D;
    obj['E'] = this.E;
    obj['gas phase'] = this.gas_phase;
    obj['reactants'] = this.reactants.map((r) => r.getJSON());
    obj['products'] = this.products.map((p) => p.getJSON());
    const ops = convertOtherProperties(this.other_properties);
    Object.assign(obj, ops);
    return obj;
  }
}

/**
 * @typedef {Object} BranchedParams
 * @property {ReactionComponent[]} reactants
 * @property {ReactionComponent[]} nitrate_products
 * @property {ReactionComponent[]} alkoxy_products
 * @property {string} [name]
 * @property {string} [gas_phase]
 * @property {number} [X]
 * @property {number} [Y]
 * @property {number} [a0]
 * @property {number} [n]
 */

/**
 * Branched (no-RO2) reaction-rate expression producing separate nitrate and
 * alkoxy product channels.
 */
class Branched {
  /** @type {'BRANCHED_NO_RO2'} */
  static type = 'BRANCHED_NO_RO2';
  #keys = [
    'X',
    'Y',
    'a0',
    'n',
    'reactants',
    'nitrate_products',
    'alkoxy_products',
    'name',
    'gas_phase',
  ];
  /**
   * @param {BranchedParams} params
   */
  constructor(params) {
    /** @type {'BRANCHED_NO_RO2'} */
    this.type = Branched.type;
    this.X = params['X'];
    this.Y = params['Y'];
    this.a0 = params['a0'];
    this.n = params['n'];
    this.reactants = params['reactants'];
    this.nitrate_products = params['nitrate_products'];
    this.alkoxy_products = params['alkoxy_products'];
    this.name = params['name'];
    this.gas_phase = params['gas_phase'];
    this.other_properties = {};
    Object.entries(params).forEach(([key, value]) => {
      if (this.#keys.includes(key) == false) {
        this.other_properties[key] = value;
      }
    });
  }
  getJSON() {
    let obj = {};
    obj['type'] = this.type;
    obj['name'] = this.name;
    obj['X'] = this.X;
    obj['Y'] = this.Y;
    obj['a0'] = this.a0;
    obj['n'] = this.n;
    obj['gas phase'] = this.gas_phase;
    obj['reactants'] = this.reactants.map((r) => r.getJSON());
    obj['nitrate products'] = this.nitrate_products.map((p) => p.getJSON());
    obj['alkoxy products'] = this.alkoxy_products.map((p) => p.getJSON());
    const ops = convertOtherProperties(this.other_properties);
    Object.assign(obj, ops);
    return obj;
  }
}

/**
 * @typedef {Object} EmissionParams
 * @property {ReactionComponent[]} products
 * @property {string} [name]
 * @property {string} [gas_phase]
 * @property {number} [scaling_factor] Defaults to 1.0.
 */

/**
 * Emission of one or more species at a scaled rate.
 */
class Emission {
  /** @type {'EMISSION'} */
  static type = 'EMISSION';
  #keys = ['scaling_factor', 'products', 'name', 'gas_phase'];
  /**
   * @param {EmissionParams} params
   */
  constructor(params) {
    /** @type {'EMISSION'} */
    this.type = Emission.type;
    this.scaling_factor = params['scaling_factor'] || 1.0;
    this.products = params['products'];
    this.name = params['name'];
    this.gas_phase = params['gas_phase'];
    this.other_properties = {};
    Object.entries(params).forEach(([key, value]) => {
      if (this.#keys.includes(key) == false) {
        this.other_properties[key] = value;
      }
    });
  }
  getJSON() {
    let obj = {};
    obj['type'] = this.type;
    obj['name'] = this.name;
    obj['scaling factor'] = this.scaling_factor;
    obj['gas phase'] = this.gas_phase;
    obj['products'] = this.products.map((p) => p.getJSON());
    const ops = convertOtherProperties(this.other_properties);
    Object.assign(obj, ops);
    return obj;
  }
}

/**
 * @typedef {Object} FirstOrderLossParams
 * @property {ReactionComponent[]} reactants
 * @property {ReactionComponent[]} [products] Optional; used to compute integrated rates.
 * @property {string} [name]
 * @property {string} [gas_phase]
 * @property {number} [scaling_factor] Defaults to 1.0.
 */

/**
 * First-order loss of one or more species at a scaled rate.
 */
class FirstOrderLoss {
  /** @type {'FIRST_ORDER_LOSS'} */
  static type = 'FIRST_ORDER_LOSS';
  #keys = ['scaling_factor', 'reactants', 'products', 'name', 'gas_phase'];
  /**
   * @param {FirstOrderLossParams} params
   */
  constructor(params) {
    /** @type {'FIRST_ORDER_LOSS'} */
    this.type = FirstOrderLoss.type;
    this.scaling_factor = params['scaling_factor'] || 1.0;
    this.reactants = params['reactants'];
    // products are optional for first-order loss (used to compute integrated rates)
    this.products = params['products'] || [];
    this.name = params['name'];
    this.gas_phase = params['gas_phase'];
    this.other_properties = {};
    Object.entries(params).forEach(([key, value]) => {
      if (this.#keys.includes(key) == false) {
        this.other_properties[key] = value;
      }
    });
  }
  getJSON() {
    let obj = {};
    obj['type'] = this.type;
    obj['name'] = this.name;
    obj['scaling factor'] = this.scaling_factor;
    obj['gas phase'] = this.gas_phase;
    obj['reactants'] = this.reactants.map((r) => r.getJSON());
    // only emit products when present, so configs without them stay unchanged
    if (this.products.length > 0) {
      obj['products'] = this.products.map((p) => p.getJSON());
    }
    const ops = convertOtherProperties(this.other_properties);
    Object.assign(obj, ops);
    return obj;
  }
}

/**
 * @typedef {Object} PhotolysisParams
 * @property {ReactionComponent[]} reactants
 * @property {ReactionComponent[]} products
 * @property {string} [name]
 * @property {string} [gas_phase]
 * @property {number} [scaling_factor] Defaults to 1.0.
 */

/**
 * Photolysis reaction driven by a scaled photolysis rate.
 */
class Photolysis {
  /** @type {'PHOTOLYSIS'} */
  static type = 'PHOTOLYSIS';
  #keys = ['scaling_factor', 'reactants', 'products', 'name', 'gas_phase'];
  /**
   * @param {PhotolysisParams} params
   */
  constructor(params) {
    /** @type {'PHOTOLYSIS'} */
    this.type = Photolysis.type;
    this.scaling_factor = params['scaling_factor'] || 1.0;
    this.reactants = params['reactants'];
    this.products = params['products'];
    this.name = params['name'];
    this.gas_phase = params['gas_phase'];
    this.other_properties = {};
    Object.entries(params).forEach(([key, value]) => {
      if (this.#keys.includes(key) == false) {
        this.other_properties[key] = value;
      }
    });
  }
  getJSON() {
    let obj = {};
    obj['type'] = this.type;
    obj['name'] = this.name;
    obj['scaling factor'] = this.scaling_factor;
    obj['gas phase'] = this.gas_phase;
    obj['reactants'] = this.reactants.map((r) => r.getJSON());
    obj['products'] = this.products.map((p) => p.getJSON());
    const ops = convertOtherProperties(this.other_properties);
    Object.assign(obj, ops);
    return obj;
  }
}

/**
 * @typedef {Object} SurfaceParams
 * @property {ReactionComponent} gas_phase_species
 * @property {ReactionComponent[]} gas_phase_products
 * @property {string} [name]
 * @property {string} [gas_phase]
 * @property {number} [reaction_probability] Defaults to 1.0.
 */

/**
 * Surface (heterogeneous) reaction characterized by a reaction probability.
 */
class Surface {
  /** @type {'SURFACE'} */
  static type = 'SURFACE';
  #keys = ['reaction_probability', 'gas_phase_species', 'gas_phase_products', 'name', 'gas_phase'];
  /**
   * @param {SurfaceParams} params
   */
  constructor(params) {
    /** @type {'SURFACE'} */
    this.type = Surface.type;
    this.reaction_probability = params['reaction_probability'] || 1.0;
    this.gas_phase_species = params['gas_phase_species'];
    this.gas_phase_products = params['gas_phase_products'];
    this.name = params['name'];
    this.gas_phase = params['gas_phase'];
    this.other_properties = {};
    Object.entries(params).forEach(([key, value]) => {
      if (this.#keys.includes(key) == false) {
        this.other_properties[key] = value;
      }
    });
  }
  getJSON() {
    let obj = {};
    obj['type'] = this.type;
    obj['name'] = this.name;
    obj['reaction probability'] = this.reaction_probability;
    obj['gas phase'] = this.gas_phase;
    // b/c Kyle said so ??
    obj['gas-phase species'] = this.gas_phase_species.name;
    obj['gas-phase products'] = this.gas_phase_products.map((p) => p.getJSON());
    const ops = convertOtherProperties(this.other_properties);
    Object.assign(obj, ops);
    return obj;
  }
}

/**
 * @typedef {Object} TaylorSeriesParams
 * @property {ReactionComponent[]} reactants
 * @property {ReactionComponent[]} products
 * @property {string} [name]
 * @property {string} [gas_phase]
 * @property {number} [A]
 * @property {number} [B]
 * @property {number} [C] Mutually exclusive with `Ea`.
 * @property {number} [D]
 * @property {number} [E]
 * @property {number} [Ea] Mutually exclusive with `C`.
 * @property {number[]} [taylor_coefficients] Defaults to [1.0].
 */

/**
 * Taylor-series reaction-rate expression.
 */
class TaylorSeries {
  /** @type {'TAYLOR_SERIES'} */
  static type = 'TAYLOR_SERIES';
  #keys = [
    'A',
    'B',
    'C',
    'D',
    'E',
    'Ea',
    'taylor_coefficients',
    'reactants',
    'products',
    'name',
    'gas_phase',
  ];
  /**
   * @param {TaylorSeriesParams} params
   */
  constructor(params) {
    /** @type {'TAYLOR_SERIES'} */
    this.type = TaylorSeries.type;
    this.A = params['A'] || 1.0;
    this.B = params['B'] || 0.0;
    this.C = params['C'] || 0.0;
    this.D = params['D'] || 300.0;
    this.E = params['E'] || 0.0;
    // C and Ea are mutually exclusive
    this.Ea = params['Ea'];
    this.taylor_coefficients = params['taylor_coefficients'] || [1.0];
    this.reactants = params['reactants'];
    this.products = params['products'];
    this.name = params['name'];
    this.gas_phase = params['gas_phase'];
    this.other_properties = {};
    Object.entries(params).forEach(([key, value]) => {
      if (this.#keys.includes(key) == false) {
        this.other_properties[key] = value;
      }
    });
  }
  getJSON() {
    let obj = {};
    obj['type'] = this.type;
    obj['name'] = this.name;
    obj['A'] = this.A;
    obj['B'] = this.B;
    if (this.Ea === undefined) obj['C'] = this.C;
    else obj['Ea'] = this.Ea;
    obj['D'] = this.D;
    obj['E'] = this.E;
    obj['taylor coefficients'] = this.taylor_coefficients;
    obj['gas phase'] = this.gas_phase;
    obj['reactants'] = this.reactants.map((r) => r.getJSON());
    obj['products'] = this.products.map((p) => p.getJSON());
    const ops = convertOtherProperties(this.other_properties);
    Object.assign(obj, ops);
    return obj;
  }
}

/**
 * Shared parameter shape for the Troe and ternary-chemical-activation rate forms.
 * @typedef {Object} TroeLikeParams
 * @property {ReactionComponent[]} reactants
 * @property {ReactionComponent[]} products
 * @property {string} [name]
 * @property {string} [gas_phase]
 * @property {number} [k0_A]
 * @property {number} [k0_B]
 * @property {number} [k0_C]
 * @property {number} [kinf_A]
 * @property {number} [kinf_B]
 * @property {number} [kinf_C]
 * @property {number} [Fc] Defaults to 0.6.
 * @property {number} [N] Defaults to 1.0.
 */

/**
 * Troe falloff reaction-rate expression.
 */
class Troe {
  /** @type {'TROE'} */
  static type = 'TROE';
  #keys = [
    'k0_A',
    'k0_B',
    'k0_C',
    'kinf_A',
    'kinf_B',
    'kinf_C',
    'Fc',
    'N',
    'reactants',
    'products',
    'name',
    'gas_phase',
  ];
  /**
   * @param {TroeLikeParams} params
   */
  constructor(params) {
    /** @type {'TROE'} */
    this.type = Troe.type;
    this.k0_A = params['k0_A'] || 1.0;
    this.k0_B = params['k0_B'] || 0.0;
    this.k0_C = params['k0_C'] || 0.0;
    this.kinf_A = params['kinf_A'] || 1.0;
    this.kinf_B = params['kinf_B'] || 0.0;
    this.kinf_C = params['kinf_C'] || 0.0;
    this.Fc = params['Fc'] || 0.6;
    this.N = params['N'] || 1.0;
    this.reactants = params['reactants'];
    this.products = params['products'];
    this.name = params['name'];
    this.gas_phase = params['gas_phase'];
    this.other_properties = {};
    Object.entries(params).forEach(([key, value]) => {
      if (this.#keys.includes(key) == false) {
        this.other_properties[key] = value;
      }
    });
  }
  getJSON() {
    let obj = {};
    obj['name'] = this.name;
    obj['type'] = this.type;
    obj['k0_A'] = this.k0_A;
    obj['k0_B'] = this.k0_B;
    obj['k0_C'] = this.k0_C;
    obj['kinf_A'] = this.kinf_A;
    obj['kinf_B'] = this.kinf_B;
    obj['kinf_C'] = this.kinf_C;
    obj['Fc'] = this.Fc;
    obj['N'] = this.N;
    obj['gas phase'] = this.gas_phase;
    obj['reactants'] = this.reactants.map((r) => r.getJSON());
    obj['products'] = this.products.map((p) => p.getJSON());
    const ops = convertOtherProperties(this.other_properties);
    Object.assign(obj, ops);
    return obj;
  }
}

/**
 * Ternary chemical-activation reaction-rate expression.
 */
class TernaryChemicalActivation {
  /** @type {'TERNARY_CHEMICAL_ACTIVATION'} */
  static type = 'TERNARY_CHEMICAL_ACTIVATION';
  #keys = [
    'k0_A',
    'k0_B',
    'k0_C',
    'kinf_A',
    'kinf_B',
    'kinf_C',
    'Fc',
    'N',
    'reactants',
    'products',
    'name',
    'gas_phase',
  ];
  /**
   * @param {TroeLikeParams} params
   */
  constructor(params) {
    /** @type {'TERNARY_CHEMICAL_ACTIVATION'} */
    this.type = TernaryChemicalActivation.type;
    this.k0_A = params['k0_A'] || 1.0;
    this.k0_B = params['k0_B'] || 0.0;
    this.k0_C = params['k0_C'] || 0.0;
    this.kinf_A = params['kinf_A'] || 1.0;
    this.kinf_B = params['kinf_B'] || 0.0;
    this.kinf_C = params['kinf_C'] || 0.0;
    this.Fc = params['Fc'] || 0.6;
    this.N = params['N'] || 1.0;
    this.reactants = params['reactants'];
    this.products = params['products'];
    this.name = params['name'];
    this.gas_phase = params['gas_phase'];
    this.other_properties = {};
    Object.entries(params).forEach(([key, value]) => {
      if (this.#keys.includes(key) == false) {
        this.other_properties[key] = value;
      }
    });
  }
  getJSON() {
    let obj = {};
    obj['type'] = this.type;
    obj['name'] = this.name;
    obj['gas phase'] = this.gas_phase;
    obj['k0_A'] = this.k0_A;
    obj['k0_B'] = this.k0_B;
    obj['k0_C'] = this.k0_C;
    obj['kinf_A'] = this.kinf_A;
    obj['kinf_B'] = this.kinf_B;
    obj['kinf_C'] = this.kinf_C;
    obj['Fc'] = this.Fc;
    obj['N'] = this.N;
    obj['reactants'] = this.reactants.map((r) => r.getJSON());
    obj['products'] = this.products.map((p) => p.getJSON());
    const ops = convertOtherProperties(this.other_properties);
    Object.assign(obj, ops);
    return obj;
  }
}

/**
 * @typedef {Object} TunnelingParams
 * @property {ReactionComponent[]} reactants
 * @property {ReactionComponent[]} products
 * @property {string} [name]
 * @property {string} [gas_phase]
 * @property {number} [A]
 * @property {number} [B]
 * @property {number} [C]
 */

/**
 * Wigner tunneling reaction-rate expression.
 */
class Tunneling {
  /** @type {'TUNNELING'} */
  static type = 'TUNNELING';
  #keys = ['A', 'B', 'C', 'reactants', 'products', 'name', 'gas_phase'];
  /**
   * @param {TunnelingParams} params
   */
  constructor(params) {
    /** @type {'TUNNELING'} */
    this.type = Tunneling.type;
    this.A = params['A'] || 1.0;
    this.B = params['B'] || 0.0;
    this.C = params['C'] || 0.0;
    this.reactants = params['reactants'];
    this.products = params['products'];
    this.name = params['name'];
    this.gas_phase = params['gas_phase'];
    this.other_properties = {};
    Object.entries(params).forEach(([key, value]) => {
      if (this.#keys.includes(key) == false) {
        this.other_properties[key] = value;
      }
    });
  }
  getJSON() {
    let obj = {};
    obj['type'] = this.type;
    obj['name'] = this.name;
    obj['A'] = this.A;
    obj['B'] = this.B;
    obj['C'] = this.C;
    obj['gas phase'] = this.gas_phase;
    obj['reactants'] = this.reactants.map((r) => r.getJSON());
    obj['products'] = this.products.map((p) => p.getJSON());
    const ops = convertOtherProperties(this.other_properties);
    Object.assign(obj, ops);
    return obj;
  }
}

/**
 * @typedef {Object} UserDefinedParams
 * @property {ReactionComponent[]} reactants
 * @property {ReactionComponent[]} products
 * @property {string} [name]
 * @property {string} [gas_phase]
 * @property {number} [scaling_factor] Defaults to 1.0.
 */

/**
 * User-defined reaction whose rate is supplied externally.
 */
class UserDefined {
  /** @type {'USER_DEFINED'} */
  static type = 'USER_DEFINED';
  #keys = ['scaling_factor', 'reactants', 'products', 'name', 'gas_phase'];
  /**
   * @param {UserDefinedParams} params
   */
  constructor(params) {
    /** @type {'USER_DEFINED'} */
    this.type = UserDefined.type;
    this.scaling_factor = params['scaling_factor'] || 1.0;
    this.reactants = params['reactants'];
    this.products = params['products'];
    this.name = params['name'];
    this.gas_phase = params['gas_phase'];
    this.other_properties = {};
    Object.entries(params).forEach(([key, value]) => {
      if (this.#keys.includes(key) == false) {
        this.other_properties[key] = value;
      }
    });
  }
  getJSON() {
    let obj = {};
    obj['type'] = this.type;
    obj['name'] = this.name;
    obj['scaling factor'] = this.scaling_factor;
    obj['gas phase'] = this.gas_phase;
    obj['reactants'] = this.reactants.map((r) => r.getJSON());
    obj['products'] = this.products.map((p) => p.getJSON());
    const ops = convertOtherProperties(this.other_properties);
    Object.assign(obj, ops);
    return obj;
  }
}

/**
 * @typedef {Object} LambdaRateConstantParams
 * @property {ReactionComponent[]} reactants
 * @property {ReactionComponent[]} products
 * @property {string} [name]
 * @property {string} [gas_phase]
 * @property {string} [lambda_function] A C++ lambda expression string used by the
 *   parser. When using a JavaScript callback (setReactionRateCallback), this acts
 *   as a placeholder and is overridden at runtime.
 */

/**
 * Reaction whose rate constant is provided by a lambda callback (a C++ lambda
 * string for the parser, or a JavaScript callback set at runtime).
 */
class LambdaRateConstant {
  /** @type {'LAMBDA_RATE_CONSTANT'} */
  static type = 'LAMBDA_RATE_CONSTANT';
  #keys = ['reactants', 'products', 'name', 'gas_phase', 'lambda_function'];
  /**
   * @param {LambdaRateConstantParams} params
   */
  constructor(params) {
    /** @type {'LAMBDA_RATE_CONSTANT'} */
    this.type = LambdaRateConstant.type;
    this.reactants = params['reactants'];
    this.products = params['products'];
    this.name = params['name'];
    this.gas_phase = params['gas_phase'];
    // A C++ lambda expression string used by the parser. When using a JavaScript
    // callback (setReactionRateCallback), this acts as a placeholder and is
    // overridden at runtime. Defaults to a zero-returning lambda.
    this.lambda_function =
      params['lambda_function'] || '[](double T, double P, double air_density) { return 0.0; }';
    this.other_properties = {};
    Object.entries(params).forEach(([key, value]) => {
      if (this.#keys.includes(key) == false) {
        this.other_properties[key] = value;
      }
    });
  }

  getJSON() {
    let obj = {};
    obj['type'] = this.type;
    obj['name'] = this.name;
    obj['gas phase'] = this.gas_phase;
    obj['lambda function'] = this.lambda_function;
    obj['reactants'] = this.reactants.map((r) => r.getJSON());
    obj['products'] = this.products.map((p) => p.getJSON());

    const ops = convertOtherProperties(this.other_properties);
    Object.assign(obj, ops);
    return obj;
  }
}

// ----------------------------------------------------------------------------
// Mechanism (formerly mechanism.js)
// ----------------------------------------------------------------------------

/**
 * Any of the concrete reaction-rate classes exported via `reactionTypes`.
 * @typedef {Arrhenius | Branched | Emission | FirstOrderLoss | Photolysis | Surface | TaylorSeries | Troe | TernaryChemicalActivation | Tunneling | UserDefined | LambdaRateConstant} Reaction
 */

/**
 * @typedef {Object} MechanismParams
 * @property {string} [name]
 * @property {string} [version]
 * @property {Species[]} [species]
 * @property {Phase[]} [phases]
 * @property {Reaction[]} [reactions]
 */

/**
 * A complete mechanism — its species, phases, and reactions — serializable to
 * MUSICA's JSON configuration via {@link Mechanism#getJSON} / {@link Mechanism#getString}.
 */
class Mechanism {
  /**
   * @param {MechanismParams} params
   */
  constructor({ name, version, species, phases, reactions }) {
    this.name = name;
    this.version = version;
    this.species = species;
    this.phases = phases;
    this.reactions = reactions;
  }
  getJSON() {
    let obj = {};
    obj['name'] = this.name;
    obj['version'] = this.version;
    obj['species'] = this.species.map((s) => s.getJSON());
    obj['phases'] = this.phases.map((p) => p.getJSON());
    obj['reactions'] = this.reactions.map((r) => r.getJSON());
    return obj;
  }
  // Allows for one-step conversion to string to pass to C++
  getString() {
    return JSON.stringify(this.getJSON());
  }
}

export const types = { Species, PhaseSpecies, Phase, ReactionComponent };

export const reactionTypes = {
  Arrhenius,
  Branched,
  Emission,
  FirstOrderLoss,
  Photolysis,
  Surface,
  TaylorSeries,
  Troe,
  TernaryChemicalActivation,
  Tunneling,
  UserDefined,
  LambdaRateConstant,
};

export { Mechanism };
