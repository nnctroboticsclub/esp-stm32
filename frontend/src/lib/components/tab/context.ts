import { getContext, hasContext, setContext } from "svelte";
import { writable, type Writable } from "svelte/store";

class Tab {
  title: string;
  getSecondaryBarTitles: () => string[];

  constructor(title: string, getSecondaryBarTitles: () => string[]) {
    this.title = title;
    this.getSecondaryBarTitles = getSecondaryBarTitles;
  }
}


export function getActiveTab(): Writable<string> {
  const has_context = hasContext("tab-active")
  if (has_context) {
    return getContext("tab-active");
  }
  const store = writable<string>("");
  setContext("tab-active", store);

  return store;
}

export function getActiveSecondaryTab(): Writable<string> {
  const has_context = hasContext("tab-secondary-active")
  if (has_context) {
    return getContext("tab-secondary-active");
  }
  const store = writable<string>("");
  setContext("tab-secondary-active", store);

  return store;
}

export function getTabs(): Writable<Tab[]> {
  const has_context = hasContext("tab-tabs")
  if (has_context) {
    return getContext("tab-tabs");
  }

  const store = writable([]);
  setContext("tab-tabs", store);

  return store;
}