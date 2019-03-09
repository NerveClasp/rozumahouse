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
    info: Info
    status: [Led]
  }
  type Info {
    actions: [String]
    commands: [String]
    animations: [String]
    leds: [Int]
    mode: [String]
  }
  type Led {
    activeLeds: Int
    brightness: Int
    mode: String
    animation: String
    animationDuration: Int
    ledOn: Boolean
    color: [Color]
  }
  input ColorInput {
    rgb: RgbInput
    hex: String
  }
  input RgbInput {
    r: Int!
    g: Int!
    b: Int!
  }
  type Color {
    rgb: Rgb
    hex: String
  }
  type Rgb {
    r: Int!
    g: Int!
    b: Int!
  }
  type Mutation {
    toggleLed(mac: String!, led: Int!, ledOn: Boolean!): Device
    changeLedAnimation(mac: String!, led: Int!, animation: String!): Device
    changeLedBrightness(mac: String!, led: Int!, brightness: Int!): Device
    changeLed(
      mac: String!
      led: Int!
      brightness: Int
      animation: String
      color: [ColorInput]
    ): Device
    reboot(mac: String!): Device
    checkForUpdates(mac: String!): Device
    setActiveLeds(mac: String!, led: Int!, activeLeds: Int!): Device
  }
`;
