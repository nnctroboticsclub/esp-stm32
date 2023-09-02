<script lang="ts">
	import Log from '$lib/components/log.svelte';
	import Button from '$lib/general_components/button.svelte';
	import InputText from '$lib/general_components/input/input_text.svelte';
	import { Config, NVS } from '$lib/nvs_dump_reader';

	import Client from '$lib/nw_w_client';
	import { pushUI, ui } from '$lib/ui';
	import SettingModal from '$lib/ui/setting_modal.svelte';

	let files: FileList;
	let filename = 'Unspecified';
	let ip_addr: string;

	$: filename = files ? files[0]?.name ?? 'Unspecified' : 'Unspecified';

	let client: Client;

	$: if (files) {
		console.log(`Uploading file ${filename}`);
		const reader = new FileReader();
		reader.onload = async (e) => {
			const data = e.target?.result;
			if (!data) return;

			await client.BLEnter();
			await client.BLUpload(data);
			await client.S3reset();
		};
		reader.readAsArrayBuffer(files[0]);
	}

	let nvs: NVS;
	$: if (client) {
		console.log('[Config] Reading config');
		client.DumpNVS().then((x) => (nvs = NVS.fromDump([...new Uint8Array(x)])));
	}

	let default_config: Config;
	let config: Config;

	$: if (nvs) {
		default_config = new Config(nvs);
		config = default_config.clone();
	}
</script>

<div id="app" class:shadowed={$ui !== 'main'}>
	<div class="app-bar">
		<Button on:click={() => (client = new Client(ip_addr))}>Connect</Button>
		{#if config}
			<Button on:click={() => pushUI('setting_modal')}>Edit ESP32 config</Button>
		{/if}
	</div>
	<InputText name="IP Address" bind:value={ip_addr} placeholder="Server IP Address" />
	<div id="file_chooser">
		<input type="file" name="" id="file_selector" bind:files />
		<span>Flash Upload</span>
	</div>
	<div class="file_name">
		File:<br />
		<span>{filename}</span>
	</div>
	<Log patch_console={true} />
</div>

<SettingModal {config} {default_config} {client} />

<style>
	#app {
		position: absolute;
		top: 0;
		left: 0;
		z-index: 0;
		width: 90vw;
		height: 90vh;
		margin: 5vh 5vw;
		padding: 20px;
		box-sizing: border-box;
		box-shadow: 0 0 10px #888;
		border-radius: 10px;
	}
	#app > .app-bar {
		width: 100%;
		height: 60px;
		display: flex;
		flex-direction: row;
		align-items: center;
		justify-content: flex-start;
	}
	#app > #file_chooser {
		position: relative;
		display: block;
		width: 200px;
		height: 200px;
		margin: 30px calc(50% - 100px);
		z-index: 0;
		padding: 1%;
		border-radius: 10px;
		box-shadow: 0 0 10px #888;
	}
	#app > #file_chooser > input[type='file'] {
		position: absolute;
		top: 0;
		left: 0;
		height: 100%;
		width: 100%;
		z-index: 500;
		opacity: 0;
		cursor: pointer;
	}
	#app > #file_chooser > span {
		position: absolute;
		top: 0;
		left: 0;
		display: block;
		height: 100%;
		width: 100%;
		z-index: 0;
		text-align: center;
		line-height: 200px;
		font-size: 2em;
	}
	#app > .file_name {
		display: block;
		width: 100%;
		height: 50px;
		font-size: 1.2em;
	}
	#app.shadowed::after {
		z-index: 10;
		content: '';
		position: fixed;
		top: 0;
		left: 0;
		width: 100%;
		height: 100%;

		background-color: black;
		opacity: 40%;
	}
</style>
