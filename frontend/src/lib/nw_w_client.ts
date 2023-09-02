function fetch_wrapper(url: string, data: string | ArrayBuffer | undefined = undefined, method: string = "POST") {
  const data_size =
    typeof data === "string" ? data.length :
      data instanceof ArrayBuffer ? data.byteLength :
        -1;
  const data_size_str = data_size != -1 ? `(${data_size} bytes)` : "";

  console.log(`[Fetch] ${method} ${url} siz${data_size_str}`);

  return fetch(url, {
    method: method,
    body: data,
  });
}

export default class Client {
  constructor(private ip_addr: string) {
    this.ip_addr = ip_addr;
  }

  async S3reset() {
    const res = await fetch_wrapper(
      `http://${this.ip_addr}/api/stm32/reset`
    );
    if (res.ok) {
      console.log("[Client] STM32 Reset Success");
    } else {
      console.error("[Client] STM32 Reset Failed");
    }
  }
  async reset() {
    const res = await fetch_wrapper(
      `http://${this.ip_addr}/api/reset`
    );
    if (res.ok) {
      console.log("[Client] Reset Success");
    } else {
      console.error("[Client] Reset Failed");
    }
  }

  async BLEnter() {
    const res = await fetch_wrapper(
      `http://${this.ip_addr}/api/stm32/bootloader/boot`
    );
    if (res.ok) {
      console.log("[Client] Boot Success");
    } else {
      console.error("[Client] Boot Failed");
    }
  }

  async BLUpload(data: string | ArrayBuffer | undefined) {
    const res = await fetch_wrapper(
      `http://${this.ip_addr}/api/stm32/bootloader/upload`,
      data
    );
    if (res.ok) {
      console.log("[Client] Upload Success");
    } else {
      console.error("[Client] Upload Failed");
    }
  }

  async DumpNVS(): Promise<ArrayBuffer> {
    const res = await fetch_wrapper(`http://${this.ip_addr}/api/nvs/dump`);
    if (res.ok) {
      return await res.arrayBuffer();
    } else {
      throw new Error("[Client] NVS Dump Failed");
    }
  }
  async UploadNVS(ns: string, key: string, type: number, data: number[]) {
    const payload = [
      (ns.length >>> 24) & 0xff,
      (ns.length >>> 16) & 0xff,
      (ns.length >>> 8) & 0xff,
      ns.length & 0xff,
      ...[...ns].map(x => x.charCodeAt(0)),

      (key.length >>> 24) & 0xff,
      (key.length >>> 16) & 0xff,
      (key.length >>> 8) & 0xff,
      key.length & 0xff,
      ...[...key].map(x => x.charCodeAt(0)),

      type,

      ...data
    ];
    const buf = new ArrayBuffer(payload.length);
    const view = new Uint8Array(buf);
    view.set(payload);

    const res = await fetch_wrapper(
      `http://${this.ip_addr}/api/nvs/set`,
      buf
    );
    if (res.ok) {
      console.log("[Client] NVS Set Success");
    } else {
      console.error("[Client] NVS Set Failed");
    }
  }
}
