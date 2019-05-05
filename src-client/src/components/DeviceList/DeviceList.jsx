import React, { useState } from 'react';
import { Mutation } from 'react-apollo';
import gql from 'graphql-tag';
import { Query } from 'react-apollo';
import Device from '../Device';
// import classes from './DeviceList.module.scss';

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

const DeviceList = props => {
  const [copiedColor, setCopiedColor] = useState();
  const [copiedGradient, setCopiedGradient] = useState();
  const copyControls = {
    copiedColor,
    setCopiedColor,
    copiedGradient,
    setCopiedGradient,
  };

  return (
    <Query query={GET_DEVICES} pollInterval={500}>
      {({ loading, error, data }) => {
        if (loading) return 'Loading...';
        if (error) return `Error! ${error.message}`;

        return (
          <div>
            <Mutation mutation={LED_CHANGE}>
              {changeLed => (
                <>
                  {data.devices &&
                    data.devices.map(
                      (device, i) =>
                        device.mac && (
                          <Device
                            {...device}
                            {...copyControls}
                            key={i}
                            changeLed={changeLed}
                          />
                        )
                    )}
                </>
              )}
            </Mutation>
          </div>
        );
      }}
    </Query>
  );
};

export default DeviceList;
