<script lang="ts">
	import { setContext } from 'svelte';
	import { writable } from 'svelte/store';

	export let name: string;
	export let value: number;
	export let labels: string[];

	const value_store = writable(value);
	setContext('gc-input-value', value_store);
	$: value_store.set(value);
</script>

<div class="input select">
	<span class="name">
		{name}
	</span>
	{#each labels as label, index}
		<div
			class="select-elem"
			class:active={index === value}
			on:click={() => (value = index)}
			on:keydown={(e) => {}}
			role="button"
			tabindex="0"
		>
			{label}
		</div>
	{/each}
</div>

<style>
	.input {
		display: block;
		width: 100%;
		height: 50px;
		line-height: 50px;

		border-top: 1px solid gray;
		box-sizing: content-box;
	}
	.input:first-child {
		border-top: none;
	}
	.name {
		display: inline-block;
		width: 100px;
		height: 100%;
		line-height: 50px;
		text-align: center;
		box-sizing: border-box;
		border-right: 1px solid gray;
		margin-right: 10px;
	}

	.select {
		display: flex;
		flex-direction: row;
	}

	.select > .select-elem {
		position: relative;
		display: block;
		width: 100px;
		height: calc(100% - 4px);
		margin: 2px 0;
		line-height: 50px;
		text-align: center;
		cursor: pointer;
		margin-right: 3px;
	}
	.select > .select-elem:hover {
		background-color: var(--hover-background-color);
	}
	.select > .select-elem.active {
		background-color: var(--highlight-background-color);
	}
	.select > .select-elem:nth-last-child(n + 2):after {
		content: '';

		position: absolute;
		top: 0;
		left: 100px;

		display: inline-block;
		width: 1px;
		height: 100%;

		background-color: gray;
	}
</style>
