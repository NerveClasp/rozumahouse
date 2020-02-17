const { ledStrip } = require('../../socket');
const { rgb } = require('../../utils');
const StoredDevices = require('../../databaseless');
const devs = new StoredDevices();

const getDevice = (_, args) => {
  const { mac, ip } = args;
  if (!mac) return devs.getDeviceByIp(ip); // TODO: rework/rethink
  return devs.getDeviceByMac(mac);
};

const getDevices = (_, args) => {
  const { model } = args;
  const devices = devs.getDevices();
  // devices.forEach(dev => {
  //   const { socket, ...other } = dev;
  //   // console.log(other);
  // });
  // console.log(devices);
  if (!model) return devices;
  return devices.filter(device => device.model === model);
};

const turnLedOff = (_, { mac, led }) => {
  const device = devs.getDeviceByMac(mac);
  const { socket } = device;
  if (led === 'both') {
    led.off({ socket, led: 0 });
    led.off({ socket, led: 1 });
  } else {
    led.off({ socket, led });
  }
  devs.updateDevice(device);
  return device;
};

const toggleLed = (_, args) => {
  const { mac, led, ledOn, ...other } = args;
  const device = devs.getDeviceByMac(mac);
  const { socket } = device;
  device.status[led] = { ...device.status[led], ledOn, ...other };
  if (ledOn) {
    ledStrip.send({
      socket,
      led,
      ledOn,
      action: 'command',
      ...device.status[led],
    });
  } else {
    ledStrip.send({ socket, led, ledOn, action: 'command' });
  }
  devs.updateDevice(device);
  return device;
};

const changeLedAnimation = (_, { mac, led, animation }) => {
  const device = devs.getDeviceByMac(mac);
  const { socket } = device;
  ledStrip.animation({ socket, led, animation });
  device.status[led].animation = animation;
  devs.updateDevice(device);
  return device;
};

const changeLedBrightness = (_, { mac, led, brightness }) => {
  const device = devs.getDeviceByMac(mac);
  const { socket } = device;
  ledStrip.brightness({ socket, brightness, led });
  device.status[led].brightness = brightness;
  devs.updateDevice(device);
  return device;
};

const reboot = (_, { mac }) => {
  const device = devs.getDeviceByMac(mac);
  const { socket } = device;
  ledStrip.reboot({ socket });
  return device;
};

const checkForUpdates = (_, { mac }) => {
  const device = devs.getDeviceByMac(mac);
  const { socket } = device;
  ledStrip.checkForUpdates({ socket });
  return device;
};

const setActiveLeds = (_, { mac, led, activeLeds, ...other }) => {
  const device = devs.getDeviceByMac(mac);
  const { socket } = device;
  ledStrip.setActiveLeds({ socket, led, activeLeds, ...other });
  return device;
};

/*
mac: String!
      led: Int!
      brightness: Int
      mode: String
      color: [Int]
      animation: String
*/
const changeLed = (_, message) => {
  const { mac, led, ...status } = message;
  const device = devs.getDeviceByMac(mac);
  const { socket } = device;
  device.status[led] = { ...device.status[led], ...status };
  ledStrip.sendCommand({
    socket,
    ...message,
  });
  devs.updateDevice(device);
  return device;
};

module.exports = {
  Query: {
    device: getDevice,
    devices: getDevices,
  },
  Mutation: {
    toggleLed,
    changeLedAnimation,
    changeLedBrightness,
    changeLed,
    reboot,
    checkForUpdates,
    setActiveLeds,
  },
};
