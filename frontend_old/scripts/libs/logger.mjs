const log = document.getElementById("log");

/**
 * #log にテキストを追加します
 * @param {string} ログに追記する文字列
 * @returns {void}
 */
export function log_write(text) {
  log.innerHTML += text + "<br />";
  log.scrollTop = log.scrollHeight;
}

export function patchConsoleLog() {
  function factory(orig, name) {
    function my_stringify(x) {
      if (x.toString && x.toString() !== "[object Object]") {
        return x.toString();
      }

      if (typeof x === "string") {
        return x;
      } else if (typeof x === "number") {
        return x.toString();
      } else if (typeof x === "boolean") {
        return x.toString();
      } else if (typeof x === "undefined") {
        return "undefined";
      } else if (typeof x === "function") {
        return "<function>";
      }

      if (Array.isArray(x)) {
        return "[" + x.map(my_stringify).join(", ") + "]";
      }

      return JSON.stringify(x);
    }

    return function () {
      const args = Array.from(arguments);
      const buf = "[" + name + "] " + args.map(my_stringify).join(" ");
      log_write(buf);
      orig.apply(console, args);
    }
  }

  console.log = factory(console.log, "Console::log");
  console.error = factory(console.error, "Console::error");
  console.warn = factory(console.warn, "Console::warn");
  console.info = factory(console.info, "Console::info");

  console.log("Patched console logging functions.");
}