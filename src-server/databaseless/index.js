let devices = [];

module.exports = function() {
  this.updateDevices = newDevices => {
    devices = newDevices;
  };
  this.getDevices = () => devices;
  this.getDeviceByMac = mac => devices.find(d => d.mac === mac);
  this.getDeviceByIp = ip => devices.find(d => d.ip === ip);
  // this.updateDevice = device => {
  //   devices = devices.map(d => {
  //     if (d.mac !== device.mac) return d;
  //     return { ...d, ...device };
  //   });
  // };
};
