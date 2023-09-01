<script lang="ts">
	import { createEventDispatcher } from 'svelte';

	export let active: boolean = false;
	export let width: number = 80;
	export let tab_index: number = 0;

	const dispatch = createEventDispatcher();
</script>

<div
	class="button"
	class:active
	style="width: {width}"
	aria-pressed={active}
	aria-label="button"
	role="button"
	tabindex={tab_index}
	on:click={() => {
		dispatch('click');
	}}
	on:keydown={(e) => {
		if (e.key === ' ' || e.key === 'Enter') {
			dispatch('click');
		}
	}}
>
	<slot />
</div>

<style>
	.button {
		display: inline-block;
		padding: 10px;
		margin: 5px 5px;
		border: 1px solid #444;
		border-radius: 5px;
		cursor: pointer;
		box-shadow: 0 0 2px #888;
		z-index: 10;
		text-align: center;
	}
	.button:hover {
		background-color: var(--hover-background-color);
	}
	.button.active {
		background-color: var(--highlight-background-color);
	}
</style>
