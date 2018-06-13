#ifndef USB_CONNECTION_H
#define USB_CONNECTION_H

#include "libusb.h"

class USB_connection
{
    bool libusb_initialized;
    libusb_context *usbcontext;

public:
    USB_connection();
    ~USB_connection();
    int count_devices_in_update_mode();
    int count_devices_in_production_mode();
    int count_devices_vid_pid(uint32_t vid, uint32_t pid);
};

#endif // USB_CONNECTION_H
