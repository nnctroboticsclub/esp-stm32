function fetch_wrapper(url: string, data: string | ArrayBuffer | undefined = undefined, method: string = "POST") {
  const data_size =
    typeof data === "string" ? data.length :
      data instanceof ArrayBuffer ? data.byteLength :
        -1;
  const data_size_str = data_size != -1 ? `(${data_size} bytes)` : "";

  console.log(`[Fetch] ${method} ${url} size: ${data_size_str}`);

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
      console.log("I: [Client] NW-W Reset Success");
    } else {
      console.error("E: [Client] Reset Failed");
    }
  }

  async BLEnter() {
    const res = await fetch_wrapper(
      `http://${this.ip_addr}/api/stm32/bootloader/boot`
    );
    if (res.ok) {
      console.log("I: [Client] Boot Success");
    } else {
      console.error("E: [Client] Boot Failed");
    }
  }

  async BLUpload(data: string | ArrayBuffer | undefined) {
    const res = await fetch_wrapper(
      `http://${this.ip_addr}/api/stm32/bootloader/upload`,
      data
    );
    if (res.ok) {
      console.log("I: [Client] Upload Success");
    } else {
      console.error("E: [Client] Upload Failed");
    }
  }

  async DumpNVS(): Promise<ArrayBuffer> {
    const res = await fetch_wrapper(`http://${this.ip_addr}/api/nvs/dump`);
    if (res.ok) {
      return await res.arrayBuffer();
    } else {
      throw new Error("E: [Client] NVS Dump Failed");
    }

  }
}
