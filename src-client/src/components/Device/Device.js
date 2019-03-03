import React, { Component } from 'react';
import gql from 'graphql-tag';
import { Mutation } from 'react-apollo';
import { withStyles } from '@material-ui/core/styles';
import pink from '@material-ui/core/colors/pink';
import green from '@material-ui/core/colors/green';
import DeviceIcon from '../DeviceIcon';

const styles = {
  avatar: {
    margin: 10,
  },
  pinkAvatar: {
    margin: 10,
    color: '#fff',
    backgroundColor: pink[500],
  },
  greenAvatar: {
    margin: 10,
    color: '#fff',
    backgroundColor: green[500],
  },
};

const TURN_ON = gql`
  mutation TurnLedOn($mac: String!, $which: String!) {
    turnLedOn(mac: $mac, which: $which) {
      mac
    }
  }
`;

const TURN_OFF = gql`
  mutation TurnLedOff($mac: String!, $which: String!) {
    turnLedOff(mac: $mac, which: $which) {
      mac
    }
  }
`;

const ANIMATION_CHANGE = gql`
  mutation ChangeLedAnimation($mac: String!, $animation: String!) {
    changeLedAnimation(mac: $mac, animation: $animation) {
      mac
    }
  }
`;

const REBOOT = gql`
  mutation Reboot($mac: String!) {
    reboot(mac: $mac) {
      mac
    }
  }
`;

const CHECK_FOR_UPDATES = gql`
  mutation CheckForUpdates($mac: String!) {
    checkForUpdates(mac: $mac) {
      mac
    }
  }
`;

class Device extends Component {
  render() {
    const { device } = this.props;
    const { model, ip, mac, name, animation, action, activeLeds } = device;
    return (
      <tr>
        <td>{model && <DeviceIcon model={model} />}</td>
        <td>{model}</td>
        <td>{ip}</td>
        <td>{mac}</td>
        <td>{name || 'no name'}</td>
        <td>
          {animation ? (
            <Mutation mutation={ANIMATION_CHANGE} ignoreResults>
              {(changeLedAnimation, { data }) => (
                <select
                  name="animation"
                  onChange={e => {
                    const { mac } = this.props.device;
                    const { value } = e.target;
                    changeLedAnimation({
                      variables: { mac, animation: value },
                    });
                  }}
                >
                  <option value="none">none</option>
                  {animation.map((anim, i) => (
                    <option key={i} value={anim}>
                      {anim}
                    </option>
                  ))}
                </select>
              )}
            </Mutation>
          ) : (
            '--'
          )}
        </td>
        <td>
          <Mutation mutation={TURN_ON} ignoreResults>
            {(turnLedOn, { data }) => (
              <button
                onClick={e => {
                  const { mac } = this.props.device;
                  turnLedOn({ variables: { mac, which: 'both' } });
                }}
              >
                Turn ON
              </button>
            )}
          </Mutation>
          <Mutation mutation={TURN_OFF} ignoreResults>
            {(turnLedOff, { data }) => (
              <button
                onClick={e => {
                  const { mac } = this.props.device;
                  turnLedOff({ variables: { mac, which: 'both' } });
                }}
              >
                Turn OFF
              </button>
            )}
          </Mutation>
        </td>
        {action && action.find(act => act === 'reboot') && (
          <Mutation mutation={REBOOT} ignoreResults>
            {reboot => (
              <button
                onClick={e => {
                  const { mac } = this.props.device;
                  reboot({ variables: { mac } });
                }}
              >
                Reboot
              </button>
            )}
          </Mutation>
        )}
        {action && action.find(act => act === 'check-for-updates') && (
          <Mutation mutation={CHECK_FOR_UPDATES} ignoreResults>
            {checkForUpdates => (
              <button
                onClick={e => {
                  const { mac } = this.props.device;
                  checkForUpdates({ variables: { mac } });
                }}
              >
                Check For Updates
              </button>
            )}
          </Mutation>
        )}
        {activeLeds && <td>{activeLeds}</td>}
      </tr>
    );
  }
}

export default withStyles(styles)(Device);
