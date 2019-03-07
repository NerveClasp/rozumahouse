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
  mutation TurnLedOn($mac: String!, $which: Int!) {
    turnLedOn(mac: $mac, which: $which) {
      mac
    }
  }
`;

const TURN_OFF = gql`
  mutation TurnLedOff($mac: String!, $which: Int!) {
    turnLedOff(mac: $mac, which: $which) {
      mac
    }
  }
`;

const ANIMATION_CHANGE = gql`
  mutation ChangeLedAnimation(
    $mac: String!
    $which: Int!
    $animation: String!
  ) {
    changeLedAnimation(mac: $mac, which: $which, animation: $animation) {
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
    const {
      model,
      ip,
      mac,
      name,
      animation,
      action,
      activeLeds,
      which,
    } = device;
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
            {which &&
              which.map((w, which) => (
                <ListItem>
                  {animation ? (
                    <Mutation mutation={ANIMATION_CHANGE} ignoreResults>
                      {(changeLedAnimation, { data }) => (
                        <Select
                          // value={this.state.age}
                          onChange={e => {
                            const { mac } = this.props.device;
                            const { value } = e.target;
                            changeLedAnimation({
                              variables: { mac, which, animation: value },
                            });
                          }}
                        >
                          {animation.map((anim, i) => (
                            <MenuItem key={i} value={anim}>
                              {anim}
                            </MenuItem>
                          ))}
                        </Select>
                      )}
                    </Mutation>
                  ) : (
                    '--'
                  )}
                  <Mutation mutation={TURN_ON} ignoreResults>
                    {(turnLedOn, { data }) => (
                      <Button
                        onClick={e => {
                          const { mac } = this.props.device;
                          turnLedOn({ variables: { mac, which } });
                        }}
                      >
                        Turn ON
                      </Button>
                    )}
                  </Mutation>
                  <Mutation mutation={TURN_OFF} ignoreResults>
                    {(turnLedOff, { data }) => (
                      <Button
                        onClick={e => {
                          const { mac } = this.props.device;
                          turnLedOff({ variables: { mac, which } });
                        }}
                      >
                        Turn OFF
                      </Button>
                    )}
                  </Mutation>
                </ListItem>
              ))}
            <ListItem>
              {action && action.find(act => act === 'reboot') && (
                <Mutation mutation={REBOOT} ignoreResults>
                  {reboot => (
                    <Button
                      onClick={e => {
                        const { mac } = this.props.device;
                        reboot({ variables: { mac } });
                      }}
                    >
                      Reboot
                    </Button>
                  )}
                </Mutation>
              )}
              {action && action.find(act => act === 'check-for-updates') && (
                <Mutation mutation={CHECK_FOR_UPDATES} ignoreResults>
                  {checkForUpdates => (
                    <Button
                      onClick={e => {
                        const { mac } = this.props.device;
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

        {activeLeds && <td>{activeLeds}</td>}
      </Card>
    );
  }
}

export default withStyles(styles)(Device);
