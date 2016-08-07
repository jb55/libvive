#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <hidapi/hidapi.h>

typedef unsigned char u8;
void print_report(u8 *buf, int len);

enum packet_type {
  PACKET_SIXAXIS         = 232,
  PACKET_ANALOG          = 241,
  PACKET_TRACKPAD        = 242,
  PACKET_TRACKPAD_TAP    = 243,
  PACKET_TRIGGER         = 244,
  PACKET_TRIGGER_BUTTON  = 245,
};

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)                    \
  (byte & 0x80 ? '1' : '0'),                    \
    (byte & 0x40 ? '1' : '0'),                  \
    (byte & 0x20 ? '1' : '0'),                  \
    (byte & 0x10 ? '1' : '0'),                  \
    (byte & 0x08 ? '1' : '0'),                  \
    (byte & 0x04 ? '1' : '0'),                  \
    (byte & 0x02 ? '1' : '0'),                  \
    (byte & 0x01 ? '1' : '0')


static inline unsigned short
get_tick(const unsigned char* buf) {
  return ((buf[1] << 8) & 0xFF00)
       | ((buf[3])      & 0xFF);
}

void print_binary(u8 byte) {
  fprintf(stderr, BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(byte));
}

void print_sixaxis(u8 *buf, int len) {
  fprintf(stderr, "gyro %04hhd %04hhd\n", buf[9], buf[7]);
}

void print_buttons(u8 *buf) {
  u8 typ = buf[4];
  u8 buttons = buf[5];

  /* print_binary(buttons); */
  /* fprintf(stderr, "\n"); */
  /* if (buttons & 0x40) */
  /*   fprintf(stderr, "something\n"); */
  if (buttons == 6)
    fprintf(stderr, "trackpad_center\n", buttons);
  if (buttons == 4)
    fprintf(stderr, "trackpad_edge\n", buttons);
  if (buttons == 32)
    fprintf(stderr, "hamburger\n");
  if (buttons == 16)
    fprintf(stderr, "side_button\n");
}

void print_trackpad(u8 *buf, int len) {
  char x = buf[6];
  char y = buf[8];
  fprintf(stderr, "pad %4hhd %4hhd\n", x, y);
}

int main(int argc, char* argv[])
{
	int res;
	u8 report;
	unsigned char buf[128] = {0};
	#define MAX_STR 255
	wchar_t wstr[MAX_STR];
	hid_device *handle;
	int i;

	// Read a Feature Report from the device
  if (argc > 1)
    sscanf(argv[1], "%hhu", &report);
  else
    report = 1;

	// Enumerate and print the HID devices on the system
	struct hid_device_info *devs, *cur_dev;

	devs = hid_enumerate(0x0, 0x0);
	cur_dev = devs;
	while (1) {
		fprintf(stderr, "Device Found\n  type: %04hx %04hx\n  path: %s\n  serial_number: %ls",
			cur_dev->vendor_id, cur_dev->product_id, cur_dev->path, cur_dev->serial_number);
		fprintf(stderr, "\n");
		fprintf(stderr, "  Manufacturer: %ls\n", cur_dev->manufacturer_string);
		fprintf(stderr, "  Product:      %ls\n", cur_dev->product_string);
		fprintf(stderr, "\n");
    if (cur_dev->next) {
      cur_dev = cur_dev->next;
    }
    else {
      break;
    }
	}
	hid_free_enumeration(devs);


	// Open the device using the VID, PID,
	// and optionally the Serial number.
	handle = hid_open(0x28de, 0x2101, NULL);
	/* handle = hid_open(0x0bb4, 0x2c87, NULL); // vive */

  if (!handle) {
    fprintf(stderr, "Couldn't get hid handle %04hx %04hx\n",
              cur_dev->vendor_id, cur_dev->product_id);
    exit(1);
  }

	// Read the Manufacturer String
	res = hid_get_manufacturer_string(handle, wstr, MAX_STR);
	fprintf(stderr, "Manufacturer String: %ls\n", wstr);

	// Read the Product String
	res = hid_get_product_string(handle, wstr, MAX_STR);
	fprintf(stderr, "Product String: %ls\n", wstr);

	// Read the Serial Number String
	res = hid_get_serial_number_string(handle, wstr, MAX_STR);
	fprintf(stderr, "Serial Number String: %ls", wstr);
	fprintf(stderr, "\n");

	// Send a Feature Report to the device
	buf[0] = report; // First byte is report number
	buf[1] = 0xff;
  buf[2] = 0xff;
  buf[3] = 0xff;
  res = hid_send_feature_report(handle, buf, 17);

	// Read a Feature Report from the device
  buf[0] = report;
  res = hid_get_feature_report(handle, buf, 64);
  fprintf(stderr, "Feature Report %hhu (%d)\n", report, res);

  // Print out the returned buffer.
  for (i = 0; i < res; i++)
    fprintf(stderr, "%02hhx ", buf[i]);
  fprintf(stderr, "\n");

  /* buf[1] = 0x81; */
	/* hid_write(handle, buf, 1); */

	hid_set_nonblocking(handle, 0);
	// Send an Output report to toggle the LED (cmd 0x80)
	/* buf[0] = 1; // First byte is report number */
	/* buf[1] = 0x80; */
	/* res = hid_write(handle, buf, 65); */

	// Send an Output report to request the state (cmd 0x81)
  buf[0] = report;
  buf[1] = 0x81;
  hid_write(handle, buf, 1);

	// Read requested state
  /* if (res < 0 || res < 78) */

  while (1) {
    res = hid_read(handle, buf, 128);
    if (!res) continue;
    u8 buttons = buf[5];
    switch (buf[4]) {
    case PACKET_TRIGGER:
      if (buttons >= 5)
        fprintf(stderr, "trigger %hhu\n", buttons);
      break;
    case PACKET_TRIGGER_BUTTON:
      if (buttons == 1)
        fprintf(stderr, "trigger_click\n");
      break;
    case PACKET_ANALOG:
      print_buttons(buf);
      break;
    case PACKET_TRACKPAD:
      print_trackpad(buf, res);
      break;
    case PACKET_TRACKPAD_TAP:
      if (buttons == 2)
        fprintf(stderr, "trackpad_tap\n");
      break;
    case PACKET_SIXAXIS:
      /* print_sixaxis(buf, res); */
      break;
    }
    for (i = 0; i < res; i++) {
      printf("%03hhu ", buf[i]);
    }
    printf("%hu", get_tick(buf));
    printf("\n", get_tick(buf));
      /* printf("%02hhx ", buf[i]); */
      /* printf("%03hhu ", buf[i]); */
  }

	return 0;
}


