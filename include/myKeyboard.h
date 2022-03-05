#include <Arduino.h>
#include <main.h>
#ifndef ENCODER_KEYBOARD
#define ENCODER_KEYBOARD

typedef void (*usb_host_enum_cb_t)(const usb_config_desc_t *config_desc);

class MyKeyboard
{
public:
    void init();
    void loopUsb();
    void checkEvent();
    void getAperture();
    void openSession();
    String keyboardCard = "";
    String keyboardCredentials = "";

private:
    static bool _isKeyboard;
    static bool _isKeyboardReady;
    static bool _isKeyboardPolling;
    static const size_t _KEYBOARD_IN_BUFFER_SIZE;
    static usb_transfer_t *_KeyboardIn;
    static usb_transfer_t *_KeyboardOut;

    static void _keyboard_transfer_cb(usb_transfer_t *transfer);
    static void _check_interface_desc_boot_keyboard(const void *p);
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