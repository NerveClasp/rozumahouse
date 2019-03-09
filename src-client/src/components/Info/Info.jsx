import React, { Component } from 'react';
import { withStyles } from '@material-ui/core/styles';
import InfoChip from '../InfoChip';

const styles = {
  root: {
    display: 'flex',
    maxWidth: 344,
  },
  colors: {
    display: 'flex',
  },
};

const Info = props => {
  const { brightness, color, animation, classes, onChipClick } = props;
  const renderColorChip = () => {
    const content = color.map(({ rgb }, i) => (
      <span
        key={i}
        style={{
          display: 'block',
          height: 16,
          width: 16,
          margin: '0 2px',
          borderRadius: '50%',
          backgroundColor: `rgb(${rgb.r}, ${rgb.g}, ${rgb.b})`,
        }}
      />
    ));
    return (
      <InfoChip
        label="color"
        content={<div className={classes.colors}>{content}</div>}
        onClick={onChipClick}
      />
    );
  };
  return (
    <div className={classes.root}>
      <InfoChip label="animation" content={animation} onClick={onChipClick} />
      <InfoChip label="brightness" content={brightness} onClick={onChipClick} />
      {color && renderColorChip()}
    </div>
  );
};

export default withStyles(styles)(Info);
