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
import Slider from '@material-ui/lab/Slider';

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

const TOGGLE = gql`
  mutation ToggleLed($mac: String!, $led: Int!, $ledOn: Boolean!) {
    toggleLed(mac: $mac, led: $led, ledOn: $ledOn) {
      mac
    }
  }
`;

const ANIMATION_CHANGE = gql`
  mutation ChangeLedAnimation($mac: String!, $led: Int!, $animation: String!) {
    changeLedAnimation(mac: $mac, led: $led, animation: $animation) {
      mac
    }
  }
`;

const BRIGHTNESS_CHANGE = gql`
  mutation ChangeLedBrightness($mac: String!, $led: Int!, $brightness: Int!) {
    changeLedBrightness(mac: $mac, led: $led, brightness: $brightness) {
      mac
    }
  }
`;

class Device extends Component {
  state = {
    animation: this.props.animation || 'none',
    brightness: this.props.brightness || 20,
  };

  handleBrightnessChange = (e, brightness, fn) => {
    const { mac, led } = this.props;
    this.setState({ brightness });
    console.log(brightness);
    console.log(typeof brightness);
    fn({
      variables: { mac, led, brightness },
    });
  };
  render() {
    const { mac, animation, led, info } = this.props;
    console.log(this.props);
    return (
      <ListItem>
        {info.animations ? (
          <Mutation mutation={ANIMATION_CHANGE} ignoreResults>
            {(changeLedAnimation, { data }) => (
              <Select
                value={this.state.animation}
                onChange={e => {
                  const { value } = e.target;
                  this.setState({ animation: value });
                  changeLedAnimation({
                    variables: { mac, led, animation: value },
                  });
                }}
              >
                {info.animations.map((anim, i) => (
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
        <Mutation mutation={TOGGLE} ignoreResults>
          {(toggleLed, { data }) => (
            <>
              <Button
                onClick={e => {
                  toggleLed({ variables: { mac, led, ledOn: true } });
                }}
              >
                Turn ON
              </Button>
              <Button
                onClick={e => {
                  toggleLed({ variables: { mac, led, ledOn: false } });
                }}
              >
                Turn OFF
              </Button>
            </>
          )}
        </Mutation>
        <Mutation mutation={BRIGHTNESS_CHANGE} ignoreResults>
          {(changeLedBrightness, { data }) => (
            <Slider
              value={this.state.brightness}
              min={0}
              max={254}
              step={1}
              // aria-labelledby="label"
              onChange={(e, value) =>
                this.handleBrightnessChange(e, value, changeLedBrightness)
              }
            />
          )}
        </Mutation>
      </ListItem>
    );
  }
}

export default withStyles(styles)(Device);
