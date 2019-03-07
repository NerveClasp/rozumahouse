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

const off = ({ socket, which }) => {
  sendToSocket(socket, {
    action: 'command',
    which,
    mode: 'off',
    brightness: 0,
    animation: 'off',
  });
};

const init = ({ socket, brightness = 20, which, animation, ...other }) => {
  sendToSocket(socket, {
    action: 'command',
    which,
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
    which: 0,
    mode,
    from,
    to,
    brightness,
    animation: animation || 'back-and-forth',
    ...other,
  });
  sendToSocket(socket, {
    action: 'command',
    which: 1,
    mode,
    from,
    to,
    brightness,
    animation: animation || 'back-and-forth',
    ...other,
  });
};

const gradientRgb = ({ socket, which, startColor, endColor, ...other }) => {
  sendToSocket(socket, {
    action: 'command',
    which,
    mode: 'gradient_rgb',
    from: startColor || from,
    to: endColor || to,
    ...other,
  });
};

const brightness = ({ socket, brightness = 20, which, ...other }) => {
  sendToSocket(socket, {
    action: 'command',
    which,
    brightness,
    ...other,
  });
};

const animation = ({ socket, which, animation, ...other }) => {
  sendToSocket(socket, {
    action: 'command',
    which,
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
