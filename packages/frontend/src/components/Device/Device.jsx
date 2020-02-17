import React, { useState } from 'react';
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
// import MenuItem from '@material-ui/core/MenuItem';
// import Select from '@material-ui/core/Select';
// import pink from '@material-ui/core/colors/indigo';
// import green from '@material-ui/core/colors/green';
// import DeviceIcon from '../DeviceIcon';
import Led from '../Led';

const styles = {
  root: {
    width: '100%',
    margin: '16px 0',
    backgroundColor: '#7c819a',
  },
  content: {
    width: '100%',
    maxWidth: 384,
    paddingBottom: 4,
  },
  list: {
    width: '100%',
    padding: 0,
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

const Device = ({ model, ip, mac, info, status, classes, ...other }) => {
  const { actions } = info;
  const [checked, setChecked] = useState([false, false]);
  const handleCheckLed = led => e => {
    setChecked(checked.map((c, i) => (i === led ? e.target.checked : c)));
  };

  return (
    <Card className={classes.root}>
      <CardContent className={classes.content}>
        <Typography variant="h4">
          {/* {model && <DeviceIcon model={model} />} {model} */}
          {model}
        </Typography>
        <Typography variant="subtitle1">
          {ip} | {mac}
        </Typography>
      </CardContent>
      <CardActions>
        <List className={classes.list}>
          {status &&
            status.map((ledStatus, led) => (
              <Led
                key={led}
                mac={mac}
                led={led}
                checked={checked[led]}
                onCheck={handleCheckLed}
                {...ledStatus}
                {...other}
                info={info}
              />
            ))}
          <ListItem>
            {actions && actions.find(act => act === 'reboot') && (
              <Mutation mutation={REBOOT} ignoreResults>
                {reboot => (
                  <Button onClick={() => reboot({ variables: { mac } })}>
                    Reboot
                  </Button>
                )}
              </Mutation>
            )}
            {actions && actions.find(act => act === 'check-for-updates') && (
              <Mutation mutation={CHECK_FOR_UPDATES} ignoreResults>
                {checkForUpdates => (
                  <Button
                    onClick={() => checkForUpdates({ variables: { mac } })}
                  >
                    Check For Updates
                  </Button>
                )}
              </Mutation>
            )}
          </ListItem>
        </List>
      </CardActions>
    </Card>
  );
};

export default withStyles(styles)(Device);
