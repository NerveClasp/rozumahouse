import React, { Component } from 'react';
import gql from 'graphql-tag';
import { Mutation } from 'react-apollo';
import { withStyles } from '@material-ui/core/styles';
import Card from '@material-ui/core/Card';
import CardActions from '@material-ui/core/CardActions';
import CardContent from '@material-ui/core/CardContent';
import Button from '@material-ui/core/Button';
import Typography from '@material-ui/core/Typography';
import List from '@material-ui/core/List';
import ListItem from '@material-ui/core/ListItem';
import MenuItem from '@material-ui/core/MenuItem';
import Select from '@material-ui/core/Select';
import pink from '@material-ui/core/colors/pink';
import green from '@material-ui/core/colors/green';
import DeviceIcon from '../DeviceIcon';
import Led from '../Led';

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
    const { model, ip, mac, name, info, status } = this.props;
    const { actions } = info;
    return (
      <Card>
        <CardContent>
          <Typography variant="h3">
            {model && <DeviceIcon model={model} />} {model}
          </Typography>
          <Typography variant="subheading">
            {ip} | {mac} | {name || 'no name'}
          </Typography>
        </CardContent>
        <CardActions>
          <List>
            {status &&
              status.map((ledStatus, led) => (
                <Led mac={mac} led={led} {...ledStatus} info={info} />
              ))}
            <ListItem>
              {actions && actions.find(act => act === 'reboot') && (
                <Mutation mutation={REBOOT} ignoreResults>
                  {reboot => (
                    <Button
                      onClick={e => {
                        reboot({ variables: { mac } });
                      }}
                    >
                      Reboot
                    </Button>
                  )}
                </Mutation>
              )}
              {actions && actions.find(act => act === 'check-for-updates') && (
                <Mutation mutation={CHECK_FOR_UPDATES} ignoreResults>
                  {checkForUpdates => (
                    <Button
                      onClick={e => {
                        checkForUpdates({ variables: { mac } });
                      }}
                    >
                      Check For Updates
                    </Button>
                  )}
                </Mutation>
              )}
            </ListItem>
          </List>
        </CardActions>

        {/* {activeLeds && <td>{activeLeds}</td>} */}
      </Card>
    );
  }
}

export default withStyles(styles)(Device);
