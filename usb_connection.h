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
};

#endif // USB_CONNECTION_H
