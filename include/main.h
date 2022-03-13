#ifndef main_h
#define main_h
#include <Arduino.h>
#include "usb/usb_host.h"
#include "ptpconst.h"
#include "nikon.h"
#include "devprop.h"
#include "myCamera.h"
#include "WiFi.h"
#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"

void show_dev_desc(const usb_device_desc_t *dev_desc);
void show_config_desc(const void *p);
uint8_t show_interface_desc(const void *p);
void show_endpoint_desc(const void *p);
void show_hid_desc(const void *p);
void show_interface_assoc(const void *p);

const TickType_t HOST_EVENT_TIMEOUT = 1;
const TickType_t CLIENT_EVENT_TIMEOUT = 1;

extern char AzertyArraySpe[8][8];
extern char AzertyArray[102][8];
extern AsyncWebSocket ws;

extern MyCamera myCamera;
extern DevProp devProp;
#endif
