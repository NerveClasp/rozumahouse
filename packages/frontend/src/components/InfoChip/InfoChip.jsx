import React from 'react';
import { withStyles } from '@material-ui/core/styles';
import Typography from '@material-ui/core/Typography';
import Chip from '@material-ui/core/Chip';
// import pink from '@material-ui/core/colors/pink';
// import green from '@material-ui/core/colors/green';

const styles = {
  root: {
    margin: '0 4px',
  },
  label: {
    textAlign: 'center',
    textTransform: 'uppercase',
    fontSize: '10px',
  },
  chip: {
    width: '100%',
  },
};

const InfoChip = props => {
  const { classes, label, content, onClick } = props;
  const handleChipClick = () => {
    onClick(label);
  };
  return (
    <div className={classes.root}>
      <Typography className={classes.label}>{label}</Typography>
      <Chip
        label={content}
        className={classes.chip}
        onClick={handleChipClick}
        // color="secondary"
      />
    </div>
  );
};

export default withStyles(styles)(InfoChip);
