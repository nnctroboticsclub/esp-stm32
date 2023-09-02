import { NVSType, type NVSTypeInstance } from "./nvs_entry_type";

export class NVSNamespace {
  namespace_name: string;
  entries: {
    [key in NVSType]: { [key: string]: NVSEntry<NVSTypeInstance<key>> }
  }

  constructor(ns: NVSNamespace | string) {
    this.entries = {
      [NVSType.U8]: {},
      [NVSType.I8]: {},
      [NVSType.U16]: {},
      [NVSType.I16]: {},
      [NVSType.U32]: {},
      [NVSType.I32]: {},
      [NVSType.U64]: {},
      [NVSType.I64]: {},
      [NVSType.STR]: {},
      [NVSType.BLOB]: {},
    };

    if (typeof ns === 'string') {
      this.namespace_name = ns;
    } else {
      this.namespace_name = ns.namespace_name;

      const types = Object.keys(NVSType)
        .map(x => NVSType[x as any] as unknown as NVSType)
        .filter(x => Number.isInteger(x));

      for (const t of types) {
        for (const key in ns.entries[t]) {
          this.entries[t][key] = ns.entries[t][key].clone(this);
        }
      }
    }
  }

  entry<T extends NVSType>(key: string, type: T) {
    type Type = NVSTypeInstance<T>;

    const store: { [key: string]: NVSEntry<NVSTypeInstance<T>> } = this.entries[type];

    if (typeof store[key] === 'undefined') {
      store[key] = new NVSEntry<Type>(this, key);
    }

    return store[key];
  }

  dump(): string[] {
    const types = Object.keys(NVSType)
      .map(x => NVSType[x as any] as unknown as NVSType)
      .filter(x => Number.isInteger(x));

    const ns_name = this.namespace_name;

    return types.map(t =>
      Object.keys(this.entries[t])
        .map((key) => {
          return `${ns_name}.${key} = ${this.entries[t][key].get()}`;
        })
    ).flat();
  }
}

export class NVSEntry<T extends string | number | number[]> {
  ns: NVSNamespace
  key: string;
  _value: T | undefined;
  s: ((value: T) => void)[] = [];

  constructor(ns: NVSNamespace, key: string) {
    this.s = [];
    this.ns = ns;
    this.key = key;
    this._value = undefined;
  }

  toString(): string {
    return `<${this.ns.namespace_name}.${this.key}>`;
  }

  toJSON(): object {
    return {
      namespace: this.ns.namespace_name,
      key: this.key,
      value: this._value
    };
  }

  subscribe(callback: (value: T) => void): () => void {
    this.s.push(callback);
    if (typeof this._value !== 'undefined') callback(this._value);
    return () => {
      this.s = this.s.filter((cb) => cb !== callback);
    };
  }

  clone(ns: NVSNamespace): NVSEntry<T> {
    let entry = new NVSEntry<T>(ns, this.key);
    entry._value = this._value;
    return entry;
  }

  get(): T | undefined {
    return this._value;
  }

  set(value: T): T {
    this._value = value;
    this.s.forEach((cb) => cb(value));
    return value;
  }
}