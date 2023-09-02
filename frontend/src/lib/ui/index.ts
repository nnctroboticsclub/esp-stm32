import { writable } from "svelte/store";

type UIName = 'main' | 'setting_modal';
let ui_stack: UIName[] = ['main'];

export const ui = writable<UIName>('main');

export function popUI() {
  if (ui_stack.length <= 1) {
    console.error('[UI] Cannot pop UI');
    return;
  }
  ui_stack = ui_stack.slice(0, ui_stack.length - 1);
  ui.set(ui_stack[ui_stack.length - 1]);
}

export function pushUI(name: UIName) {
  ui_stack = [...ui_stack, name];
  ui.set(name);
}
