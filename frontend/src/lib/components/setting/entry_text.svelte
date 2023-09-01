<script lang="ts">
	import { getNs } from './table';
	import InputText from '$lib/general_components/input/input_text.svelte';

	export let name: string;
	export let placeholder: string = '';
	export let key: string;

	const table = getNs();
	const entry = $table.entryStr(key);
</script>

<InputText {name} {placeholder} bind:value={$entry} />
