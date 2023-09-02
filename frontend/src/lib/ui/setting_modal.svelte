<script lang="ts">
	import Modal from '$lib/components/modal.svelte';
	import { AsTab, EntryNum, EntrySelect, EntryText } from '$lib/components/setting';
	import { TabLayout } from '$lib/components/tab';
	import Button from '$lib/general_components/button.svelte';
	import type { Config } from '$lib/nvs_dump_reader';
	import type Client from '$lib/nw_w_client';
	import { popUI, ui } from '.';

	export let config: Config;
	export let default_config: Config;
	export let client: Client;

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
						? [
								(x.value.length >>> 24) & 0xff,
								(x.value.length >>> 16) & 0xff,
								(x.value.length >>> 8) & 0xff,
								x.value.length & 0xff,
								...[...x.value].map((x) => x.charCodeAt(0))
						  ]
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
</script>

<Modal showed={$ui === 'setting_modal'} on:close={popUI}>
	<TabLayout>
		<AsTab ns_list={config.network_list.map((x) => ({ ...x }))} name="network">
			<EntryNum key="id" name="ID" type="int" />
			<EntrySelect key="mode" name="Mode" labels={['None', 'STA', 'AP']} />
			<EntryText key="ssid" name="SSID" />
			<EntryText key="password" name="Password" />
			<EntryText key="hostname" name="Hostname" />
			<EntrySelect key="ip_mode" name="IP Mode" labels={['DHCP', 'Static']} />
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
			<EntryNum key="id" name="ID" type="int" />
			<EntryNum key="tx" name="TX" type="pin" />
			<EntryNum key="rx" name="RX" type="pin" />
			<EntryNum key="baudrate" name="Baud Rate" type="int" />

			<EntrySelect key="parity" name="Parity" labels={['None', 'Even', 'Odd']} />
		</AsTab>
	</TabLayout>
	<div style="position: absolute; bottom: 10px; right: 10px">
		<Button on:click={popUI}>Close</Button>
		<Button on:click={() => (config = default_config.clone())}>Reset</Button>
		<Button on:click={applyConfig}>Apply</Button>
	</div>
</Modal>
