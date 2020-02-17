let devices = [];

module.exports = function() {
  this.updateDevices = newDevices => {
    devices = newDevices;
  };
  this.getDevices = () => devices;
  this.getDeviceByMac = mac => devices.find(d => d.mac === mac);
  this.getDeviceByIp = ip => devices.find(d => d.ip === ip);
  this.updateDevice = device => {
    devices = devices.map(dev => {
      if (dev.mac === device.mac || dev.ip === device.ip) {
        return device;
      }
      return dev;
    });
  };
};
