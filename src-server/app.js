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
const { led } = require('./socket');
const StoredDevices = require('./databaseless');

const devs = new StoredDevices();

const apollo = new ApolloServer({ typeDefs, resolvers });
const app = express();
apollo.applyMiddleware({ app });

const server = http.createServer(app);
const port = process.env.PORT || 7331;
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
  devices = devices.map(device => {
    if (device.ip !== info.ip) return device;
    return { ...device, ...info };
  });
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
    const parsedMessage = JSON.parse(message);
    if (req.url !== '/auto') {
      console.log(message);
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
    if (!kind) return;
    switch (kind) {
      case 'about':
        addDeviceInfo(other);
        console.log(`other: ${JSON.stringify(other)}`);
        if (other.model === 'esp8266-dual-leds') {
          led.initDual({ socket, brightness: 2 });
        }
        break;
      default:
        break;
    }
  });

  if (req.url === '/devices') {
    led.off({ socket, ledName: 'left' });
    led.off({ socket, ledName: 'right' });
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
