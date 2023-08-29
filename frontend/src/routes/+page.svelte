<script>
	import TabContent from '$lib/tab/tab_content.svelte';
	import TabLayout from '$lib/tab/tab_layout.svelte';
</script>

<TabLayout>
	<TabContent getSecondaryBarTitles={() => ['a0', 'b0']} name="network">network</TabContent>
	<TabContent getSecondaryBarTitles={() => ['a0', 'b1']} name="stm32">stm32</TabContent>
	<TabContent getSecondaryBarTitles={() => ['a1', 'b0']} name="stm32bl">stm32bl</TabContent>
	<TabContent getSecondaryBarTitles={() => ['a1', 'b1']} name="spi">spi</TabContent>
	<TabContent getSecondaryBarTitles={() => ['a2', 'b0']} name="uart">uart</TabContent>
</TabLayout>
