
# libvive

[![vivegui](https://jb55.com/s/9f77565aabe6ef04.gif "ViveGUI")](https://jb55.com/s/vivegui_smoothing.mp4)

  libvive is a library for reading USB data from:

  * VIVE controllers
    - [x] Buttons
    - [x] Gyros
    - [ ] Lasers

  * VIVE headset
    - [ ] Lasers
    - [ ] Gyros?
    - [ ] Camera

  * Lighthouse
    - [ ] ???

## Installation

  Dependencies libvive: hidapi
  Dependencies vivegui: glfw3, GLU, GLEW

    $ make install

## Example

```c
int main(int argc, char** argv) {
  struct vive_state vive;
  struct vive_controller *controller;
  int i;

  if (vive_open(&vive)) {
    printf("Could not read from vive controllers\n");
    return 1;
  }

  for (i = 0; i < vive->num_controllers; ++i) {
    controller = &vive->controllers[i];

    printf("smoothed gyro pitch/roll: %f %f\n", 
           controller->pitch_smooth,
           controller->roll_smooth);
  }

  vive_close(&vive);
```
