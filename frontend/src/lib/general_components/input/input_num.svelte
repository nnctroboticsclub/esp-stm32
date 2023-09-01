<script lang="ts">
	import InputImpl from '$lib/general_components/input/input_impl.svelte';
	import Ip from '$lib/general_components/input/ip.svelte';
	import { setContext } from 'svelte';
	import { writable } from 'svelte/store';

	export let name: string;
	export let type: 'ip_address' | 'pin' | 'int';
	export let placeholder: string = '';
	export let value: number;

	let value_as_str = {
		subscribers: [] as ((x: string) => void)[],
		subscribe: (cb: (x: string) => void) => {
			value_as_str.subscribers.push(cb);
			if (!isNaN(value)) cb(value.toString());
			else {
				console.log('value is NaN:', value);
				cb('');
			}
			return () => {
				value_as_str.subscribers = value_as_str.subscribers.filter((x) => x !== cb);
			};
		},
		set: (x: string) => {
			value_as_str.subscribers.forEach((cb) => cb(x));
			value = parseInt(x);
		}
	};

	const value_store = writable(value);
	setContext('gc-input-value', value_store);
	$: value_store.set(value);
</script>

<div class="input {type}">
	<span class="name">
		{name}
	</span>
	{#if type === 'ip_address'}
		<Ip bind:value={$value_as_str} />
	{:else if type === 'pin'}
		<InputImpl {placeholder} type="number" bind:value={$value_as_str} />
	{:else if type === 'int'}
		<InputImpl {placeholder} type="number" bind:value={$value_as_str} width={100} />
	{/if}
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

	.input.select {
		display: flex;
		flex-direction: row;
	}
</style>
