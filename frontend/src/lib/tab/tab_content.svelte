<script lang="ts">
	import { getActiveTab, getTabs } from './context';

	export let name: string;
	export let getSecondaryBarTitles: () => string[];

	const active_tab = getActiveTab();
	const tabs = getTabs();

	tabs.update((tabs) => {
		if (tabs.find((tab) => tab.title === name)) {
			return tabs;
		}
		tabs.push({
			title: name,
			getSecondaryBarTitles
		});
		return tabs;
	});

	if ($active_tab === undefined) {
		$active_tab = name;
	}
</script>

{#if $active_tab === name}
	<div class="wrapper"><slot /></div>
{/if}

<style>
	.wrapper {
		display: block;
		width: 100%;
		height: 100%;
		margin: 10px;
		padding: 10px;
		box-sizing: border-box;
		border-radius: 5px;
		background-color: var(--background-color);
		box-shadow: 0 0 1px #888;
		z-index: 5;
	}
</style>
