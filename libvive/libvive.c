
#include "libvive.h"

static inline unsigned short
get_tick(const unsigned char* buf) {
  return ((buf[1] << 8) & 0xFF00)
       | ((buf[3])      & 0xFF); 
}

static void*
read_controller (void *data) {
  struct vive_controller *c = (struct vive_controller *) data;
	int res;
  unsigned char *buf = c->misc.hid_buffer;
  hid_device *handle = c->misc.hid;

  while (c->misc.state != VIVE_CONTROLLER_CLOSING) {
    res = hid_read(handle, buf, 128);
    if (!res) continue;
    unsigned char buttons = buf[5];
    unsigned short tick = get_tick(buf);

    switch (buf[4]) {
    case VIVE_PACKET_TRIGGER:
      c->ticks.trigger = tick;
      c->trigger = buttons;
      break;
    case VIVE_PACKET_TRIGGER_CLICK:
      c->ticks.trigger_click = tick;
      c->trigger_click = buttons;
      break;
    case VIVE_PACKET_ANALOG:
      c->ticks.analog = tick;
      switch (buttons) {
      case 6:  c->trackpad_center_click = buttons; break;
      case 4:  c->trackpad_edge_click   = buttons; break;
      case 16: c->side_button           = buttons; break;
      case 32: c->hamburger             = buttons; break;
      }
      break;
    case VIVE_PACKET_TRACKPAD:
      c->ticks.trackpad = tick;
      c->trackpad_x = buf[6];
      c->trackpad_y = buf[8];
      break;
    case VIVE_PACKET_TRACKPAD_TAP:
      c->ticks.trackpad_tap = tick;
      c->trackpad_tap = buttons;
      break;
    case VIVE_PACKET_SIXAXIS:
      c->ticks.sixaxis = tick;
      c->pitch = buf[9];
      c->roll  = buf[7];

      // TODO: confirm these
      c->roll_accel  = buf[17];
      c->pitch_accel = buf[13];
      break;
    }
  }

  return NULL;
}

void
vive_init(struct vive_state *vive) {
  int i;
  vive->errno = VIVE_SUCCESS;
  for (i = 0; i < VIVE_MAX_CONTROLLERS; ++i) {
    struct vive_controller *controller = &vive->controllers[i];

    controller->ticks.analog = 0;
    controller->ticks.sixaxis = 0;
    controller->ticks.trackpad = 0;
    controller->ticks.trigger = 0;
    controller->ticks.trigger_click = 0;

    controller->misc.hid = 0;
    controller->pitch = 0;
    controller->roll = 0;

    // TODO: init rest
  }
}

int
vive_open(struct vive_state *vive) {
  int i;
  int err = 0;

  vive_init(vive);

  if ((err = vive_get_controllers(vive))) {
    return err;
  }

  for (i = 0; i < vive->num_controllers; ++i) {
    struct vive_controller *controller = &vive->controllers[i];
    if (pthread_create(&controller->misc.thread, NULL, read_controller,
                       controller)) {
      return VIVE_ERR_THREAD_CREATE_FAIL;
    }
  }

  return VIVE_SUCCESS;
}


int
vive_get_controllers (struct vive_state *vive) {
	struct hid_device_info *devs;
	struct vive_controller *controller;
  int errno = VIVE_SUCCESS;
  int i;
  int count = 0;

	devs = hid_enumerate(VIVE_CONTROLLER_VID, VIVE_CONTROLLER_PID);
  for(i = 0; devs != NULL; ++i) {
    controller = &vive->controllers[i];
    controller->misc.hid = hid_open(VIVE_CONTROLLER_VID,
                               VIVE_CONTROLLER_PID,
                               devs->serial_number);
    if (!controller->misc.hid) {
      return VIVE_ERR_HID_OPEN_FAIL;
    }
    else count++;

    hid_set_nonblocking(controller->misc.hid, 0);

    // NOTE: do I need to do a hid_write here?
    controller->misc.hid_buffer[0] = VIVE_REPORT;
    controller->misc.hid_buffer[1] = 0x81;
    hid_write(controller->misc.hid, controller->misc.hid_buffer, 1);

    devs = devs->next;
  }
	hid_free_enumeration(devs);

  vive->num_controllers = count;
  return errno;
}
