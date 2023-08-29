import { IP, ID, Select, String, ConfigGroup } from "../../libs/config-ptn.mjs";
import TabLayout from "../../libs/tab.mjs";
import Config from "./config.mjs";
import test_nvs from "./test_nvs.mjs";

const saved_config = new Config(test_nvs);

const element = document.getElementById("setting_modal");

const config_view = new ConfigGroup("network", document.getElementById("TEST"), [
  ["ID", ID],
  ["Mode", Select, ["AP", "STA"]],
  ["Password", String],
  ["Hostname", String],
  ["SSID", IP],
  ["Subnet", IP],
  ["Gateway", IP]
]);


const setting_modal = new TabLayout(element, (e) => {
  const active = setting_modal.active;
  const secondary_active = parseInt(setting_modal.secondary_active);
  const content = setting_modal.active_content;

  switch (active) {
    case "stm32":
      saved_config.stm32
      break;
    case "stm32bl":

      break;
    case "network":
      saved_config.render_network(secondary_active, content);
      break;
    case "spi":

      break;
    case "uart":

      break;
  }
});