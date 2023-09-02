import { RawConfigParser } from './raw_config_parser';
import { NVSNamespace } from './namespace';
import type { NVSType, NVSTypeInstance } from './nvs_entry_type';


export class NVS {
  table: { [key: string]: NVSNamespace }

  constructor() {
    this.table = {}
  }

  Set<T extends NVSType>(namespace: string, key: string, value: NVSTypeInstance<T>, type: T): NVSTypeInstance<T> {
    if (!(namespace in this.table)) {
      this.table[namespace] = new NVSNamespace(namespace);
    }
    const ns = this.table[namespace];

    return ns.entry(key, type).set(value);
  }

  Get<T extends NVSType>(namespace: string, key: string, type: T): NVSTypeInstance<T> | undefined {
    return this.table[namespace].entry(key, type).get();
  }

  GetNS(namespace: string): NVSNamespace {
    const ns = this.table[namespace];
    if (!ns) {
      throw new Error(`Namespace ${namespace} not found`);
    }
    return ns;
  }

  clone(): NVS {
    const ret = new NVS();

    for (const key in this.table) {
      ret.table[key] = new NVSNamespace(this.table[key]);
    }

    return ret;
  }

  static fromDump(data: number[]) {
    const reader = new RawConfigParser(data);
    const nvs = new NVS();
    while (reader.DataAvailable()) {
      const namespace_len = reader.ReadUint8();
      const key_len = reader.ReadUint8();
      const type = reader.ReadUint8();

      const namespace = reader.ReadString(namespace_len);
      const key = reader.ReadString(key_len);

      const entry_type = type as NVSType;

      if ((type >> 4) == 0x0 || (type >> 4) == 0x1) {
        const bits = 8 * (type & 0x0f);
        let value = 0;
        switch (bits) {
          case 8:
            value = reader.ReadUint8();
            break;
          case 16:
            value = reader.ReadUint16();
            break;
          case 32:
            value = reader.ReadUint32();
            break;
          case 64:
            value = reader.ReadUint64();
            break;
          default:
            console.log(bits, type);
            throw new Error("Invalid type");
        }

        nvs.Set(namespace, key, value, entry_type);
      } else if (type == 0x21) {
        const length = reader.ReadUint32();
        const str = reader.ReadString(length);
        nvs.Set(namespace, key, str, entry_type);
      } else if (type == 0x42) {
        const length = reader.ReadUint32();
        const buffer = reader.ReadData(length);
        nvs.Set(namespace, key, buffer, entry_type);
      } else {
        console.log(type);
        throw new Error('Unknown type');
      }
    }
    return nvs;

  }

  dump() {
    return Object.keys(this.table).map(ns =>
      Object.keys(this.table[ns].entries).map(t =>
        Object.keys(this.table[ns].entries[t as unknown as NVSType]).map(key => ({
          ns,
          key,
          type: parseInt(t) as NVSType,
          value: this.table[ns].entries[t as unknown as NVSType][key].get()
        }))
      ).flat()
    ).flat();
  }
}