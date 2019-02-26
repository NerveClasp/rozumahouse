/* eslint-disable no-console */
const http = require('http');
const path = require('path');
const fs = require('fs');
const express = require('express');
const WebSocket = require('ws');
const bodyParser = require('body-parser');
// const { postgraphile } = require('postgraphile');

const { update, getFileList } = require('./updater');
const express_graphql = require('express-graphql');
const { buildSchema } = require('graphql');
const cors = require('cors');
const {
  ApolloServer,
  gql,
  graphqlExpress,
  graphiqlExpress,
} = require('apollo-server-express');
// const { makeExecutableSchema } = require('graphql-tools')
// GraphQL schema
const schema = buildSchema(`
  type Query {
    device(ip: String!): Device
    devices(model: String): [Device]
  },
  type Device {
    name: String
    room: String
    mac: String
    ip: String
    chipId: String
    model: String
    freeSketchSpace: Int
    coreVersion: String
    sdkVersion: String
    action: [String]
    command: [String]
    animation: [String]
    left: [String]
    right: [String]
  }
`);

const getDevice = args => {
  const { ip } = args;
  return devices.find(device => device.ip === ip);
};

const getDevices = args => {
  const { model } = args;
  if (!model) return devices;
  return devices.filter(device => device.model === model);
};

const getDeviceApollo = (_, args) => {
  const { ip } = args;
  return devices.find(device => device.ip === ip);
};

const getDevicesApollo = (_, args) => {
  const { model } = args;
  if (!model) return devices;
  return devices.filter(device => device.model === model);
};

// Root resolver
const root = {
  device: getDevice,
  devices: getDevices,
};

// The GraphQL schema
const typeDefs = gql`
  type Query {
    device(ip: String!): Device
    devices(model: String): [Device]
  }
  type Device {
    name: String
    room: String
    mac: String
    ip: String
    chipId: String
    model: String
    freeSketchSpace: Int
    coreVersion: String
    sdkVersion: String
    action: [String]
    command: [String]
    animation: [String]
    left: [String]
    right: [String]
  }
`;

// A map of functions which return data for the schema.
const resolvers = {
  Query: {
    device: getDeviceApollo,
    devices: getDevicesApollo,
  },
};

const apollo = new ApolloServer({ typeDefs, resolvers });

// const myGraphQLSchema = makeExecutableSchema({ typeDefs, resolvers })

const app = express();

apollo.applyMiddleware({ app });

const server = http.createServer(app);
const port = process.env.PORT || 7331;
const wsServer = new WebSocket.Server({ server });

app.use(cors());
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: false }));

// app.use(
//   '/__graphql',
//   express_graphql({
//     schema: schema,
//     rootValue: root,
//     graphiql: true,
//   })
// );

app.get('/updates', update);
// app.use(
//   postgraphile(
//     process.env.DATABASE_URL ||
//       'postgres://rozumaha_server:rozumaha@localhost/rozumahouse'
//   )
// );

let devices = [];
let users = [];

const randomColor = (min = 0, max = 125) =>
  Math.floor(Math.random() * (max - min + 1)) + min;

const addDeviceInfo = info => {
  devices = devices.map(device => {
    if (device.ip !== info.ip) return device;
    return { ...device, ...info };
  });
};

wsServer.on('connection', (socket, req) => {
  const { remoteAddress, remoteFamily } = req.connection;
  console.log(
    `Connection from ${remoteAddress} - ${remoteFamily} - ${req.url}`
  );
  const ipRegex = /[\d]{1,3}\.[\d]{1,3}\.[\d]{1,3}\.[\d]{1,3}/gm;
  const [ip] = ipRegex.exec(remoteAddress);

  if (req.url === '/devices') {
    devices.push({ ip });
    addDeviceInfo({
      ip,
      socket: socket,
    });
  }
  if (req.url === '/users') {
    users.push(socket);
  }

  socket.on('message', message => {
    if (req.url !== '/auto') {
      console.log(message);
      console.log('===========');
    }
    const parsedMessage = JSON.parse(message);
    const { kind, ...other } = parsedMessage;
    if (!kind) return;
    switch (kind) {
      case 'about':
        addDeviceInfo(other);
        console.log(`other: ${JSON.stringify(other)}`);
        if (other.model === 'esp8266-dual-leds') {
          socket.send(
            JSON.stringify({
              action: 'command',
              brightness: 5,
            })
          );
        }
        break;
      default:
        break;
    }
  });

  if (req.url === '/devices') {
    // setInterval(() => {
    // const message = makeGradients();
    // if (socket.readyState !== 3) {
    //   socket.send(JSON.stringify(message));
    // }
    socket.send(
      JSON.stringify({
        action: 'command',
        right: {
          mode: 'gradient_rgb',
          from: {
            r: 254,
            g: 100,
            b: 0,
          },
          to: {
            r: 0,
            g: 100,
            b: 254,
          },
        },
        left: {
          mode: 'gradient_rgb',
          from: {
            r: 254,
            g: 100,
            b: 0,
          },
          to: {
            r: 0,
            g: 100,
            b: 254,
          },
        },
        brightness: 20,
      })
    );
    // console.log(`sending ${JSON.stringify(message)}`);
    // }, delayTime);
    // setInterval(() => {
    // const message = getAnimation();
    setTimeout(() => {
      if (socket.readyState !== 3) {
        socket.send(
          JSON.stringify({
            action: 'command',
            animation: 'back-and-forth',
          })
        );
      }
    }, 3000);
    // socket.send(
    //   JSON.stringify({
    //     action: 'check-for-updates',
    //   })
    // );
    //   // console.log(`sending ${JSON.stringify(message)}`);
    // }, delayTime);
  }
});

const staticDir = path.resolve(path.join(__dirname, '..', 'build'));

console.log(JSON.stringify(getFileList()));
/**
 * Serve client only when deploying
 */
if (fs.existsSync(staticDir)) {
  app.use(express.static(staticDir));
}

apollo.installSubscriptionHandlers(wsServer);

server.listen(port, () =>
  console.log(`Rozumahouse is listening on port ${port} ;)`)
);
