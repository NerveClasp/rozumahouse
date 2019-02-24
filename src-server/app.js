/* eslint-disable no-console */
const http = require('http');
const path = require('path');
const fs = require('fs');
const express = require('express');
const WebSocket = require('ws');
const bodyParser = require('body-parser');
// const rp = require('request-promise');
// const shortid = require('shortid');
const app = express();
const server = http.createServer(app);
const port = process.env.PORT || 7331;
const wsServer = new WebSocket.Server({ server });

let devices = [];
let users = [];

const randomColor = () => Math.floor(Math.random() * 254);
const randomGradient = () => ({
  mode: 'gradient_rgb',
  from: {
    r: randomColor(),
    g: randomColor(),
    b: randomColor(),
  },
  to: {
    r: randomColor(),
    g: randomColor(),
    b: randomColor(),
  },
});

wsServer.on('connection', (socket, req) => {
  const { remoteAddress, remoteFamily } = req.connection;
  console.log(
    `Connection from ${remoteAddress} - ${remoteFamily} - ${req.url}`
  );

  if (req.url === '/devices') {
    devices.push(socket);
  }
  if (req.url === '/users') {
    users.push(socket);
  }

  socket.on('message', message => {
    if (req.url !== '/auto') {
      console.log(message);
    }
  });

  if (req.url === '/devices') {
    setInterval(() => {
      const message = {
        action: 'command',
        right: randomGradient(),
        left: randomGradient(),
        brightness: 10,
      };
      // console.log(`sending ${JSON.stringify(message)}`);
      socket.send(JSON.stringify(message));
    }, 2000);
  }
});

app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: false }));

const staticDir = path.resolve(path.join(__dirname, '..', 'build'));

/**
 * Serve client only when deploying
 */
if (fs.existsSync(staticDir)) {
  app.use(express.static(staticDir));
}

server.listen(port, () =>
  console.log(`Rozumahouse is listening on port ${port} ;)`)
);
