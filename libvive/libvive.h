
#include <hidapi/hidapi.h>
#include <pthread.h>

#define VIVE_REPORT 1
#define VIVE_CONTROLLER_VID 0x28de
#define VIVE_CONTROLLER_PID 0x2101
#define VIVE_MAX_CONTROLLERS 8

enum vive_errors {
  VIVE_SUCCESS,
  VIVE_ERR_HID_OPEN_FAIL,
  VIVE_ERR_THREAD_CREATE_FAIL,
};

enum vive_controller_state {
  VIVE_CONTROLLER_INIT,
  VIVE_CONTROLLER_READING,
  VIVE_CONTROLLER_CLOSING,
  VIVE_CONTROLLER_CLOSED,
};

enum vive_controller_options {
  VIVE_CONTROLLER_SIXAXIS_SMOOTHING = 1 << 0,
};

enum vive_packet_type {
  VIVE_PACKET_SIXAXIS       = 232,
  VIVE_PACKET_ANALOG        = 241,
  VIVE_PACKET_TRACKPAD      = 242,
  VIVE_PACKET_TRACKPAD_TAP  = 243,
  VIVE_PACKET_TRIGGER       = 244,
  VIVE_PACKET_TRIGGER_CLICK = 245,
};

struct vive_ticks {
  unsigned short analog;
  unsigned short sixaxis;
  unsigned short trackpad;
  unsigned short trackpad_tap;
  unsigned short trigger;
  unsigned short trigger_click;
};

struct vive_controller_misc {
  pthread_t thread;
  int state;
  int options;
  unsigned char hid_buffer[255];
  hid_device *hid;
};

struct vive_controller {
  struct vive_controller_misc misc;
  struct vive_ticks ticks;

  unsigned char trigger;
  char trigger_click;
  char pitch, roll, pitch_accel, roll_accel;
  float pitch_smooth, roll_smooth;
  char trackpad_x;
  char trackpad_y;
  char trackpad_edge_click;
  char trackpad_center_click;
  char trackpad_tap;
  char hamburger;
  char side_button;
};

struct vive_state {
  int errno;
  struct vive_controller controllers[VIVE_MAX_CONTROLLERS];
  int num_controllers;
};

int vive_open(struct vive_state *vive);
int vive_close(struct vive_state *vive);
int vive_get_controllers(struct vive_state *vive);
