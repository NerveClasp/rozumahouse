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

const init = ({ socket, brightness = 20, ledName, animation }) => {
  sendToSocket(socket, {
    action: 'command',
    [ledName]: {
      mode,
      from,
      to,
    },
    brightness,
    animation: animation || 'back-and-forth',
  });
};

const initDual = ({ socket, brightness = 20, animation }) => {
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
  });
};

const gradientRgb = ({ socket, ledName, startColor, endColor }) => {
  sendToSocket(socket, {
    action: 'command',
    [ledName]: {
      mode,
      from: startColor || from,
      to: endColor || to,
    },
  });
};

const brightness = ({ socket, brightness = 20 }) => {
  sendToSocket(socket, {
    action: 'command',
    brightness,
  });
};

const animation = ({ socket, animation }) => {
  sendToSocket(socket, {
    action: 'command',
    animation,
  });
};

module.exports = { off, gradientRgb, brightness, initDual, init, animation };
