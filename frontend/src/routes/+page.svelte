<script lang="ts">
	import Log from '$lib/components/log.svelte';
	import Modal from '$lib/components/modal.svelte';
	import { AsTab, EntryNum } from '$lib/components/setting';
	import EntrySelect from '$lib/components/setting/entry_select.svelte';
	import EntryText from '$lib/components/setting/entry_text.svelte';
	import { TabLayout } from '$lib/components/tab';
	import Button from '$lib/general_components/button.svelte';
	import InputText from '$lib/general_components/input/input_text.svelte';
	import { Config, NVS } from '$lib/nvs_dump_reader';

	import Client from '$lib/nw_w_client';

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

	function applyConfig() {
		console.log('[Config] Applying config');

		const updated_script = config.nvs.dump();
		const base_script = default_config.nvs.dump();

		const script = updated_script
			.filter(
				(u_s) =>
					typeof u_s.value !== 'undefined' &&
					!base_script.some(
						(b_s) => b_s.ns === u_s.ns && b_s.key === u_s.key && b_s.value === u_s.value
					)
			)
			.map((x) => ({
				...x,
				value:
					typeof x.value === 'string'
						? [...x.value].map((x) => x.charCodeAt(0))
						: typeof x.value === 'number'
						? [
								(x.value >>> 24) & 0xff,
								(x.value >>> 16) & 0xff,
								(x.value >>> 8) & 0xff,
								x.value & 0xff
						  ]
						: (x.value as number[])
			}));

		script.forEach((opt) => {
			client.UploadNVS(opt.ns, opt.key, opt.type, opt.value);
		});

		client.reset();

		client = undefined as unknown as Client;
		config = undefined as unknown as Config;
	}

	//

	type UIName = 'main' | 'setting_modal';
	let ui_stack: UIName[] = ['main'];
	let ui: UIName;
	$: ui = ui_stack[ui_stack.length - 1];

	function popUI() {
		// console.log('[UI] pop UI');
		ui_stack = ui_stack.slice(0, ui_stack.length - 1);
	}

	function pushUI(name: UIName) {
		// console.log(`[UI] Push UI ${name}`);
		ui_stack = [...ui_stack, name];
	}
</script>

<div id="app" class:shadowed={ui !== 'main'}>
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

<Modal showed={ui === 'setting_modal'} on:close={popUI}>
	<TabLayout>
		<AsTab ns_list={config.network_list.map((x) => ({ ...x }))} name="network">
			<EntryNum key="id" name="ID" type="int" />
			<EntrySelect key="mode" name="Mode" labels={['None', 'STA', 'AP']} />
			<EntryText key="ssid" name="SSID" />
			<EntryText key="password" name="Password" />
			<EntryText key="hostname" name="Hostname" />
			<EntryNum key="ip" name="IP" type="ip_address" />
			<EntryNum key="subnet" name="Subnet Mask" type="ip_address" />
			<EntryNum key="gateway" name="Gateway" type="ip_address" />
		</AsTab>
		<AsTab ns_list={config.stm32_list.map((x) => ({ ...x }))} name="stm32">
			<EntryNum key="id" name="ID" type="int" />
			<EntryNum key="reset" name="Reset" type="pin" />
			<EntryNum key="boot0" name="Boot0" type="pin" />
			<EntryNum key="bid" name="BL" type="int" />
			<EntryNum key="rid" name="RC" type="int" />
		</AsTab>
		<AsTab ns_list={config.bootloaders.map((x) => ({ ...x }))} name="stm32bl">
			<EntryNum key="id" name="ID" type="int" />
			<EntrySelect key="bus_type" name="Type" labels={['SPI', 'UART']} />
			<EntryNum key="bus_port" name="Port" type="int" />
			<EntryNum key="cs" name="CS" type="pin" />
		</AsTab>
		<AsTab ns_list={config.spi_buses.map((x) => ({ ...x }))} name="spi">
			<EntryNum key="id" name="ID" type="int" />
			<EntryNum key="miso" name="MISO" type="pin" />
			<EntryNum key="mosi" name="MOSI" type="pin" />
			<EntryNum key="sclk" name="SCLK" type="pin" />
		</AsTab>
		<AsTab ns_list={config.uart_ports.map((x) => ({ ...x }))} name="uart">
			<EntryNum key="port" name="Port" type="int" />
			<EntryNum key="tx" name="TX" type="pin" />
			<EntryNum key="rx" name="RX" type="pin" />
			<EntryNum key="baudrate" name="baudrate" type="int" />

			<EntrySelect key="parity" name="Parity" labels={['None', 'Even', 'Odd']} />
		</AsTab>
	</TabLayout>
	<div style="position: absolute; bottom: 10px; right: 10px">
		<Button on:click={popUI}>Close</Button>
		<Button on:click={() => (config = default_config.clone())}>Reset</Button>
		<Button on:click={applyConfig}>Apply</Button>
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
