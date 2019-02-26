import React, { Component } from 'react';
import DeviceList from '../DeviceList/DeviceList';

import classes from './App.module.scss';

class App extends Component {
  handleAnimationChange = event => {
    console.log(event.target.value);
  };
  render() {
    return (
      <div className="App">
        <header className={classes.appHeader}>
          <DeviceList onAnimationChange={this.handleAnimationChange} />
        </header>
      </div>
    );
  }
}

export default App;
