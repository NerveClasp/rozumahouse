import React from 'react';
import Avatar from '@material-ui/core/Avatar';
import BulbIcon from '@material-ui/icons/BrightnessHighRounded';
import DefaultIcon from '@material-ui/icons/BubbleChart';

const DeviceIcon = props => {
  const { model, icon } = props;
  const renderIcon = () => {
    if (icon) return icon;
    if (model.indexOf('led') > 0) {
      return <BulbIcon />;
    }
    return <DefaultIcon />;
  };
  return <Avatar>{renderIcon()}</Avatar>;
};

export default DeviceIcon;
