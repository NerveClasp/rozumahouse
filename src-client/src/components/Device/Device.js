import React, { Component } from 'react';
import gql from 'graphql-tag';
import { Mutation } from 'react-apollo';

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

export default class Device extends Component {
  render() {
    const { device } = this.props;
    const { model, ip, mac, name, animation } = device;
    return (
      <tr>
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
      </tr>
    );
  }
}
