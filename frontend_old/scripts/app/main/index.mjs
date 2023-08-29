import { log_write } from "../../libs/logger.mjs";


const ip_addr = document.getElementById("ip_addr");
const file_input = document.getElementById("file_selector");
const file_name = document.querySelector("#app > .file_name > .name");

/**
 * fetch 関数のラッパーです
 * ログにトランザクションを追加するプロキシです。
 * @param {string} url
 * @param {string?|ArrayBuffer?} data
 * @returns {Promise<Response>}
 */
function fetch_wrapper(url, data) {
  const data_size =
    !isNaN(data) && (!isNaN(data.byteLength) || !isNaN(data.length))
      ? data.byteLength || data.length
      : -1;
  const data_size_str = data_size != -1 ? `(${data_size} bytes)` : "";

  log_write(`[ Fetch] ${url} ${data_size_str} [${data_size}]`);

  return fetch(url, {
    method: "POST",
    body: data,
  });
}

/**
 * @class Client
 * @property {string} ip_addr
 * @method {void} reset
 * @method {void} bl_enter
 * @method {void} bl_upload
 *
 * @constructor
 * @param {string} ip_addr
 * @returns {Client}
 */
class Client {
  constructor(ip_addr) {
    this.ip_addr = ip_addr;
  }

  async reset() {
    const res = await fetch_wrapper(
      `http://${this.ip_addr}/api/stm32/reset`
    );
    if (res.ok) {
      log_write("[Client] Reset Success");
    } else {
      log_write("[Client] Reset Failed");
    }
  }

  async bl_enter() {
    const res = await fetch_wrapper(
      `http://${this.ip_addr}/api/stm32/bootloader/boot`
    );
    if (res.ok) {
      log_write("[Client] Boot Success");
    } else {
      log_write("[Client] Boot Failed");
    }
  }

  async bl_upload(data) {
    const res = await fetch_wrapper(
      `http://${this.ip_addr}/api/stm32/bootloader/upload`,
      data
    );
    if (res.ok) {
      log_write("[Client] Upload Success");
    } else {
      log_write("[Client] Upload Failed");
    }
  }
}

file_input.addEventListener("change", (e) => {
  const file = e.target.files[0];
  file_name.innerHTML = file.name;
  const reader = new FileReader();
  reader.onload = async (e) => {
    const data = e.target.result;
    const client = new Client(ip_addr.value);

    await client.bl_enter();
    await client.bl_upload(data);
    await client.reset();
  };
  reader.readAsArrayBuffer(file);
});