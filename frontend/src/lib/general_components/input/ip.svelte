<script lang="ts">
	import InputImpl from '$lib/general_components/input/input_impl.svelte';

	export let initial_value: number = 0;
	export let value: number = initial_value;

	let octet1 = '0';
	let octet2 = '0';
	let octet3 = '0';
	let octet4 = '0';

	$: {
		octet1 = (initial_value >>> 24).toString(10);
		octet2 = ((initial_value >>> 16) & 0xff).toString(10);
		octet3 = ((initial_value >>> 8) & 0xff).toString(10);
		octet4 = (initial_value & 0xff).toString(10);
	}

	$: value =
		((parseInt(octet1) << 24) |
			(parseInt(octet2) << 16) |
			(parseInt(octet3) << 8) |
			parseInt(octet4)) >>>
		0;
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
