<script context="module">
  import { gql } from "apollo-boost";
  import { client } from "./apollo";
  import { GET_DEVICES } from "./queries";
  import Led from "./Led.svelte";

  export async function preload() {
    return {
      cache: await client.query({ query: GET_DEVICES })
    };
  }
</script>
<script>
  import { getClient, restore, query, subscribe } from 'svelte-apollo';

  export let cache;
  restore(client, GET_DEVICES, cache.data);

  const devices = subscribe(client, { query: GET_DEVICES });

  function reload() {
    devices.refetch();
  }
</script>

<ul>
  {#await $devices}
    <li>Loading...</li>
  {:then result}
    <!-- {JSON.stringify(result)} -->
    {#each result.data.devices as device (device.mac)}
      <li>{device.ip}</li>
      <!-- {JSON.stringify(device, null, 2)} -->
      {#each device.status as led}
        <p><Led mac={device.mac} ip={device.ip} {...led} /></p>
      {/each}
    {:else}
      <li>No devices found</li>
    {/each}
  {:catch error}
    <li>Error loading devices: {error}</li>
  {/await}
  <button on:click={reload}>Refresh</button>
</ul>