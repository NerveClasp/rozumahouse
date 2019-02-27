const { led } = require('../../socket');
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
  if (!model) return devices;
  return devices.filter(device => device.model === model);
};

const turnLedOff = (_, { mac, which }) => {
  const { socket } = devs.getDeviceByMac(mac);
  if (which === 'both') {
    led.off({ socket, ledName: 'left' });
    led.off({ socket, ledName: 'right' });
  } else {
    led.off({ socket, ledName: which });
  }
  return devs.getDeviceByMac(mac);
};

const turnLedOn = (_, args) => {
  const { mac, which, ...other } = args;
  const { socket } = devs.getDeviceByMac(mac);
  if (which === 'both') {
    led.initDual({ socket, ...other });
  }
  led.init({ socket, ledName: which, ...other });
  return devs.getDeviceByMac(mac);
};

const changeLedAnimation = (_, { mac, animation }) => {
  const { socket } = devs.getDeviceByMac(mac);
  led.animation({ socket, animation });
  return devs.getDeviceByMac(mac);
};

const changeLedBrightness = (_, { mac, brightness }) => {
  const { socket } = devs.getDeviceByMac(mac);
  led.brightness({ socket, brightness });
  return devs.getDeviceByMac(mac);
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
