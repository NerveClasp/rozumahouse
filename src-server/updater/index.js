/* eslint-disable no-console */
const fs = require('fs');
const md5File = require('md5-file');
const pathNode = require('path');

const update = (req, res) => {
  const { 'user-agent': userAgent } = req.headers;
  const allowedAgents = ['ESP8266-http-Update'];
  if (!allowedAgents.some(allowed => allowed === userAgent)) {
    res.writeHeader(403);
    res.end();
    return;
  }
  const bins = getFileList();
  if (userAgent === 'ESP8266-http-Update') {
    const {
      'x-esp8266-free-space': freeSpace,
      'x-esp8266-sketch-md5': md5,
      'x-esp8266-sta-mac': mac,
      'x-esp8266-version': model,
    } = req.headers;
    if (!model) {
      console.log('MODEL/version is not specified.');
      res.writeHeader(403, { 'Content-Type': 'text/plain' });
      res.write('403 Forbidden');
      res.end();
      return;
    }
    console.log(
      `Update request from ${model}, mac: ${mac}. Current sketch md5: ${md5}. Free space: ${freeSpace}`
    );
    if (!bins[model]) {
      console.log(`No bins for model ${model}`);
      res.writeHeader(404, { 'Content-Type': 'text/plain' });
      res.write('404 Not Found\n');
      res.end();
      return;
    }
    const { hash, size, path, fileName } = bins[model];
    console.log(`existing bin data: ${hash}, path: ${path}`);
    if (freeSpace < size) {
      console.log('Not enough free space for the new sketch');
      res.writeHeader(403, { 'Content-Type': 'text/plain' });
      res.write('403 Forbidden');
      res.end();
      return;
    }
    if (hash === md5) {
      console.log('No new version');
      res.writeHeader(304, { 'Content-Type': 'text/plain' });
      res.write('304 Not Modified\n');
      res.end();
      return;
    } else {
      console.log('Updating...');

      fs.readFile(path, 'binary', (err, file) => {
        if (err) {
          res.writeHeader(500, { 'Content-Type': 'text/plain' });
          res.write(`${err}\n`);
          res.end();
        } else {
          res.writeHeader(200, {
            'Content-Type': 'application/octet-stream',
            'Content-Disposition': `attachment;filename=${fileName}`,
            'Content-Length': `${size}`,
            'x-MD5': md5File.sync(path),
          });
          res.write(file, 'binary');
          res.end();
        }
      });
    }
  }
};

const getFileList = () => {
  let files = {};
  let path;
  console.log(pathNode.resolve(__dirname, '../../dist-arduino'));
  if (fs.existsSync(pathNode.resolve(__dirname, '../bins'))) {
    // prod build/higher priority
    path = pathNode.resolve(__dirname, '../bins');
  } else if (fs.existsSync(pathNode.resolve(__dirname, '../../dist-arduino'))) {
    // not a prod build/lower priority
    path = pathNode.resolve(__dirname, '../../dist-arduino');
  }

  if (!path) return files;

  fs.readdirSync(path)
    .filter(file => file.indexOf('.bin') > 0)
    .forEach(file => {
      const [name] = file.split('.');
      const filePath = pathNode.resolve(path, file);
      const { size } = fs.statSync(filePath);
      const hash = md5File.sync(filePath);
      // stats.file
      files[name] = {
        fileName: name,
        path: filePath,
        size,
        hash,
      };
    });
  return files;
};

module.exports = { update, getFileList };
