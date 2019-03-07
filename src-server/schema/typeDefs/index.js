const { gql } = require('apollo-server-express');

module.exports = gql`
  type Query {
    device(ip: String, mac: String): Device
    devices(model: String): [Device]
  }
  type Device {
    name: String
    room: String
    mac: String
    ip: String
    chipId: String
    model: String
    freeSketchSpace: Int
    coreVersion: String
    sdkVersion: String
    action: [String]
    command: [String]
    animation: [String]
    which: [Int]
    mode: [String]
    activeLeds: Int
  }
  type Mutation {
    turnLedOn(mac: String!, which: Int!): Device
    turnLedOff(mac: String!, which: Int!): Device
    changeLedAnimation(mac: String!, which: Int!, animation: String!): Device
    changeLedBrightness(mac: String!, which: Int!, brightness: String!): Device
    reboot(mac: String!): Device
    checkForUpdates(mac: String!): Device
    setActiveLeds(mac: String!, which: Int!, activeLeds: Int!): Device
  }
`;
