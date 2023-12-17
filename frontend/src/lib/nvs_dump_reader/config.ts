import type { NVSNamespace, NVSEntry } from "./namespace";
import { NVS } from "./nvs";
import { NVSType } from "./nvs_entry_type";

class SPI {
  id: NVSEntry<number>;
  miso: NVSEntry<number>;
  mosi: NVSEntry<number>;
  sclk: NVSEntry<number>;

  constructor(ns: NVSNamespace) {
    this.id = ns.entry("port", NVSType.U8);
    this.miso = ns.entry("miso", NVSType.U8);
    this.mosi = ns.entry("mosi", NVSType.U8);
    this.sclk = ns.entry("sclk", NVSType.U8);
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
    this.id = ns.entry("port", NVSType.U8);
    this.tx = ns.entry("tx", NVSType.U8);
    this.rx = ns.entry("rx", NVSType.U8);
    this.baud_rate = ns.entry("rate", NVSType.U32);
    this.parity = ns.entry("prt", NVSType.U8);
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
    this.bus_type = ns.entry("bus_type", NVSType.U8);
    this.bus_port = ns.entry("bus_port", NVSType.U8);
    this.cs = ns.entry("cs", NVSType.U8);
  }

  toString(): string {
    return `Bus[${this.bus_port}[${this.bus_type}] cs: ${this.cs}]`;
  }
}

class STM32Bootloader extends Bus {
  id: NVSEntry<number>;

  constructor(ns: NVSNamespace) {
    super(ns)

    this.id = ns.entry("id", NVSType.U8);
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
    this.id = ns.entry("id", NVSType.U8);
    this.reset = ns.entry("reset", NVSType.U8);
    this.boot0 = ns.entry("boot0", NVSType.U8);
    this.bid = ns.entry("bid", NVSType.U8);
    this.rid = ns.entry("rid", NVSType.U8);
  }

  toString(): string {
    return `STM32[${this.id}] (reset: ${this.reset}, boot0: ${this.boot0}, bid: ${this.bid}, rid: ${this.rid})`;
  }
}
class SerialProxy {
  id: NVSEntry<number>;
  port: NVSEntry<number>;

  constructor(ns: NVSNamespace) {
    this.id = ns.entry("id", NVSType.U8);
    this.port = ns.entry("uart", NVSType.U8);
  }

  toString(): string {
    return `SerialProxy[${this.port}]`;
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
    this.id = ns.entry("id", NVSType.U8);
    this.mode = ns.entry("mode", NVSType.U8);
    this.ip_mode = ns.entry("ip_mode", NVSType.U8);
    this.ssid = ns.entry("ssid", NVSType.STR);
    this.password = ns.entry("password", NVSType.STR);
    this.hostname = ns.entry("hostname", NVSType.STR);
    this.ip = ns.entry("ip", NVSType.U32);
    this.subnet = ns.entry("subnet", NVSType.U32);
    this.gateway = ns.entry("gateway", NVSType.U32);
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
  csp: NVSEntry<number>;

  constructor(ns: NVSNamespace) {
    this.cds = ns.entry("cds", NVSType.U8);
    this.cdu = ns.entry("cdu", NVSType.U8);
    this.csb = ns.entry("csb", NVSType.U8);
    this.cs3 = ns.entry("cs3", NVSType.U8);
    this.cn = ns.entry("cn", NVSType.U8);
    this.csp = ns.entry("csp", NVSType.U8);
  }

  toString(): string {
    return `Master (cds: ${this.cds}, cdu: ${this.cdu}, csb: ${this.csb}, cs3: ${this.cs3}, cn: ${this.cn}, csp: ${this.csp})`;
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
  serial_proxy_list: SerialProxy[];


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


    this.serial_proxy_list = [...Array(this.master.csp.get() ?? 0).keys()]
      .map((i) => new SerialProxy(this.nvs.GetNS(`a_sp${i + 1}`)));

  }

  clone(): Config {
    return new Config(this.nvs.clone());
  }
}