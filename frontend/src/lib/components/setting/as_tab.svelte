<script lang="ts">
	import { Setting } from '.';
	import { TabContent, getActiveSecondaryTab } from '../tab';
	import type { Table } from './table';

	export let ns_list: Table[];
	export let name: string;

	const secondary_tab = getActiveSecondaryTab();

	let ns: Table;
	$: ns = ns_list.find((x) => x.entryNum('id').get() === parseInt($secondary_tab)) || ns_list[0];

	const secondary_tabs = ns_list.map((x) => x.entryNum('id').get()?.toString() ?? '--');

	$: if (ns) console.log('[AsLog] Using ns=', ns.dump());
</script>

<TabContent secondaryBarTitles={secondary_tabs} {name}>
	{#if ns}
		<Setting {ns}>
			<slot />
		</Setting>
	{/if}
</TabContent>
