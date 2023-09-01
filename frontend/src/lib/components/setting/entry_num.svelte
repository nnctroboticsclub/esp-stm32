<script lang="ts">
	import { getNs } from './table';
	import InputNum from '$lib/general_components/input/input_num.svelte';
	import { setContext } from 'svelte';

	export let name: string;
	export let type: 'ip_address' | 'pin' | 'int' | 'select';
	export let placeholder: string = '';
	export let key: string;

	const table = getNs();
	const entry = table.entryNum(key);
	console.debug('[EntryNum] entry=', entry, '(num) on', table.namespace_name);
</script>

<InputNum {name} {type} {placeholder} bind:value={$entry}><slot /></InputNum>
