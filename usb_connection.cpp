#include "usb_connection.h"

//03eb:2ff1 Atmel Corp.
const uint32_t vendor = 0x03eb;
const uint32_t product = 0x2ff1;

USB_connection::USB_connection(){
    if (libusb_init(&usbcontext)) {
        this->libusb_initialized = false;
        return; // DEVICE_ACCESS_ERROR;
      }
    this->libusb_initialized = true;
}

USB_connection::~USB_connection(){
//    libusb_exit(usbcontext);
}

int USB_connection::count_devices_in_update_mode(){
        libusb_device **list;
        size_t i,devicecount;
        int devices_in_update_mode=0;

        if (!this->libusb_initialized) return -2;

        devicecount = libusb_get_device_list( usbcontext, &list );

        for( i = 0; i < devicecount; i++ ) {
            libusb_device *device = list[i];
            struct libusb_device_descriptor descriptor;

            if( libusb_get_device_descriptor(device, &descriptor) ) {
                 break;
            }

            if( (vendor  == descriptor.idVendor) &&
                (product == descriptor.idProduct))
            {
                devices_in_update_mode++;
            }
        }

        return devices_in_update_mode;
}
