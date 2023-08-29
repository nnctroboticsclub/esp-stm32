<script lang="ts">
	import Button from './button.svelte';

	export let tabs: string[];
	export let active_tab: string;
</script>

<div class="bar">
	{#each tabs as tab}
		<Button
			active={active_tab === tab}
			on:click={() => {
				active_tab = tab;
			}}>{tab}</Button
		>
	{/each}
</div>

<style>
	.bar {
		display: flex;
		flex-direction: row;
		width: 100%;
	}
</style>
