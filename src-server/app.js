/* eslint-disable no-console */
const http = require('http');
const path = require('path');
const fs = require('fs');
const express = require('express');
const WebSocket = require('ws');
const bodyParser = require('body-parser');
// const { postgraphile } = require('postgraphile');
const cors = require('cors');
const { ApolloServer } = require('apollo-server-express');

const { update, getFileList } = require('./updater');
const { typeDefs, resolvers } = require('./schema');
const StoredDevices = require('./databaseless');

const devs = new StoredDevices();

const apollo = new ApolloServer({ typeDefs, resolvers });
const app = express();
apollo.applyMiddleware({ app });

const server = http.createServer(app);
const port = process.env.PORT || 8888;
const wsServer = new WebSocket.Server({ server });

app.use(cors());
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: false }));

// app.use(
//   postgraphile(
//     process.env.DATABASE_URL ||
//       'postgres://rozumaha_server:rozumaha@localhost/rozumahouse'
//   )
// );

app.get('/updates', update);

let devices = [];
let users = [];

const addDeviceInfo = info => {
  let deviceExists;
  devices = devices.map(device => {
    if (device.ip === info.ip || device.mac === info.mac) {
      deviceExists = true;
      return { ...device, ...info };
    }
    return device;
  });
  if (!deviceExists) {
    devices.push(info);
  }
  devs.updateDevices(devices);
};

wsServer.on('connection', (socket, req) => {
  const { remoteAddress, remoteFamily } = req.connection;
  console.log(
    `Connection from ${remoteAddress} - ${remoteFamily} - ${req.url}`
  );
  const ipRegex = /[\d]{1,3}\.[\d]{1,3}\.[\d]{1,3}\.[\d]{1,3}/gm;
  const [ip] = ipRegex.exec(remoteAddress);

  if (req.url === '/devices' || req.url === '/auto') {
    addDeviceInfo({ ip });
    setTimeout(() => {
      addDeviceInfo({
        ip,
        socket: socket,
      });
    }, 1000);
  }
  if (req.url === '/users') {
    users.push(socket);
  }

  socket.on('message', message => {
    let parsedMessage;
    try {
      parsedMessage = JSON.parse(message);
    } catch (error) {
      console.error(error);
    }
    if (!parsedMessage) return;

    if (req.url !== '/auto') {
      console.log(parsedMessage);
      console.log('===========');
    } else {
      // temp autoheater
      /*
      { model: 'auto-heater',
        mac: 'A0:20:A6:21:C7:6D',
        type: 'currentSettings',
        data: { minTemp: 20, maxTemp: 22, heatOn: false, manual: false } }
      */
      const { type, data, ...other } = parsedMessage;
      switch (type) {
        case 'currentSettings':
          addDeviceInfo({ ip, settings: data, ...other });
          break;
        case 'sensorData':
          addDeviceInfo({ ip, sensors: data, ...other });
          break;
        default:
          break;
      }
    }
    const { kind, ...other } = parsedMessage;
    // console.log(other);
    if (!kind) return;
    switch (kind) {
      case 'about':
        addDeviceInfo(other);
        break;
      default:
        break;
    }
  });

  // if (req.url === '/devices') {
  // }
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
