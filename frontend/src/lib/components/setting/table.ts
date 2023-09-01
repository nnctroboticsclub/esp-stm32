import type { NVSEntry, NVSNamespace } from "$lib/nvs_dump_reader/namespace";
import { getContext, hasContext, setContext } from "svelte";
import { writable, type Writable } from "svelte/store";


export type Table = NVSNamespace;

export function setNs(ns: Table) {
  if (hasContext("setting-namespace")) {
    const store = getContext<Writable<Table>>("setting-namespace");
    store.set(ns);
  } else {
    setContext("setting-namespace", writable(ns));
  }

}

export function getNs(): Writable<Table> {
  if (!hasContext("setting-namespace")) {
    throw new Error("No setting namespace found");
  }

  return getContext("setting-namespace");
}

export default getNs;