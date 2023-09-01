<script lang="ts">
	import { getNs } from './table';
	import InputNum from '$lib/general_components/input/input_num.svelte';
	import type { NVSEntry } from '$lib/nvs_dump_reader/namespace';

	export let name: string;
	export let type: 'ip_address' | 'pin' | 'int';
	export let placeholder: string = '';
	export let key: string;

	const table = getNs();

	let entry: NVSEntry<number>;
	$: entry = $table.entryNum(key);
</script>

<InputNum {name} {type} {placeholder} bind:value={$entry}><slot /></InputNum>
