import { Select as SelectElements } from "./tab.mjs";

export class ConfigOption {
  /**
   * @param {string} name A option name
   * @param {HTMLElement} element A element to render on
   */
  constructor(name, element) {
    this.name = name;

    this.element = element;
    this.element.setAttribute("name", name);

    [...this.element.children].forEach((x) => {
      this.element.removeChild(x);
    });


    const title = document.createElement("span");
    title.classList.add("title");
    title.innerText = name;

    this.element.appendChild(title);
  }
}

export class IP extends ConfigOption {
  /**
   * @param {string} name A option name
   * @param {HTMLElement} element A element to render on
   */
  constructor(name, element) {
    super(name, element);
    element.classList.add("ip");

    /** @type {HTMLInputElement} */
    this.octets = [];

    [...Array(4).keys()].forEach((_x, i) => {
      if (i != 0) {
        let dot = document.createElement("span");
        dot.classList.add("dot");
        dot.innerText = ".";
        this.element.append(dot);
      }
      let octet = document.createElement("input");
      octet.type = "number";
      octet.min = 0;
      octet.max = 255;
      octet.value = 0;
      this.element.append(octet);
      this.octets.push(octet);
    });
  }

  /**
   * @param {number} ip
   */
  load(ip) {
    const octets = this.octets;

    octets[0].value = (ip >> 24) & 0xff;
    octets[1].value = (ip >> 16) & 0xff;
    octets[2].value = (ip >> 8) & 0xff;
    octets[3].value = ip & 0xff;
  }

  /**
   * @returns {number}
   */
  get value() {
    const octets = this.octets;

    return (octets[0].value << 24) | (octets[1].value << 16) | (octets[2].value << 8) | octets[3].value;
  }

  /**
   * @returns {string}
   */
  toString() {
    const octets = this.octets;

    return `Config<${octets[0].value}.${octets[1].value}.${octets[2].value}.${octets[3].value} on ${this.name}>`;
  }
}
export class ID extends ConfigOption {
  /**
   * @param {string} name A option name
   * @param {HTMLElement} element A element to render on
   */
  constructor(name, element) {
    super(name, element);
    element.classList.add("id");

    /** @type {HTMLInputElement} */
    this.input = [];

    let number = document.createElement("input");
    number.type = "number";
    number.min = 0;
    number.value = 0;
    this.element.append(number);

  }

  /**
   * @param {number} val
   */
  set value(val) {
    this.number.value = val;
  }

  /**
   * @returns {number}
   */
  get value() {
    return this.number.value;
  }

  /**
   * @returns {string}
   */
  toString() {
    return `Config<${this.value} on ${this.name}>`;
  }
}
export class String extends ConfigOption {
  /**
   * @param {string} name A option name
   * @param {HTMLElement} element A element to render on
   */
  constructor(name, element) {
    super(name, element);
    element.classList.add("string");

    /** @type {HTMLInputElement} */
    this.input = document.createElement("input");
    this.input.type = "text";
    this.input.value = "";
    this.element.append(this.input);
  }

  /**
   * @param {string} val
   */
  set value(val) {
    this.string.value = val;
  }

  /**
   * @returns {string}
   */
  get value() {
    return this.string.value;
  }

  /**
   * @returns {string}
   */
  toString() {
    return `Config<${this.value} on ${this.name}>`;
  }
}
export class Select extends ConfigOption {
  /**
   * @param {string} name A option name
   * @param {HTMLElement} element A element to render on
   * @param {string[]} options A list of options
   */
  constructor(name, element, options) {
    super(name, element);
    element.classList.add("select");

    this.options = [...Array(options.length).keys()].map((x) => {
      /** @type {HTMLInputElement} */
      const elem = document.createElement("div");
      elem.classList.add("select");
      elem.setAttribute("name", options[x]);
      elem.setAttribute("x-value", x);
      elem.innerText = options[x];
      this.element.append(elem);
      return elem;
    });

    this.select = new SelectElements(this.options);
    this.select.registerClickHandler();
  }

  /**
   * @returns {string}
   */
  get value_str() {
    return this.select.active_content.innerText;
  }

  /**
   * @returns {number}
   */
  get value() {
    return parseInt(this.select.active_content.getAttribute("x-value"));
  }

  /**
   * @param {number} val
   */
  set value(val) {
    this.select.activate(val);
  }
}



export class ConfigGroup {
  /**
   * @param {String} name A Group name
   * @param {HTMLElement} element A element to render on
   * @param {Array<(string, Function, ...)>} options A list of options
   */
  constructor(name, element, options) {
    this.name = name; this.element = element;

    this.element.classList.add("setting");
    this.element.setAttribute("name", name);

    /** @type {Object<String, ConfigOption>} */
    this.variables = {};

    options.forEach((x) => {
      const [name, T, ...args] = x;

      const element = document.createElement("div");
      this.variables[name] = new T(name, element, ...args);
      this.element.append(element);
    })
  }

}