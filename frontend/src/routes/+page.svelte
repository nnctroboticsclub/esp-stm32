<script lang="ts">
	import Log from '$lib/components/log.svelte';
	import Modal from '$lib/components/modal.svelte';
	import { AsTab, EntryNum } from '$lib/components/setting';
	import EntrySelect from '$lib/components/setting/entry_select.svelte';
	import EntryText from '$lib/components/setting/entry_text.svelte';
	import { TabLayout } from '$lib/components/tab';
	import Button from '$lib/general_components/button.svelte';
	import InputText from '$lib/general_components/input/input_text.svelte';
	import { Config, NVS, TestNVS } from '$lib/nvs_dump_reader';

	import Client from '$lib/nw_w_client';

	let files: FileList;
	let filename = 'Unspecified';
	let ip_addr: string;

	$: filename = files ? files[0]?.name ?? 'Unspecified' : 'Unspecified';

	$: if (files) {
		console.log(`Uploading file ${filename}`);
		const reader = new FileReader();
		reader.onload = async (e) => {
			const data = e.target?.result;
			const client = new Client(ip_addr);

			await client.BLEnter();
			await client.BLUpload(data);
			await client.reset();
		};
		reader.readAsArrayBuffer(files[0]);
	}

	const nvs = NVS.fromDump(TestNVS);

	const default_config = new Config(nvs);
	let config = default_config.clone();

	console.log(nvs);
	console.log(config);

	// config.network_list[0].ssid.set('Hello World');
	// config.network_list[0].password.set('Hello World');
	// config.network_list[0].hostname.set('Hello World');
	// config.network_list[0].ip.set(0x01020304);
	// config.network_list[0].subnet.set(0x01020304);
	// config.network_list[0].gateway.set(0x01020304);

	function calculateScript(base: Config, update: Config): string[] {
		const updated_script = update.nvs.dumpScript();
		const base_script = base.nvs.dumpScript();

		return updated_script.filter((line) => !base_script.includes(line));
	}

	const diff = calculateScript(default_config, config);

	//

	type UIName = 'main' | 'setting_modal';
	let ui_stack: UIName[] = ['main', 'setting_modal'];
	let ui: UIName;
	$: ui = ui_stack[ui_stack.length - 1];

	function popUI() {
		console.log('[UI] pop UI');
		ui_stack = ui_stack.slice(0, ui_stack.length - 1);
	}

	function pushUI(name: UIName) {
		console.log(`[UI] Push UI ${name}`);
		ui_stack = [...ui_stack, name];
	}
</script>

<div id="app" class:shadowed={ui !== 'main'}>
	<Button on:click={() => pushUI('setting_modal')}>Settings</Button>
	<InputText name="IP Address" bind:value={ip_addr} placeholder="Server IP Address" />
	<div id="file_chooser">
		<input type="file" name="" id="file_selector" bind:files />
		<span>Flash Upload</span>
	</div>
	<div class="file_name">
		File:<br />
		<span>{filename}</span>
	</div>

	<div style="overflow-y: scroll; height: 300px;">
		{#each diff as line}
			{line}<br />
		{/each}
	</div>
	<Log patch_console={true} />
</div>

<Modal showed={ui === 'setting_modal'} on:close={popUI}>
	<TabLayout>
		<AsTab ns_list={config.network_list} name="network">
			<EntryNum key="id" name="ID" type="int" />
			<EntrySelect key="mode" name="Mode" labels={['None', 'STA', 'AP']} />
			<EntryText key="ssid" name="SSID" />
			<EntryText key="password" name="Password" />
			<EntryText key="hostname" name="Hostname" />
			<EntryNum key="ip" name="IP" type="ip_address" />
			<EntryNum key="subnet" name="Subnet Mask" type="ip_address" />
			<EntryNum key="gateway" name="Gateway" type="ip_address" />
		</AsTab>
		<AsTab ns_list={config.stm32_list} name="stm32">
			<EntryNum key="id" name="ID" type="int" />
			<EntryNum key="reset" name="Reset" type="pin" />
			<EntryNum key="boot0" name="Boot0" type="pin" />
			<EntryNum key="bid" name="BL" type="int" />
			<EntryNum key="rid" name="RC" type="int" />
		</AsTab>
		<AsTab ns_list={config.bootloaders} name="stm32bl">
			<EntryNum key="id" name="ID" type="int" />
			<EntrySelect key="bus_type" name="Type" labels={['SPI', 'UART']} />
			<EntryNum key="bus_port" name="Port" type="int" />
			<EntryNum key="cs" name="CS" type="pin" />
		</AsTab>
		<AsTab ns_list={config.spi_buses} name="spi">
			<EntryNum key="port" name="ID" type="int" />
			<EntryNum key="miso" name="MISO" type="pin" />
			<EntryNum key="mosi" name="MOSI" type="pin" />
			<EntryNum key="sclk" name="SCLK" type="pin" />
		</AsTab>
		<AsTab ns_list={config.uart_ports} name="uart">
			<EntryNum key="port" name="Port" type="int" />
			<EntryNum key="tx" name="TX" type="pin" />
			<EntryNum key="rx" name="RX" type="pin" />
			<EntryNum key="baudrate" name="baudrate" type="int" />

			<EntrySelect key="parity" name="Parity" labels={['None', 'Even', 'Odd']} />
		</AsTab>
	</TabLayout>
	<div class="bottom">
		<Button on:click={popUI}>Close</Button>
		<Button on:click={() => (config = default_config.clone())}>Reset</Button>
		<Button on:click={() => (config = config.clone())}>Apply</Button>
	</div>
</Modal>

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
