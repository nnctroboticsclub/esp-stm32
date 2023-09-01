function fetch_wrapper(url: string, data: string|ArrayBuffer, method: string="POST") {
  const data_size =
    !isNaN(data) && (!isNaN(data.byteLength) || !isNaN(data.length))
      ? data.byteLength || data.length
      : -1;
  const data_size_str = data_size != -1 ? `(${data_size} bytes)` : "";

  console.log(`V: [Fetch] ${url} ${data_size_str} [${data_size}]`);

  return fetch(url, {
    method: "POST",
    body: data,
  });
}

export default class Client {
  ip_addr: string
  constructor(ip_addr) {
    this.ip_addr = ip_addr;
  }

  async reset() {
    const res = await fetch_wrapper(
      `http://${this.ip_addr}/api/stm32/reset`
    );
    if (res.ok) {
      log_write("I: [Client] NW-W Reset Success");
    } else {
      log_write("E: [Client] Reset Failed");
    }
  }

  async BLEnter() {
    const res = await fetch_wrapper(
      `http://${this.ip_addr}/api/stm32/bootloader/boot`
    );
    if (res.ok) {
      log_write("I: [Client] Boot Success");
    } else {
      log_write("E: [Client] Boot Failed");
    }
  }

  async BLUpload(data) {
    const res = await fetch_wrapper(
      `http://${this.ip_addr}/api/stm32/bootloader/upload`,
      data
    );
    if (res.ok) {
      log_write("I: [Client] Upload Success");
    } else {
      log_write("E: [Client] Upload Failed");
    }
  }
}
