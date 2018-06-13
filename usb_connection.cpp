#include "usb_connection.h"

//03eb:2ff1 Atmel Corp.
const uint32_t vendor = 0x03eb;
const uint32_t product = 0x2ff1;

//20a0:4109 Clay Logic
const uint32_t vendor_prod = 0x20a0;
const uint32_t product_prod = 0x4109;

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


int USB_connection::count_devices_vid_pid(uint32_t vid, uint32_t pid){
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

        if( (vid  == descriptor.idVendor) &&
            (pid == descriptor.idProduct))
        {
            devices_in_update_mode++;
        }
    }

    return devices_in_update_mode;
}

int USB_connection::count_devices_in_production_mode(){
    return count_devices_vid_pid(vendor_prod, product_prod);
}


int USB_connection::count_devices_in_update_mode(){
    return count_devices_vid_pid(vendor, product);
}
