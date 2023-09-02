export class NVSNamespace {
  namespace_name: string;
  str_entries: { [key: string]: NVSEntry<string> }
  num_entries: { [key: string]: NVSEntry<number> }
  num_array_entries: { [key: string]: NVSEntry<number[]> }

  constructor(ns: NVSNamespace | string) {
    this.str_entries = {};
    this.num_entries = {};
    this.num_array_entries = {};

    if (typeof ns === 'string') {
      this.namespace_name = ns;
    } else {
      this.namespace_name = ns.namespace_name;
      console.log('[NVSNamespace] Cloning namespace', this.namespace_name);

      for (const key in ns.str_entries) {
        this.str_entries[key] = ns.str_entries[key].clone(this);
      }

      for (const key in ns.num_entries) {
        this.num_entries[key] = ns.num_entries[key].clone(this);
      }

      for (const key in ns.num_array_entries) {
        this.num_array_entries[key] = ns.num_array_entries[key].clone(this);
      }
    }
  }

  entryStr(key: string): NVSEntry<string> {
    if (key in this.str_entries) {
      return this.str_entries[key];
    }

    this.str_entries[key] = new NVSEntry<string>(this, key);
    return this.str_entries[key];
  }
  entryNum(key: string): NVSEntry<number> {
    if (key in this.num_entries) {
      return this.num_entries[key];
    }
    this.num_entries[key] = new NVSEntry<number>(this, key);
    return this.num_entries[key];
  }
  entryNumArray(key: string): NVSEntry<number[]> {
    if (key in this.num_array_entries) {
      return this.num_array_entries[key];
    }
    this.num_array_entries[key] = new NVSEntry<number[]>(this, key);
    return this.num_array_entries[key];
  }

  entry<T extends string | number | number[]>(key: string, instance: T): NVSEntry<T> {
    if (typeof instance === 'string') {
      // @ts-ignore
      return this.entryStr(key);
    } else if (typeof instance === 'number') {
      // @ts-ignore
      return this.entryNum(key);
    } else if (Array.isArray(instance) && typeof instance[0] === 'number') {
      // @ts-ignore
      return this.entryNumArray(key);
    }

    throw new Error(`Invalid instance ${instance} ${typeof instance}=T`);
  }

  dump(): string[] {
    return [
      ...Object.keys(this.str_entries).map((key) => {
        return `${this.namespace_name}.${key} = ${this.str_entries[key].get()}`;
      }),
      ...Object.keys(this.num_entries).map((key) => {
        return `${this.namespace_name}.${key} = ${this.num_entries[key].get()}`;
      }),
      ...Object.keys(this.num_array_entries).map((key) => {
        return `${this.namespace_name}.${key} = ${this.num_array_entries[key].get()?.join(',')}`;
      })
    ]
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