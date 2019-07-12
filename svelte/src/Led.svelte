<script>
  export let mac;
  export let ip;
  export let brightness;
  export let activeLeds;
  export let mode;
  export let color;
  export let animation;
  export let animationDuration;
  export let ledOn;

  import { mutate } from "svelte-apollo";
  import { gql } from "apollo-boost";
  import { client } from "./apollo";
  import { LED_CHANGE } from "./queries";

  async function addBook() {
    try {
      await mutate(client, {
        mutation: LED_CHANGE,
        variables: { mac, ip, brightness }
      });
    } catch (error) {
      // TODO
    }
  }
</script>

<div>
  {brightness}
</div>