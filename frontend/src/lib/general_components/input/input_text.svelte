<script lang="ts">
	import InputImpl from '$lib/general_components/input/input_impl.svelte';

	export let name: string;
	export let placeholder: string = '';
	export let value: string;
	export let width: number = 100;
</script>

<div class="input">
	<span class="name">
		{name}
	</span>
	<InputImpl {placeholder} type="text" bind:value {width} />
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
</style>
