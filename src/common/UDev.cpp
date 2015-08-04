#include "UDev.h"
#include <libudev.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <unistd.h>
#include <string.h>
#include <string>


std::string UDev::findDevice(const std::string& vendorstr, const std::string& productstr)
{
    std::string devString;
    struct udev_list_entry *devices, *dev_list_entry;
    struct udev_device *dev;
    struct udev *udev = udev_new();
    struct udev_enumerate *enumerate = udev_enumerate_new(udev);

    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);
    udev_list_entry_foreach(dev_list_entry, devices) {
        dev = udev_device_new_from_syspath(udev, udev_list_entry_get_name(dev_list_entry));

        const char* devpath = udev_device_get_devnode(dev);
        dev = udev_device_get_parent_with_subsystem_devtype(dev,"usb","usb_device");
        const char* vendor = udev_device_get_sysattr_value(dev,"idVendor");
        const char* product = udev_device_get_sysattr_value(dev,"idProduct");

        if (vendor != 0 && product != 0 && devpath != 0)
        {
            if (!strncmp(vendor,vendorstr.c_str(),vendorstr.size()) 
                && !strncmp(product,productstr.c_str(),productstr.size()))
            {
                devString = devpath;
            }
        }
        udev_device_unref(dev);
    }

    udev_enumerate_unref(enumerate);
    udev_unref(udev);
    return devString;       
}

