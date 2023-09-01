<script lang="ts">
	import InputSelect from '$lib/general_components/input/input_select.svelte';
	import type { NVSEntry } from '$lib/nvs_dump_reader/namespace';
	import { getNs } from './table';

	export let name: string;
	export let key: string;
	export let labels: string[];

	const table = getNs();

	let entry: NVSEntry<number>;
	$: entry = $table.entryNum(key);
</script>

<InputSelect {name} bind:value={$entry} {labels}><slot /></InputSelect>
