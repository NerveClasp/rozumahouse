const randomColor = (min = 0, max = 254) =>
  Math.floor(Math.random() * (max - min + 1)) + min;

const sendToSocket = (socket, obj) => {
  socket.send(JSON.stringify(obj));
};

module.exports = { randomColor, sendToSocket };
