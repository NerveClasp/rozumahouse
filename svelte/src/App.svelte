<script>
  export let name;
  import Devices, { preload } from "./Devices.svelte";
  import { client } from "./apollo.js";
  import { setClient } from "svelte-apollo";
  const preloading = preload();
  setClient(client);
</script>

<style>
	h1 {
	  color: purple;
	}
</style>

<h1>Hello {name}!</h1>
<h2>Devices</h2>

	{#await preloading}
		<p>Preloading devices....</p>
	{:then preloaded}
		<Devices {...preloaded} />
	{:catch error}
		<p>Error preloading devices: {error}</p>
	{/await}
