import React, { useState } from 'react';
import { withStyles } from '@material-ui/core/styles';
import Fab from '@material-ui/core/Fab';
import Button from '@material-ui/core/Button';
import AddIcon from '@material-ui/icons/Add';
import SvgIcon from '@material-ui/core/SvgIcon';
import EditIcon from '@material-ui/icons/Edit';
import ApplyIcon from '@material-ui/icons/Done';
import DeleteIcon from '@material-ui/icons/Delete';
import CopyIcon from '@material-ui/icons/FileCopy';
import NavigationIcon from '@material-ui/icons/Navigation';
import { ChromePicker } from 'react-color';
import { omit } from 'lodash';
import { wrapper, pickerWrapper } from './LedColorPicker.module.scss';

const styles = theme => ({
  fab: {
    margin: theme.spacing.unit,
    width: 36,
    height: 36,
  },
  extendedIcon: {
    marginRight: theme.spacing.unit,
  },
});

const LedColorPicker = ({ color, classes, onChange }) => {
  const [colors, setColors] = useState(
    Array.apply(null, Array(4)).map((c, i) =>
      color[i] ? { rgb: omit(color[i].rgb, ['__typename']) } : {}
    )
  );
  const [currentColor, setCurrentColor] = useState(colors[0]);
  const [copiedColor, setCopyColor] = useState();
  const [currentColorIndex, setCurrentColorIndex] = useState(-1);
  const [isColorPickerOpen, toggleColorPicker] = useState(false);
  const applyColorChange = ({ rgb }) => {
    const { r, g, b } = rgb;
    const color = { rgb: { r, g, b } };
    setCurrentColor(color);
    updateColors(color, currentColorIndex);
  };

  const updateColors = (color, index) =>
    setColors(colors.map((c, i) => (i === index ? color : c)));

  const toggleColor = (index, color) => {
    const finalColor = color.rgb ? color : currentColor;
    setCurrentColorIndex(index);
    setCurrentColor(finalColor);
    updateColors(finalColor, index);
    toggleColorPicker(true);
  };

  const applyColors = () => {
    toggleColorPicker(false);
    onChange(colors.filter(c => c.rgb));
  };

  const clearColor = () => {
    updateColors({}, currentColorIndex);
    toggleColorPicker(false);
  };

  const copyColor = () => {
    setCopyColor(currentColor);
  };

  const pasteColor = () => {
    setCurrentColor(copiedColor);
    updateColors(copiedColor, currentColorIndex);
  };

  const getFabStyle = ({ color, background, text }) => {
    if (!color || !color.rgb) return null;
    const { rgb } = color;
    const { r, g, b } = rgb;
    return {
      backgroundColor: background && `rgb(${r}, ${g}, ${b})`,
      color: text && `rgb(${255 - r}, ${255 - g}, ${255 - b})`,
    };
  };
  return (
    <div className={wrapper}>
      {isColorPickerOpen && (
        <div className={pickerWrapper}>
          <ChromePicker
            disableAlpha
            color={currentColor.rgb}
            onChangeComplete={applyColorChange}
          />
          <div>
            {copiedColor && (
              <Fab className={classes.fab} onClick={pasteColor}>
                <SvgIcon>
                  <path d="M19 2h-4.18C14.4.84 13.3 0 12 0c-1.3 0-2.4.84-2.82 2H5c-1.1 0-2 .9-2 2v16c0 1.1.9 2 2 2h14c1.1 0 2-.9 2-2V4c0-1.1-.9-2-2-2zm-7 0c.55 0 1 .45 1 1s-.45 1-1 1-1-.45-1-1 .45-1 1-1zm7 18H5V4h2v3h10V4h2v16z" />
                </SvgIcon>
              </Fab>
            )}
            <Fab
              className={classes.fab}
              onClick={copyColor}
              style={getFabStyle({ color: copiedColor, background: true })}
            >
              <CopyIcon
                style={getFabStyle({ color: copiedColor, text: true })}
              />
            </Fab>
            <Fab className={classes.fab} onClick={clearColor}>
              <DeleteIcon />
            </Fab>
          </div>
        </div>
      )}
      {colors.map((c, i) => {
        return (
          <Fab
            key={i}
            className={classes.fab}
            onClick={() => toggleColor(i, c)}
            style={getFabStyle({ color: c, background: true })}
          >
            {c.rgb ? (
              <EditIcon
                color="action"
                style={getFabStyle({ color: c, text: true })}
              />
            ) : (
                <AddIcon />
              )}
          </Fab>
        );
      })}
      <Fab className={classes.fab} onClick={applyColors}>
        <ApplyIcon />
      </Fab>
    </div>
  );
};

export default withStyles(styles)(LedColorPicker);
