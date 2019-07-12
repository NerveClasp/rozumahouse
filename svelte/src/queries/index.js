import { gql } from "apollo-boost";

const GET_DEVICES = gql`
  {
    devices {
      model
      ip
      mac
      name
      info {
        actions
        animations
      }
      status {
        activeLeds
        brightness
        mode
        color {
          rgb {
            r
            g
            b
          }
        }
        animation
        animationDuration
        ledOn
      }
    }
  }
`;

const LED_CHANGE = gql`
  mutation ChangeLed(
    $mac: String!
    $led: Int!
    $brightness: Int
    $animation: String
    $color: [ColorInput]
  ) {
    changeLed(
      mac: $mac
      led: $led
      brightness: $brightness
      animation: $animation
      color: $color
    ) {
      status {
        brightness
        animation
        color {
          rgb {
            r
            g
            b
          }
        }
      }
    }
  }
`;

export { GET_DEVICES, LED_CHANGE };