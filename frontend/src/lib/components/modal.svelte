<script lang="ts">
	import { createEventDispatcher } from 'svelte';

	export let showed: boolean;

	let dispatch = createEventDispatcher();
</script>

{#if showed}
	<div
		class="modal"
		on:click|self={() => {
			dispatch('close');
		}}
		role="dialog"
	>
		<div class="container">
			<slot />
		</div>
	</div>
{/if}

<style>
	.modal {
		position: fixed;
		top: 0;
		left: 0;
		width: 100vw;
		height: 100vh;
	}
	.container {
		position: absolute;
		top: 100px;
		left: 100px;
		right: 100px;
		bottom: 100px;
		z-index: 20;

		background-color: var(--background-color);
		box-shadow: 0 0 7px #888;

		border-radius: 10px;
		padding: 20px;
		box-sizing: border-box;
	}
</style>
