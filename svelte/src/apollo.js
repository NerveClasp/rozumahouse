import ApolloClient from 'apollo-boost';
import { InMemoryCache } from 'apollo-cache-inmemory';


export const client = new ApolloClient({
  uri: `http://${window.location.hostname}:8888/graphql`,
  cache: new InMemoryCache(),
});