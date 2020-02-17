import React, { Component } from 'react';
import CssBaseline from '@material-ui/core/CssBaseline';
import DeviceList from '../DeviceList/DeviceList';

import classes from './App.module.scss';

class App extends Component {
  handleAnimationChange = event => {
    console.log(event.target.value);
  };
  render() {
    return (
      <React.Fragment>
        <CssBaseline />
        <div className="App">
          <header className={classes.appHeader}>
            Test
            <DeviceList onAnimationChange={this.handleAnimationChange} />
          </header>
        </div>
      </React.Fragment>
    );
  }
}

export default App;
