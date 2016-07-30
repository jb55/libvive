
#include "libvive.h"
#include <glisy/vec2.h>
#include <stdio.h>

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

  const float alpha = 0.02f;

  // smoothing
  vec2 gyro = vec2_create();
  vec2 sp = vec2_create();
  vec2 sp2 = vec2_create();
  vec2 sp_last = vec2_create();
  vec2 sp2_last = vec2_create();

  while (c->misc.state != VIVE_CONTROLLER_CLOSING) {
    float roll_last  = c->roll;
    float pitch_last = c->pitch;
    sp_last  = sp;
    sp2_last = sp2;

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

      gyro = vec2(c->roll, c->pitch);

      // double exponential smoothing
      sp  = vec2_add(vec2_scale(gyro, alpha),
                     vec2_scale(sp_last, (1.0f - alpha)));

      sp2 = vec2_add(vec2_scale(sp, alpha),
                     vec2_scale(sp2_last, (1.0f - alpha)));

      vec2 b1 = vec2_scale(vec2_sub(sp, sp2), (alpha / (1.0f - alpha)));
      vec2 b0 = vec2_sub(vec2_scale(sp, 2.0f), sp2);

      c->roll_smooth  = b0.x;
      c->pitch_smooth = b0.y;

      // TODO: confirm these
      c->roll_accel  = buf[17];
      c->pitch_accel = buf[13];
      break;
    }
  }

  c->misc.state = VIVE_CONTROLLER_CLOSED;
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
    controller->misc.state = VIVE_CONTROLLER_INIT;
    controller->misc.options = 0;

    controller->pitch = 0;
    controller->roll = 0;
    controller->pitch_accel = 0;
    controller->roll_accel = 0;
    controller->trackpad_x = 0;
    controller->trackpad_y = 0;
    controller->trackpad_edge_click = 0;
    controller->trackpad_center_click = 0;
    controller->trackpad_tap = 0;
    controller->hamburger = 0;
    controller->side_button = 0;

    // TODO: init rest
  }
}

int vive_close(struct vive_state *vive) {
  int i = 0;
  for (i = 0; i < vive->num_controllers; ++i) {
    void *data;
    struct vive_controller *c = &vive->controllers[i];
    c->misc.state = VIVE_CONTROLLER_CLOSING;
    pthread_join(c->misc.thread, &data);
    c->misc.state = VIVE_CONTROLLER_CLOSED;
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
    controller->misc.state = VIVE_CONTROLLER_READING;

    if (pthread_create(&controller->misc.thread, NULL, read_controller,
                       controller)) {
      controller->misc.state = VIVE_CONTROLLER_CLOSED;
      return VIVE_ERR_THREAD_CREATE_FAIL;
    }
    printf("starting read thread %d\n", i);
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

    hid_device *handle = controller->misc.hid =
      hid_open(VIVE_CONTROLLER_VID,
               VIVE_CONTROLLER_PID,
               devs->serial_number);

    unsigned char *buf = controller->misc.hid_buffer;

    if (!controller->misc.hid) {
      return VIVE_ERR_HID_OPEN_FAIL;
    }
    else count++;

    hid_set_nonblocking(controller->misc.hid, 0);

    // Send a Feature Report to the device
    buf[0] = VIVE_REPORT; // First byte is report number
    buf[1] = 0xff;
    buf[2] = 0xff;
    buf[3] = 0xff;
    hid_send_feature_report(handle, buf, 17);

    // NOTE: do I need to do a hid_write here?
    /* buf[0] = VIVE_REPORT; */
    /* buf[1] = 0x81; */
    /* hid_write(controller->misc.hid, buf, 1); */

    devs = devs->next;
  }
	hid_free_enumeration(devs);

  vive->num_controllers = count;
  return errno;
}
