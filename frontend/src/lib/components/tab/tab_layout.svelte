<script lang="ts">
	import { derived } from 'svelte/store';
	import { getActiveSecondaryTab, getActiveTab, getTabs } from './context';
	import TabList from './tab_list.svelte';

	const tabs = getTabs();
	const active_tab = getActiveTab();
	const secondary_tab = getActiveSecondaryTab();

	const secondary_bar_titles = derived(
		active_tab,
		(a) => $tabs.find((x) => x.title == a)?.secondaryBarTitles || []
	);

	const tab_titles = derived(tabs, (a) => a.map((x) => x.title));

	$active_tab = $tab_titles[0];

	secondary_bar_titles.subscribe((a) => {
		if ($secondary_bar_titles.includes($secondary_tab)) return;
		$secondary_tab = $secondary_bar_titles[0];
	});
</script>

<div>
	<TabList tabs={$tab_titles} bind:active_tab={$active_tab} />
	<TabList tabs={$secondary_bar_titles} bind:active_tab={$secondary_tab} />

	<slot />
</div>

<style>
</style>
