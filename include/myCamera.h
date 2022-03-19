#include <Arduino.h>
#include "main.h"
#ifndef MYCAMERA_CLASS
#define MYCAMERA_CLASS

typedef void (*usb_host_enum_cb_t)(const usb_config_desc_t *config_desc);

class MyCamera
{
    typedef struct
    {
        uint32_t total;
        uint32_t len;
        uint32_t offset;
        uint8_t data[64];
    } PtpTransfer;

public:
    static const char *const prNames[];
    void init();
    void loopUsb();
    void checkEvent();
    void getAperture();
    uint32_t getDevProp(DevProp::DevPropDesc propDesc);
    void openSession();
    void getImage();
    void getPreview();
    void getPreviewHq();
    void GetObjectHandles();
    bool getLiveViewStatus();
    void setLiveViewStatus(bool status);
    void getDevProps();
    void flushQueue();
    void Operation(uint16_t opCode, uint8_t nbrParams, uint32_t *params);
    // void PrintDevProp(uint8_t **pp, uint16_t *pcntdn);
    uint16_t getResponseCode(uint32_t timeout, uint8_t *&data, uint32_t &total, bool toData = true);
    static bool isCameraReady;

private:
    static bool _isCamera;
    static bool _isCameraPolling;
    static usb_transfer_t *_CameraIn;
    static usb_transfer_t *_CameraOut;
    static usb_transfer_t *_CameraInterrupt;

    static void _camera_transfer_cb(usb_transfer_t *transfer);
    static void _check_interface_desc_camera(const void *p);
    static void _prepare_endpoint(const void *p);
    static void _show_config_desc_full(const usb_config_desc_t *config_desc);
    static usb_host_client_handle_t _Client_Handle;
    static usb_device_handle_t _Device_Handle;

    static usb_host_enum_cb_t _USB_host_enumerate;
    static void _client_event_callback(const usb_host_client_event_msg_t *event_msg, void *arg);
    static void _usbh_setup(usb_host_enum_cb_t enumeration_cb);
    void _usbh_task(void);
};

#endif