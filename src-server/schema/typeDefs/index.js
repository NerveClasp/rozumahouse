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
    left: [String]
    right: [String]
    activeLeds: Int
  }
  type Mutation {
    turnLedOn(mac: String!, which: String!): Device
    turnLedOff(mac: String!, which: String!): Device
    changeLedAnimation(mac: String!, animation: String!): Device
    changeLedBrightness(mac: String!, brightness: String!): Device
    reboot(mac: String!): Device
    checkForUpdates(mac: String!): Device
    setActiveLeds(mac: String!, activeLeds: Int!): Device
  }
`;
