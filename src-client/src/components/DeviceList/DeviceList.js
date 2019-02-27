import React from 'react';

import gql from 'graphql-tag';
import { Query } from 'react-apollo';
import Device from '../Device';
import classes from './DeviceList.module.scss';

const GET_DEVICES = gql`
  {
    devices {
      model
      ip
      mac
      name
      action
      command
      animation
      left
      right
    }
  }
`;

const DeviceList = props => (
  <Query query={GET_DEVICES} pollInterval={500}>
    {({ loading, error, data }) => {
      if (loading) return 'Loading...';
      if (error) return `Error! ${error.message}`;

      return (
        <table className={classes.table}>
          <thead>
            <tr>
              <td>Model</td>
              <td>IP</td>
              <td>MAC</td>
              <td>Name</td>
            </tr>
          </thead>
          <tbody>
            {data.devices &&
              data.devices.map((device, i) => (
                <Device device={device} key={i} />
              ))}
          </tbody>
        </table>
      );
    }}
  </Query>
);

export default DeviceList;
