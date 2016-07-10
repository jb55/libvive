#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <hidapi/hidapi.h>

typedef unsigned char u8;

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
		printf("Device Found\n  type: %04hx %04hx\n  path: %s\n  serial_number: %ls",
			cur_dev->vendor_id, cur_dev->product_id, cur_dev->path, cur_dev->serial_number);
		printf("\n");
		printf("  Manufacturer: %ls\n", cur_dev->manufacturer_string);
		printf("  Product:      %ls\n", cur_dev->product_string);
		printf("\n");
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
	handle = hid_open(0x28de, 0x2000, L"LHR-1B4AB126");

  if (!handle) {
    printf("Couldn't get hid handle %04hx %04hx\n",
              cur_dev->vendor_id, cur_dev->product_id);
    exit(1);
  }

	// Read the Manufacturer String
	res = hid_get_manufacturer_string(handle, wstr, MAX_STR);
	printf("Manufacturer String: %ls\n", wstr);

	// Read the Product String
	res = hid_get_product_string(handle, wstr, MAX_STR);
	printf("Product String: %ls\n", wstr);

	// Read the Serial Number String
	res = hid_get_serial_number_string(handle, wstr, MAX_STR);
	printf("Serial Number String: %ls", wstr);
	printf("\n");

	// Send a Feature Report to the device
	/* buf[0] = 0x1; // First byte is report number */
	/* buf[1] = 0xa0; */
	/* buf[2] = 0x0a; */
	/* res = hid_send_feature_report(handle, buf, 17); */

	// Set the hid_read() function to be non-blocking.

	// Read a Feature Report from the device
  printf("Feature Report %hhu\n", report);
  res = hid_get_feature_report(handle, &report, 64);
  printf("Len: %d\n", res);

  // Print out the returned buffer.
  for (i = 0; i < res; i++)
    printf("%02hhx ", buf[i]);
  printf("\n");


	hid_set_nonblocking(handle, 1);
	// Send an Output report to toggle the LED (cmd 0x80)
	/* buf[0] = 1; // First byte is report number */
	/* buf[1] = 0x80; */
	/* res = hid_write(handle, buf, 65); */

	// Send an Output report to request the state (cmd 0x81)
	/* buf[1] = 0x; */
	/* hid_write(handle, buf, 65); */

	// Read requested state
  res = hid_read(handle, buf, 64);
  /* if (res < 0 || res < 78) */
  printf("res: %d\n", res);

  /* print_report(&buf[3], res - 3); */
  for (i = 0; i < res; i++)
    printf("%02hhx ", buf[i]);
  printf("\n");


	return 0;
}


