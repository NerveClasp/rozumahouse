const randomColor = (min = 0, max = 254) =>
  Math.floor(Math.random() * (max - min + 1)) + min;

const sendToSocket = (socket, obj) => {
  if (socket.readyState !== 3) {
    socket.send(JSON.stringify(obj));
  } else {
    console.warn('socket is not open');
  }
};

module.exports = { randomColor, sendToSocket };
