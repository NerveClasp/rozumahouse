import React, { Component } from 'react';
import gql from 'graphql-tag';
import { Mutation } from 'react-apollo';
import { withStyles } from '@material-ui/core/styles';
import Card from '@material-ui/core/Card';
import CardContent from '@material-ui/core/CardContent';
// import Checkbox from '@material-ui/core/Checkbox';
import Info from '../Info';
import Controls from '../Controls';

const styles = {
  listItem: {
    alignItems: 'baseline',
    margin: '4px 0',
  },
  content: {
    display: 'flex',
    alignItems: 'center',
    justifyContent: 'center',
    padding: '4px 8px',
    flexDirection: 'column',
  },
  wrapper: {
    display: 'flex',
  },
  select: {
    width: '100%',
    padding: 8,
  },
  brightnessWrapper: {
    width: '100%',
    height: '100%',
  },
  brightness: {
    width: '100%',
    padding: 16,
  },
};

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

class Led extends Component {
  state = {
    changeKind: '',
    animation: this.props.animation || 'none',
    brightness: this.props.brightness || 20,
    color: this.props.color || [],
  };

  // TODO: getDerivedStateFromProps

  handleBrightnessChange = (e, brightness) => {
    this.setState({ brightness });
    this.applyMutation({ brightness });
  };

  handleAnimationChange = e => {
    const { value } = e.target;

    this.setState({ animation: value });
    this.applyMutation({ animation: value });
  };

  applyMutation = (changes, mutate) => {
    const { mac, led } = this.props;
    mutate({
      variables: { mac, led, ...changes },
      refetchQueries: true,
    });
  };

  handleColorChange = (newColor, index) => {
    const { color } = this.state;
    const updatedColor = color.map((c, i) => {
      if (index === i) return newColor;
      return c;
    });
    this.setState({
      color: updatedColor,
    });
  };

  handleControlsChange = (change, mutation) => {
    this.setState({ ...change });
    this.applyMutation(change, mutation);
  };

  handleInfoClick = kind => {
    const { changeKind } = this.state;
    this.setState({ changeKind: kind === changeKind ? '' : kind });
  };

  render() {
    const {
      // led,
      info,
      // checked,
      // onCheck,
      classes,
      copiedColor,
      setCopiedColor,
      copiedGradient,
      setCopiedGradient,
    } = this.props;
    const { color, brightness, animation, changeKind } = this.state;

    return (
      <Card className={classes.listItem}>
        <Mutation mutation={LED_CHANGE}>
          {changeLed => (
            <CardContent className={classes.content}>
              <div className={classes.wrapper}>
                {/* <Checkbox checked={checked} onChange={onCheck(led)} /> */}
                <div className="settings">
                  <Controls
                    kind={changeKind}
                    brightness={brightness}
                    animation={animation}
                    animations={info.animations}
                    color={color}
                    copiedColor={copiedColor}
                    setCopiedColor={setCopiedColor}
                    copiedGradient={copiedGradient}
                    setCopiedGradient={setCopiedGradient}
                    onChange={change =>
                      this.handleControlsChange(change, changeLed)
                    }
                  />
                  <Info
                    brightness={brightness}
                    animation={animation}
                    color={color}
                    onChipClick={this.handleInfoClick}
                  />
                </div>
              </div>
            </CardContent>
          )}
        </Mutation>
      </Card>
    );
  }
}

export default withStyles(styles)(Led);
