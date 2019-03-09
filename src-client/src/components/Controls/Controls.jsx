import React, { Component } from 'react';
import PropTypes from 'prop-types';
import { withStyles } from '@material-ui/core/styles';
import Chip from '@material-ui/core/Chip';
import Slider from '@material-ui/lab/Slider';
import { SketchPicker } from 'react-color';

const styles = {
  root: {},
  controls: {
    marginBottom: 8,
    minHeight: 32,
    display: 'flex',
    justifyContent: 'center',
    alignItems: 'center',
  },
  animations: {
    display: 'flex',
  },
  animation: {
    margin: '0 2px',
  },
};

class Controls extends Component {
  static propTypes = {
    kind: PropTypes.oneOf(['brightness', 'color', 'animation', '']).isRequired,
    brightness: PropTypes.number,
    color: PropTypes.array,
    animations: PropTypes.array,
    animation: PropTypes.string,
    onChange: PropTypes.func.isRequired,
  };

  static defaultProps = {
    brightness: 20,
    color: [],
    animation: '',
    animations: [],
  };

  handleColorChange = (newColor, index) => {
    const { color } = this.state;
    const updatedColor = color.map((c, i) => {
      if (index === i) return newColor;
      return c;
    });
    this.setState({ color: updatedColor });
  };

  handleBrightnessChange = (_, brightness) => {
    const { onChange } = this.props;
    onChange({ brightness });
  };

  handleAnimationClick = animation => {
    const { onChange } = this.props;
    onChange({ animation });
  };

  addColor = () => {
    const { color } = this.state;
    this.setState({ color: [...color, { hex: '#fff' }] });
  };

  renderControls = () => {
    const {
      kind,
      brightness,
      // color,
      animation,
      animations,
      classes,
    } = this.props;
    switch (kind) {
      case 'brightness':
        return (
          <Slider
            className={classes.brightness}
            value={brightness}
            min={0}
            max={124}
            step={1}
            // aria-labelledby="label"
            onChange={this.handleBrightnessChange}
          />
        );
      case 'animation':
        return (
          <div className={classes.animations}>
            {animations.map((a, i) => {
              return (
                <Chip
                  key={i}
                  className={classes.animation}
                  label={a}
                  onClick={() => this.handleAnimationClick(a)}
                  clickable
                  color={a === animation ? 'primary' : 'secondary'}
                />
              );
            })}
          </div>
        );
      default:
        return 'Control not found';
    }
  };
  renderColorPickers = () => {
    // const { color } = this.state;

    return (
      <div>
        <SketchPicker
          color={this.state.color.rgb}
          onChangeComplete={color => this.handleColorChange(color)}
        />
      </div>
    );
  };

  render() {
    const { classes, kind } = this.props;
    return (
      <div className={classes.root}>
        {kind && (
          <div className={classes.controls}>{this.renderControls()}</div>
        )}
      </div>
    );
  }
}

export default withStyles(styles)(Controls);
