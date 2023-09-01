<script lang="ts">
	export let value: string;
	export let placeholder: string = '';
	export let type: string;
	export let width: number = 200;

	function setType(node: HTMLInputElement) {
		node.type = type;
	}
</script>

<input {placeholder} bind:value use:setType style="width: {width}px" />

<style>
	input {
		resize: none;
		width: 500px;
		height: 40px;
		background-color: transparent;
		border: none;
		border-bottom: 1px solid lightgray;
		box-sizing: border-box;
		line-height: 40px;
		text-align: center;
		color: #ccc;
		overflow-y: hidden;
	}
	input:focus {
		outline: none;
		animation: border_lr 0.2s forwards;
	}
</style>
