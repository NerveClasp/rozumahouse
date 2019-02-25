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
let forwardNone = 0;
const delayTime = 15000;

const getAnimation = () => {
  forwardNone++;
  return {
    action: 'command',
    right: {
      animation: 'forward',
      // animation: forwardNone % 2 ? 'forward' : 'none',
    },
    left: {
      animation: 'forward',

      // animation: forwardNone % 2 ? 'forward' : 'none',
    },
  };
};

const randomColor = (min = 0, max = 125) =>
  Math.floor(Math.random() * (max - min + 1)) + min;

const randomGradient = () => ({
  r: randomColor(),
  g: randomColor(),
  b: randomColor(),
});

const makeGradients = () => {
  const from = randomGradient();
  return {
    action: 'command',
    right: {
      mode: 'gradient_rgb',
      from,
      to: randomGradient(),
    },
    left: {
      mode: 'gradient_rgb',
      from,
      to: randomGradient(),
    },
    brightness: 254,
  };
};

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
        brightness: 254,
      })
    );
    // console.log(`sending ${JSON.stringify(message)}`);
    // }, delayTime);
    // setInterval(() => {
    // const message = getAnimation();
    if (socket.readyState !== 3) {
      socket.send(
        JSON.stringify({
          action: 'command',
          animation: 'back-and-forth',
        })
      );
      // socket.send(JSON.stringify(message));
    }
    //   // console.log(`sending ${JSON.stringify(message)}`);
    // }, delayTime);
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
