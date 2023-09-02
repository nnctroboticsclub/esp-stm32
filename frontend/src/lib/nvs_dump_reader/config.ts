import type { NVSNamespace, NVSEntry } from "./namespace";
import { NVS } from "./nvs";

class SPI {
  id: NVSEntry<number>;
  miso: NVSEntry<number>;
  mosi: NVSEntry<number>;
  sclk: NVSEntry<number>;

  constructor(ns: NVSNamespace) {
    this.id = ns.entry<number>("port", 0);
    this.miso = ns.entry<number>("miso", 0);
    this.mosi = ns.entry<number>("mosi", 0);
    this.sclk = ns.entry<number>("sclk", 0);
  }

  toString(): string {
    return `SPI${this.id} (MISO: ${this.miso}, MOSI: ${this.mosi}, SCLK: ${this.sclk})`;
  }
}

class UART {
  id: NVSEntry<number>;
  tx: NVSEntry<number>;
  rx: NVSEntry<number>;
  baud_rate: NVSEntry<number>;
  parity: NVSEntry<number>;

  constructor(ns: NVSNamespace) {
    this.id = ns.entry<number>("port", 0);
    this.tx = ns.entry<number>("tx", 0);
    this.rx = ns.entry<number>("rx", 0);
    this.baud_rate = ns.entry<number>("rate", 0);
    this.parity = ns.entry<number>("prt", 0);
  }

  toString(): string {
    return `UART${this.id} (->${this.tx} <-${this.rx} spd:${this.baud_rate} prt:${this.parity})`;
  }
}

class Bus {
  bus_type: NVSEntry<number>;
  bus_port: NVSEntry<number>;
  cs: NVSEntry<number>;

  constructor(ns: NVSNamespace) {
    this.bus_type = ns.entry<number>("bus_type", 0);
    this.bus_port = ns.entry<number>("bus_port", 0);
    this.cs = ns.entry<number>("cs", 0);
  }

  toString(): string {
    return `Bus[${this.bus_port}[${this.bus_type}] cs: ${this.cs}]`;
  }
}

class STM32Bootloader extends Bus {
  id: NVSEntry<number>;

  constructor(ns: NVSNamespace) {
    super(ns)

    this.id = ns.entry<number>("id", 0);
  }

  toString(): string {
    return `STM32BL[${this.id}] on ${this.bus_port}[${this.bus_type}] cs: ${this.cs}`;
  }
}

class STM32 {
  id: NVSEntry<number>;
  reset: NVSEntry<number>;
  boot0: NVSEntry<number>;
  bid: NVSEntry<number>;
  rid: NVSEntry<number>;

  constructor(ns: NVSNamespace) {
    this.id = ns.entry<number>("id", 0);
    this.reset = ns.entry<number>("reset", 0);
    this.boot0 = ns.entry<number>("boot0", 0);
    this.bid = ns.entry<number>("bid", 0);
    this.rid = ns.entry<number>("rid", 0);
  }

  toString(): string {
    return `STM32[${this.id}] (reset: ${this.reset}, boot0: ${this.boot0}, bid: ${this.bid}, rid: ${this.rid})`;
  }
}

class Network {
  id: NVSEntry<number>;
  mode: NVSEntry<number>;
  ip_mode: NVSEntry<number>;
  ssid: NVSEntry<string>;
  password: NVSEntry<string>;
  hostname: NVSEntry<string>;
  ip: NVSEntry<number>; // TODO(syoch): Change this to IP Object
  subnet: NVSEntry<number>;
  gateway: NVSEntry<number>;

  constructor(ns: NVSNamespace) {
    this.id = ns.entry<number>("id", 0);
    this.mode = ns.entry<number>("mode", 0);
    this.ip_mode = ns.entry<number>("ip_mode", 0);
    this.ssid = ns.entry<string>("ssid", "");
    this.password = ns.entry<string>("password", "");
    this.hostname = ns.entry<string>("hostname", "");
    this.ip = ns.entry<number>("ip", 0);
    this.subnet = ns.entry<number>("subnet", 0);
    this.gateway = ns.entry<number>("gateway", 0);
  }

  toString(): string {
    return `Network[${this.id}] (mode: ${this.mode}, ip_mode: ${this.ip_mode}, auth: ${this.ssid}:${this.password}, hostname: ${this.hostname}, ip: ${this.ip}, subnet: ${this.subnet}, gateway: ${this.gateway})`;
  }
}

class Master {
  cds: NVSEntry<number>;
  cdu: NVSEntry<number>;
  csb: NVSEntry<number>;
  cs3: NVSEntry<number>;
  cn: NVSEntry<number>;

  constructor(ns: NVSNamespace) {
    this.cds = ns.entry<number>("cds", 0);
    this.cdu = ns.entry<number>("cdu", 0);
    this.csb = ns.entry<number>("csb", 0);
    this.cs3 = ns.entry<number>("cs3", 0);
    this.cn = ns.entry<number>("cn", 0);
  }

  toString(): string {
    return `Master (cds: ${this.cds}, cdu: ${this.cdu}, csb: ${this.csb}, cs3: ${this.cs3}, cn: ${this.cn})`;
  }
}


export class Config {
  nvs: NVS;

  master: Master;
  spi_buses: SPI[];
  uart_ports: UART[];
  bootloaders: STM32Bootloader[];
  stm32_list: STM32[];
  network_list: Network[];


  constructor(nvs: NVS | number[]) {
    if (nvs instanceof NVS) {
      this.nvs = nvs;
    } else {
      this.nvs = NVS.fromDump(nvs);
    }

    this.master = new Master(this.nvs.GetNS("mas"));

    this.spi_buses = [...Array(this.master.cds.get() ?? 0).keys()]
      .map((i) => new SPI(this.nvs.GetNS(`a_cs${i + 1}`)));

    this.uart_ports = [...Array(this.master.cdu.get() ?? 0).keys()]
      .map((i) => new UART(this.nvs.GetNS(`a_cu${i + 1}`)));

    this.bootloaders = [...Array(this.master.csb.get() ?? 0).keys()]
      .map((i) => new STM32Bootloader(this.nvs.GetNS(`a_sb${i + 1}`)));

    this.stm32_list = [...Array(this.master.cs3.get() ?? 0).keys()]
      .map((i) => new STM32(this.nvs.GetNS(`a_s3${i + 1}`)));

    this.network_list = [...Array(this.master.cn.get() ?? 0).keys()]
      .map((i) => new Network(this.nvs.GetNS(`a_nw${i + 1}`)));

  }

  clone(): Config {
    return new Config(this.nvs.clone());
  }
}