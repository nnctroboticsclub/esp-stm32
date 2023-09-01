import type { NVSEntry, NVSNamespace } from "$lib/nvs_dump_reader/namespace";
import { getContext, hasContext, setContext } from "svelte";


export type Table = NVSNamespace;

export function setNs(ns: Table) {
  setContext("setting-namespace", ns);
}

export function getNs(): Table {
  if (!hasContext("setting-namespace")) {
    throw new Error("No setting namespace found");
  }

  return getContext("setting-namespace");
}

export default getNs;