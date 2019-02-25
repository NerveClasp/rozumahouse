/* eslint-disable no-console */
/*
[HTTP_USER_AGENT] => ESP8266-http-Update
[HTTP_X_ESP8266_STA_MAC] => 18:FE:AA:AA:AA:AA
[HTTP_X_ESP8266_AP_MAC] => 1A:FE:AA:AA:AA:AA
[HTTP_X_ESP8266_FREE_SPACE] => 671744
[HTTP_X_ESP8266_SKETCH_SIZE] => 373940
[HTTP_X_ESP8266_SKETCH_MD5] => a56f8ef78a0bebd812f62067daf1408a
[HTTP_X_ESP8266_CHIP_SIZE] => 4194304
[HTTP_X_ESP8266_SDK_VERSION] => 1.3.0
[HTTP_X_ESP8266_VERSION] => DOOR-7-g14f53a19
*/
/*
'user-agent': 'ESP8266-http-Update',
connection: 'close',
'x-esp8266-sta-mac': 'A0:20:A6:21:AF:33',
'x-esp8266-ap-mac': 'A2:20:A6:21:AF:33',
'x-esp8266-free-space': '593920',
'x-esp8266-sketch-size': '452336',
'x-esp8266-sketch-md5': 'cfd2a962b91ff96ef6b56816c76d465c',
'x-esp8266-chip-size': '4194304',
'x-esp8266-sdk-version': '3.0.0-dev(c0f7b44)',
'x-esp8266-mode': 'sketch'
*/
module.exports = (req, res) => {
  const { 'user-agent': userAgent } = req.headers;
  const allowedAgents = ['ESP8266-http-Update'];
  if (!allowedAgents.some(allowed => allowed === userAgent)) {
    res.writeHeader(403);
    res.end();
    return;
  }
  if (userAgent === 'ESP8266-http-Update') {
    const {
      'x-esp8266-free-space': freeSpace,
      'x-esp8266-sketch-md5': md5,
      'x-esp8266-sta-mac': mac,
      'x-esp8266-version': model,
    } = req.headers;
    if (!model) {
      console.log('MODEL/version is not specified.');
      res.writeHeader(304);
      res.end();
      return;
    }
    console.log(
      `Update request from ${model}, mac: ${mac}. Current sketch md5: ${md5}. Free space: ${freeSpace}`
    );
  }
  // console.log(req.get('user-agent'));
  // console.log(req.get('x-esp8266-free-space'));
  // console.log(req.get('x-esp8266-sketch-size'));
  // console.log(req.get('x-esp8266-sketch-md5'));
  // console.log(req.get('x-esp8266-sta-mac'));
  // console.log(req.get('x-esp8266-version'));
  // console.log(req.headers);

  // console.log(req);

  // var version = req.get('x-ESP8266-version');
  // if (version !== 'v3') {
  //   res.sendFile(`${__dirname}/statics/updates/xxxxxxxxxxxx.bin`);
  // } else {
  console.log('No new version');
  res.writeHeader(304);
  res.end();
};
