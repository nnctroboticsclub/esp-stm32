import { log_write } from "../../libs/logger.mjs";


const ip_addr = document.getElementById("ip_addr");
const file_input = document.getElementById("file_selector");
const file_name = document.querySelector("#app > .file_name > .name");

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