const low = require('lowdb');
const FileSync = require('lowdb/adapters/FileSync');

const adapter = new FileSync('devices.json');
const db = low(adapter);

// Set some defaults (required if your JSON file is empty)
db.defaults({ devices: [] }).write();

// Add a post
db.get('devices')
  .push({ id: 1, title: 'lowdb is awesome' })
  .write();

db.get('devices')
  .find({ id: 1 })
  .assign({ lol: 'nice' })
  .write();

db.get('devices')
  .find({ id: 1 })
  .assign({ lol: 'even better' })
  .write();

console.log(
  db
    .get('devices')
    .find({ id: 2 })
    .value()
);
db.get('devices')
  .find({ id: 2 })
  .assign({ but: 'can you do this?' })
  .write();
// // Set a user using Lodash shorthand syntax
// db.set('user.name', 'typicode').write();

// // Increment count
// db.update('count', n => n + 1).write();

module.exports = { db };
