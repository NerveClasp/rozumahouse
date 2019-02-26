import React, { Component } from 'react';

export default class Device extends Component {
  handleAnimationChange = e => {
    const { ip, mac } = this.props.device;
    const { value } = e.target;
    console.log(value, ip, mac);
  };
  render() {
    const { device } = this.props;
    const { model, ip, mac, name, animation } = device;
    return (
      <tr>
        <td>{model}</td>
        <td>{ip}</td>
        <td>{mac}</td>
        <td>{name || 'no name'}</td>
        <td>
          <select name="animation" onChange={this.handleAnimationChange}>
            {animation.map((anim, i) => (
              <option key={i} value={anim}>
                {anim}
              </option>
            ))}
          </select>
        </td>
      </tr>
    );
  }
}

/*
devices {
      model
      ip
      mac
      name
      action
      command
      animation
      left
      right
    }
*/
