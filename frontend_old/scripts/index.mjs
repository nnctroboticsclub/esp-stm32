import { patchConsoleLog } from "./libs/logger.mjs";
patchConsoleLog();

import("./app/main/index.mjs");
import("./app/setting/index.mjs");
