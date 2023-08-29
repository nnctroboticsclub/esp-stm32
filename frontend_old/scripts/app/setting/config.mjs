class RawConfigParser {
  /**
   * @param {number[]} data Buffer
   */
  constructor(data) {
    this.data = data;
  }

  /**
   * Read Uint8
   * @returns {number} A 8bit unsigned integer
   */
  ReadUint8() {
    const value = this.data[0];
    this.data = this.data.slice(1);
    return value;
  }

  /**
   * Read Uint16
   * @returns {number} A 16bit unsigned integer
   */
  ReadUint16() {
    const value = (this.data[0] << 8) | (this.data[1]);
    this.data = this.data.slice(2);
    return value;
  }

  /**
   * Read Uint32
   * @returns {number} A 32bit unsigned integer
   */
  ReadUint32() {
    const value = (this.data[0] << 24) | (this.data[1] << 16) | (this.data[2] << 8) | this.data[3];
    this.data = this.data.slice(4);
    return value >>> 0;
  }

  /**
   * Read Uint64
   * @returns {number} A 64bit unsigned integer
   */
  ReadUint64() {
    const value = (this.data[0] << 56) | (this.data[1] << 48) | (this.data[2] << 40) | (this.data[3] << 32) | (this.data[4] << 24) | (this.data[5] << 16) | (this.data[6] << 8) | this.data[7];
    this.data = this.data.slice(8);
    return value;
  }

  /**
   * Read Data
   * @param {number} len A Data length to read
   * @returns {number[]} A Data
   */
  ReadData(len) {
    const value = this.data.slice(0, len);
    this.data = this.data.slice(len);
    return value;
  }

  /**
   * Read String
   * @param {number} len A String length to read
   * @returns {string} A String
   */
  ReadString(len) {
    return String.fromCharCode.apply(null, this.ReadData(len)).replace(/\0/g, '');
  }

  /**
   * Check if data is available
   * @returns {boolean} True if data is available
   */
  DataAvailable() {
    return this.data.length > 0;
  }

}

class NVSNamespace {
  constructor() {
    /** @type {Object<string, string | number | number[]>} */
    this.entries = {}
  }

  /**
   * @param {string} key
   * @param {string | number | number[]} value
   */
  Set(key, value) {
    this.entries[key] = value;
  }

  /**
   * @param {string} key
   * @returns {string | number | number[]} value
   */
  Get(key) {
    return this.entries[key];
  }
}

class NVS {
  constructor() {
    /** @type {Object.<string, NVSNamespace>} */
    this.table = {}
  }

  /**
   * @param {string} namespace
   * @param {string} key
   * @param {number | string | number[]} value
   */
  Set(namespace, key, value) {
    if (!(namespace in this.table)) {
      this.table[namespace] = new NVSNamespace();
    }
    this.table[namespace].Set(key, value);
  }

  /**
   * Get value
   * @param {string} namespace
   * @param {string} key
   * @returns {number | string | number[]} value
   */
  Get(namespace, key) {
    return this.table[namespace].Get(key);
  }

  /**
   * Get namespace
   * @param {string} namespace
   * @returns {NVSNamespace} namespace
   */
  GetNS(namespace) {
    return this.table[namespace];
  }

  /**
   * @param {number[]} data
   */
  static fromDump(data) {
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
}

class SPI {
  /**
   * @param {NVSNamespace} ns
   */
  constructor(ns) {
    /** @type {number} */
    this.port = ns.Get("port");
    /** @type {number} */
    this.miso = ns.Get("miso");
    /** @type {number} */
    this.mosi = ns.Get("mosi");
    /** @type {number} */
    this.sclk = ns.Get("sclk");
  }

  toString() {
    return `SPI${this.port} (MISO: ${this.miso}, MOSI: ${this.mosi}, SCLK: ${this.sclk})`;
  }
}

class Bus {
  /**
   * @param {NVSNamespace} ns
   */
  constructor(ns) {
    /** @type {number} */
    this.type = ns.Get("bus_type");
    /** @type {number} */
    this.port = ns.Get("bus_port");
    /** @type {number} */
    this.cs = ns.Get("cs");
  }

  toString() {
    return `Bus[${this.port}[${this.type}] cs: ${this.cs}]`;
  }

}

class STM32Bootloader {
  /**
   * @param {NVSNamespace} ns
   */
  constructor(ns) {
    /** @type {number} */
    this.id = ns.Get("id");
    /** @type {Bus} */
    this.bus = new Bus(ns);
  }

  toString() {
    return `STM32BL[${this.id}] ${this.bus}`;
  }
}

class STM32 {
  /**
   * @param {NVSNamespace} ns
   */
  constructor(ns) {
    /** @type {number} */
    this.id = ns.Get("id");
    /** @type {number} */
    this.reset = ns.Get("reset");
    /** @type {number} */
    this.boot0 = ns.Get("boot0");
    /** @type {number} */
    this.bid = ns.Get("bid");
    /** @type {number} */
    this.rid = ns.Get("rid");

  }

  toString() {
    return `STM32[${this.id}] (reset: ${this.reset}, boot0: ${this.boot0}, bid: ${this.bid}, rid: ${this.rid})`;
  }
}

class Network {
  /**
   * @param {NVSNamespace} ns
   */
  constructor(ns) {
    this.id = ns.Get("id");
    this.mode = ns.Get("mode");
    this.ip_mode = ns.Get("ip_mode");
    this.ssid = ns.Get("ssid");
    this.password = ns.Get("password");
    this.hostname = ns.Get("hostname");
    this.ip = ns.Get("ip");
    this.subnet = ns.Get("subnet");
    this.gateway = ns.Get("gateway");
  }

  toString() {
    return `Network[${this.id}] (mode: ${this.mode}, ip_mode: ${this.ip_mode}, auth: ${this.ssid}:${this.password}, hostname: ${this.hostname}, ip: ${this.ip}, subnet: ${this.subnet}, gateway: ${this.gateway})`;
  }

  /**
   * @param {HTMLElement} element
   */
  render(element) {
    const id_element = [...element.children].find((x) => x.getAttribute("name") == "id");
    const mode_element = [...element.children].find((x) => x.getAttribute("name") == "mode");
    const ssid_element = [...element.children].find((x) => x.getAttribute("name") == "ssid");
    const password_element = [...element.children].find((x) => x.getAttribute("name") == "password");
    const hostname_element = [...element.children].find((x) => x.getAttribute("name") == "hostname");
    const ip_element = [...element.children].find((x) => x.getAttribute("name") == "ip");
    const subnet_element = [...element.children].find((x) => x.getAttribute("name") == "subnet");
    const gw_element = [...element.children].find((x) => x.getAttribute("name") == "gw");

    id_element.getElementsByTagName("textarea")[0].value = this.id;
    // mode_element.getElementsByTagName("textarea")[0].value = this.mode;
    ssid_element.getElementsByTagName("textarea")[0].value = this.ssid;
    password_element.getElementsByTagName("textarea")[0].value = this.password;
    hostname_element.getElementsByTagName("textarea")[0].value = this.hostname;
    ip_element.getElementsByTagName("textarea")[0].value = this.ip;
    subnet_element.getElementsByTagName("textarea")[0].value = this.subnet;
    gw_element.getElementsByTagName("textarea")[0].value = this.gateway;

  }
}


export default class Config {
  /**
     * @param {Number[]} data
     * @returns {Config};
     */
  constructor(data) {
    const nvs = NVS.fromDump(data);
    console.log("Keys", JSON.stringify(Object.keys(nvs.table)));
    console.log("NVS", JSON.stringify(nvs.GetNS("mas").entries));

    /** @type {NVS} */
    this.nvs = nvs;

    const master = nvs.GetNS("mas");

    /** @type {SPI[]} */
    this.spi_buses = [...Array(master.Get("cds")).keys()].map((i) => new SPI(nvs.GetNS(`a_cs${i + 1}`)));

    /** @type {STM32Bootloader[]} */
    this.bootloaders = [...Array(master.Get("csb")).keys()].map((i) => new STM32Bootloader(nvs.GetNS(`a_sb${i + 1}`)));

    /** @type {STM32[]} */
    this.stm32_list = [...Array(master.Get("cs3")).keys()].map((i) => new STM32(nvs.GetNS(`a_s3${i + 1}`)));

    /** @type {Network[]} */
    this.network_list = [...Array(master.Get("cn")).keys()].map((i) => new Network(nvs.GetNS(`a_nw${i + 1}`)));
  }

  /**
   *
   * @param {number} id A ID of network object
   * @param {HTMLElement} element A HTMLElement to render
   */
  render_network(id, element) {
    const network = this.network_list.find((network) => network.id == id);
    if (!network) {
      console.log(this);
      throw new Error(`Network ${id} not found`);
    }
    network.render(element);
  }
}