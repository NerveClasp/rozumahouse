const { led } = require('../../socket');
const StoredDevices = require('../../databaseless');
const devs = new StoredDevices();

const getDevice = (_, args) => {
  const { mac, ip } = args;
  if (!mac) return devs.getDeviceByIp(ip); // TODO: rework/rethink
  return device;
};

const getDevices = (_, args) => {
  const { model } = args;
  const devices = devs.getDevices();
  devices.forEach(dev => {
    const { socket, ...other } = dev;
    console.log(other);
  });
  // console.log(devices);
  if (!model) return devices;
  return devices.filter(device => device.model === model);
};

const turnLedOff = (_, { mac, which }) => {
  const device = devs.getDeviceByMac(mac);
  const { socket } = device;
  if (which === 'both') {
    device.state = { ...device.state, left: 'off', right: 'off' };
    led.off({ socket, ledName: 'left' });
    led.off({ socket, ledName: 'right' });
  } else {
    device.state[which] = 'off';
    led.off({ socket, ledName: which });
  }
  devs.updateDevice(device);
  return device;
};

const turnLedOn = (_, args) => {
  const { mac, which, ...other } = args;
  const device = devs.getDeviceByMac(mac);
  const { socket } = device;
  if (which === 'both') {
    led.initDual({ ...other, ...device });
    device.state = { ...device.state, left: 'on', right: 'on' };
  } else {
    device.state[which] = 'on';
    led.init({ socket, ledName: which, ...other, ...device });
  }
  devs.updateDevice(device);
  return device;
};

const changeLedAnimation = (_, { mac, animation }) => {
  const device = devs.getDeviceByMac(mac);
  const { socket } = device;
  led.animation({ socket, animation });
  device.curAnimation = animation;
  devs.updateDevice(device);
  return device;
};

const changeLedBrightness = (_, { mac, brightness }) => {
  const device = devs.getDeviceByMac(mac);
  const { socket } = device;
  led.brightness({ socket, brightness });
  devs.updateDevice(device);
  return device;
};

module.exports = {
  Query: {
    device: getDevice,
    devices: getDevices,
  },
  Mutation: {
    turnLedOn,
    turnLedOff,
    changeLedAnimation,
    changeLedBrightness,
  },
};
