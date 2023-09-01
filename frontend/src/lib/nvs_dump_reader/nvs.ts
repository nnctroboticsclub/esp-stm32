import { RawConfigParser } from './raw_config_parser';
import { NVSNamespace } from './namespace';



export class NVS {
  table: { [key: string]: NVSNamespace }

  constructor() {
    this.table = {}
  }

  Set<T extends string | number | number[]>(namespace: string, key: string, value: T): T {
    if (!(namespace in this.table)) {
      this.table[namespace] = new NVSNamespace(namespace);
    }
    const ns = this.table[namespace];

    if (typeof value === 'string') {
      return ns.entryStr(key).set(value) as T;
    } else if (typeof value === 'number') {
      return ns.entryNum(key).set(value) as T;
    } else if (Array.isArray(value) && typeof value[0] === 'number') {
      return ns.entryNumArray(key).set(value) as T;
    }


    return value;
  }

  Get<T extends string | number | number[]>(namespace: string, key: string, instance: T): T | undefined {
    return this.table[namespace].entry(key, instance).get();
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
        nvs.Set(namespace, key, value);
      } else if (type == 0x21) {
        const length = reader.ReadUint32();
        const str = reader.ReadString(length);
        nvs.Set(namespace, key, str);
      } else if (type == 0x42) {
        const length = reader.ReadUint32();
        const buffer = reader.ReadData(length);
        nvs.Set(namespace, key, buffer);
      } else {
        console.log(type);
        throw new Error('Unknown type');
      }
    }
    return nvs;

  }

  dumpScript(): string[] {
    return Object.keys(this.table).map((key) => this.table[key].
      dump()).flat();
  }
}