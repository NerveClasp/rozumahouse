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

const off = ({ socket, ledName }) => {
  sendToSocket(socket, {
    action: 'command',
    [ledName]: {
      mode: 'off',
    },
    brightness: 0,
    animation: 'off',
  });
};

const init = ({ socket, brightness = 20, ledName, animation, ...other }) => {
  sendToSocket(socket, {
    action: 'command',
    [ledName]: {
      mode,
      from,
      to,
    },
    brightness,
    animation: animation || 'back-and-forth',
    ...other,
  });
};

const initDual = ({ socket, brightness = 20, animation, ...other }) => {
  sendToSocket(socket, {
    action: 'command',
    left: {
      mode,
      from,
      to,
    },
    right: {
      mode,
      from,
      to,
    },
    brightness,
    animation: animation || 'back-and-forth',
    ...other,
  });
};

const gradientRgb = ({ socket, ledName, startColor, endColor, ...other }) => {
  sendToSocket(socket, {
    action: 'command',
    [ledName]: {
      mode: 'gradient_rgb',
      from: startColor || from,
      to: endColor || to,
    },
    ...other,
  });
};

const brightness = ({ socket, brightness = 20, ...other }) => {
  sendToSocket(socket, {
    action: 'command',
    brightness,
    ...other,
  });
};

const animation = ({ socket, animation, ...other }) => {
  sendToSocket(socket, {
    action: 'command',
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

const setActiveLeds = ({ socket, activeLeds, ...other }) => {
  sendToSocket(socket, {
    action: 'command',
    activeLeds,
    ...other,
  });
};

module.exports = {
  off,
  gradientRgb,
  brightness,
  initDual,
  init,
  animation,
  checkForUpdates,
  reboot,
  send,
  setActiveLeds,
};
