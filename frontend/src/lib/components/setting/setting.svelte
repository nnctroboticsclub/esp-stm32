<script lang="ts">
	import { setNs, type Table } from './table';

	export let ns: Table;

	setNs(ns);
</script>

<slot />
