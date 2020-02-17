const { sendToSocket } = require('../utils');

const from = {
  r: 254,
  g: 100,
  b: 0,
};
const to = {
  r: 0,
  g: 100,
  b: 254,
};

const mode = 'gradient_rgb';

const init = ({ socket, brightness = 20, led, animation, ...other }) => {
  sendToSocket(socket, {
    action: 'command',
    led,
    mode,
    from,
    to,
    brightness,
    animation: animation || 'back-and-forth',
    ...other,
  });
};

const initDual = ({ socket, brightness = 20, animation, ...other }) => {
  sendToSocket(socket, {
    action: 'command',
    led: 0,
    mode,
    from,
    to,
    brightness,
    animation: animation || 'back-and-forth',
    ...other,
  });
  sendToSocket(socket, {
    action: 'command',
    led: 1,
    mode,
    from,
    to,
    brightness,
    animation: animation || 'back-and-forth',
    ...other,
  });
};

const color = ({ socket, led, color, ...other }) => {
  sendToSocket(socket, {
    action: 'command',
    led,
    color,
    ...other,
  });
};

const brightness = ({ socket, brightness = 20, led, ...other }) => {
  sendToSocket(socket, {
    action: 'command',
    led,
    brightness,
    ...other,
  });
};

const animation = ({ socket, led, animation, ...other }) => {
  sendToSocket(socket, {
    action: 'command',
    led,
    animation,
    ...other,
  });
};

const checkForUpdates = ({ socket }) => {
  sendToSocket(socket, {
    action: 'check-for-updates',
  });
};

const reboot = ({ socket }) => {
  sendToSocket(socket, {
    action: 'reboot',
  });
};

const send = ({ socket, ...message }) => {
  sendToSocket(socket, message);
};

const sendCommand = ({ socket, ...message }) => {
  sendToSocket(socket, {
    action: 'command',
    ...message,
  });
};

const setActiveLeds = ({ socket, activeLeds, ...other }) => {
  sendToSocket(socket, {
    action: 'command',
    activeLeds,
    ...other,
  });
};

module.exports = {
  color,
  brightness,
  initDual,
  init,
  animation,
  checkForUpdates,
  reboot,
  send,
  sendCommand,
  setActiveLeds,
};
