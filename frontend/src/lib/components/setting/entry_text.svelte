<script lang="ts">
	import { getNs } from './table';
	import InputText from '$lib/general_components/input/input_text.svelte';
	import type { NVSEntry } from '$lib/nvs_dump_reader/namespace';

	export let name: string;
	export let placeholder: string = '';
	export let key: string;

	const table = getNs();
	const entry = $table[key] as NVSEntry<string>;
</script>

<InputText {name} {placeholder} bind:value={$entry} />
