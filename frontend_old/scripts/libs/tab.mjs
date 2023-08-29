export class Select {
  /**
   * @constructor
   * @param {HTMLElement[]} elements
   * @param {(e: Event) => void} activate_cb
   */
  constructor(elements, activate_cb) {
    this.elements = [...elements];
    this.activate_cb = activate_cb;
  }

  /**
   * @method
   * Register click handler
   */
  registerClickHandler() {
    const self = this;
    this.elements.forEach((element) => {
      element.addEventListener("click", (e) => {
        self.clickHandler(e);
      });
    });
  }

  /**
   * @method
   * Click handler for each elements
   * @param {Event} e
   */
  clickHandler(e) {
    this.activate(e.target.getAttribute("name"));

    if (this.activate_cb) {
      this.activate_cb(e);
    }
  }

  /**
   * @method
   * @param {string} name
   */
  activate(name) {
    this.elements.forEach((element) => {
      if (element.getAttribute("name") == name) {
        element.classList.add("active");
      } else {
        element.classList.remove("active");
      }
    });

  }

  get active_content() {
    return this.elements.find((element) => {
      return element.classList.contains("active");
    });
  }

  get active() {
    return this.active_content.getAttribute("name");
  }
}

export default class TabLayout {
  /**
   * @param {HTMLElement} element
   * @param {() => void} activate_cb
   */
  constructor(element, activate_cb) {
    const appbar = element.querySelector(".appbar");

    /** @type {Select} */
    this.contents = new Select(element.querySelectorAll(".tab-content"));

    /** @type {Select} */
    this.tabs = new Select(appbar.querySelectorAll(".tab"), (e) => {
      const name = e.target.getAttribute("name");
      this.contents.activate(name);
      if (activate_cb) {
        activate_cb();
      }
    });

    this.tabs.registerClickHandler();

    const secondary_bar = element.querySelector(".secondary-bar");
    if (secondary_bar) {
      this.secondary_tabs = new Select(secondary_bar.querySelectorAll(".tab"), (_e) => { activate_cb(); });
      this.secondary_tabs.registerClickHandler();
    }

    setTimeout(() => {
      activate_cb();
    }, 100);
  }

  /**
   * @method
   * @param {string} name
   */
  activate(name) {
    this.tabs.activate(name);
  }

  /**
   * @method
   * @param {string} name
   */
  activateSecondary(name) {
    this.secondary_tabs.activate(name);
  }

  get active() {
    return this.tabs.active;
  }

  get secondary_active() {
    return this.secondary_tabs.active;
  }
  get active_content() {
    return this.contents.active_content;
  }
}