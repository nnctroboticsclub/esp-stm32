<script lang="ts">
	import InputImpl from '$lib/general_components/input/input_impl.svelte';

	export let initial_value: string = '0.0.0.0';
	export let value: string = initial_value;

	let octet1 = '0';
	let octet2 = '0';
	let octet3 = '0';
	let octet4 = '0';

	$: [octet1, octet2, octet3, octet4] = initial_value.split('.');

	$: value = `${octet1}.${octet2}.${octet3}.${octet4}`;
</script>

<InputImpl placeholder="127" type="number" bind:value={octet1} width={50} />
<span class="dot" />
<InputImpl placeholder="0" type="number" bind:value={octet2} width={50} />
<span class="dot" />
<InputImpl placeholder="0" type="number" bind:value={octet3} width={50} />
<span class="dot" />
<InputImpl placeholder="1" type="number" bind:value={octet4} width={50} />

<style>
	.dot::after {
		content: '.';
		display: inline-block;
		width: 20px;
		vertical-align: bottom;
	}
</style>
