<script lang="ts">
	export let patch_console: boolean = false;

	let buffer: string = '';

	if (patch_console) {
		function factory(orig: Function, name: string) {
			function my_stringify(x: any): string {
				if (x === undefined) return 'undefined';
				if (x === null) return 'null';

				if (Object.hasOwn(x, 'toString') && x.toString() !== '[object Object]') {
					return x.toString();
				}

				if (typeof x === 'string') {
					return x;
				} else if (typeof x === 'number') {
					return x.toString();
				} else if (typeof x === 'boolean') {
					return x.toString();
				} else if (typeof x === 'function') {
					return '<function>';
				}

				if (Array.isArray(x)) {
					return '[' + x.map(my_stringify).join(', ') + ']';
				}

				return JSON.stringify(x);
			}

			return function () {
				const args = Array.from(arguments);
				const buf = name + args.map(my_stringify).join(' ');
				buffer += buf + '\n';
				orig.apply(console, args);
			};
		}

		console.log = factory(console.log, 'C L: ');
		console.error = factory(console.error, 'C E: ');
		console.warn = factory(console.warn, 'C W: ');
		console.info = factory(console.info, 'C I: ');
		console.debug = factory(console.debug, 'C D: ');

		console.log('Patched console logging functions.');
	}
</script>

<div class="log">
	{@html buffer.replace(/\n/g, '<br />').split(' ').map((x) => x.trim()).join(' ')}
</div>

<style>
	.log {
		position: absolute;
		top: 400px;
		bottom: 20px;
		left: 20px;
		right: 20px;
		z-index: 10;
		background-color: var(--log-background);
		padding: 10px;
		border-radius: 10px;
		box-sizing: border-box;
		overflow-y: scroll;
		font-family: 'Fira Code Light', 'Fira Code', monospace;
		font-size: small;
	}
</style>
