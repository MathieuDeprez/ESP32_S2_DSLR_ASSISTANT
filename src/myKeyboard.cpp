#include "../include/myKeyboard.h"

bool MyKeyboard::_isKeyboardPolling = false;
bool MyKeyboard::_isKeyboard = false;
usb_transfer_t *MyKeyboard::_KeyboardIn = NULL;
usb_transfer_t *MyKeyboard::_KeyboardOut = NULL;
bool MyKeyboard::_isKeyboardReady = false;

usb_host_client_handle_t MyKeyboard::_Client_Handle;
usb_device_handle_t MyKeyboard::_Device_Handle;

usb_host_enum_cb_t MyKeyboard::_USB_host_enumerate;

bool isBiDirectional = false;

MyKeyboard myKeyboard;

void MyKeyboard::init()
{
    Serial.println("MyKeyboard init");
    _usbh_setup(_show_config_desc_full);
}

void MyKeyboard::_keyboard_transfer_cb(usb_transfer_t *transfer)
{
    Serial.println("_keyboard_transfer_cb");

    if (_Device_Handle == transfer->device_handle)
    {
        _isKeyboardPolling = false;
        int in_xfer = transfer->bEndpointAddress & USB_B_ENDPOINT_ADDRESS_EP_DIR_MASK;
        uint8_t *const p = transfer->data_buffer;
        if (!in_xfer)
        {
            Serial.printf("transfer sent to %02x: ", transfer->bEndpointAddress);
            for (uint8_t i = 0; i < transfer->actual_num_bytes; i++)
            {
                Serial.printf("%02x ", p[i]);
            }
            Serial.println();
        }
        else if (transfer->status == 0)
        {
            // if (transfer->actual_num_bytes >= 8)
            //{

            Serial.printf("transfer received from %02x: ", transfer->bEndpointAddress);
            for (uint8_t i = 0; i < transfer->actual_num_bytes; i++)
            {
                Serial.printf("%02x ", p[i]);
            }
            Serial.println();

            esp_err_t err = usb_host_transfer_submit(transfer);
            if (err != ESP_OK)
            {
                ESP_LOGI("", "usb_host_transfer_submit In fail: %x", err);
            }

            /*static unsigned long timerP = millis();

            uint8_t *const p = transfer->data_buffer;
            if (p[0] || p[1] || p[2] || p[3] || p[4] || p[5] || p[6] || p[7])
            {
                if (millis() - timerP < 100 || millis() == timerP)
                    return;
                timerP = millis();
                // Serial.printf("HID report: %02x %02x %02x %02x %02x %02x %02x %02x\n",
                //			  p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
                if (p[2] != 0 && p[2] <= sizeof(AzertyArray) / sizeof(AzertyArray[0]))
                {
                    Serial.printf("key pressed[%d]: %s\n", p[2], AzertyArray[p[2]]);
                }
                else if (p[0] != 0)
                {
                    for (uint8_t b = 0; b < 8; b++)
                    {
                        if ((p[0] & (1 << b)) > 0)
                        {
                            Serial.printf("key pressed: %s\n", AzertyArraySpe[b]);
                            break;
                        }
                    }
                }
            }*/
            /*}
            else
            {
                Serial.printf("Keyboard boot hid transfer too short or long: %d\n", transfer->actual_num_bytes);
            }*/
        }
        else
        {
            Serial.printf("transfer->status %d\n", transfer->status);
        }
    }
    else
    {
        Serial.printf("_Device_Handle != transfer->device_handle\n");
    }
}

void MyKeyboard::_check_interface_desc_boot_keyboard(const void *p)
{
    const usb_intf_desc_t *intf = (const usb_intf_desc_t *)p;

    /*if ((intf->bInterfaceClass == USB_CLASS_HID) &&
        (intf->bInterfaceSubClass == 1) &&
        (intf->bInterfaceProtocol == 1))
    {
        _isKeyboard = true;
        Serial.printf("Claiming a boot keyboard!\n");
        esp_err_t err = usb_host_interface_claim(_Client_Handle, _Device_Handle,
                                                 intf->bInterfaceNumber, intf->bAlternateSetting);
        if (err != ESP_OK)
            Serial.printf("usb_host_interface_claim failed: %x\n", err);
    }
    else*/

    if ((intf->bInterfaceClass == USB_CLASS_STILL_IMAGE) &&
        (intf->bInterfaceSubClass == 1))
    {

        if ((intf->bInterfaceProtocol == 1) || (intf->bInterfaceProtocol == 2))
        {
            isBiDirectional = (intf->bInterfaceProtocol == 2);
            if (isBiDirectional)
            {
                if (intf->bNumEndpoints < 2)
                {
                    _isKeyboard = false;
                    return;
                }
            }
            else
            {
                if (intf->bNumEndpoints < 1)
                {
                    _isKeyboard = false;
                    return;
                }
            }
            _isKeyboard = true;
            ESP_LOGI("", "Claiming a %s-directional printer!", (isBiDirectional) ? "bi" : "uni");
            esp_err_t err = usb_host_interface_claim(_Client_Handle, _Device_Handle,
                                                     intf->bInterfaceNumber, intf->bAlternateSetting);
            if (err != ESP_OK)
                ESP_LOGI("", "usb_host_interface_claim failed: %x", err);
        }
    }
    else
    {
        Serial.printf("Not Claiming a boot keyboard!\n-bInterfaceClass: 0x%02x\n-bInterfaceSubClass: %d\n-bInterfaceProtocol: %d\n",
                      intf->bInterfaceClass, intf->bInterfaceSubClass, intf->bInterfaceProtocol);
    }
}

void MyKeyboard::_prepare_endpoint(const void *p)
{
    Serial.println("_prepare_endpoint");
    const usb_ep_desc_t *endpoint = (const usb_ep_desc_t *)p;
    esp_err_t err;

    Serial.printf("bmAttributes: 0x%02x, bEndpointAddress: 0x%02x\n", endpoint->bmAttributes, endpoint->bEndpointAddress);

    // must be interrupt for HID
    // if ((endpoint->bmAttributes & USB_BM_ATTRIBUTES_XFERTYPE_MASK) != USB_BM_ATTRIBUTES_XFER_INT)
    if ((endpoint->bmAttributes & USB_BM_ATTRIBUTES_XFERTYPE_MASK) != USB_BM_ATTRIBUTES_XFER_BULK)
    {
        Serial.printf("Not interrupt endpoint: 0x%02x\n", endpoint->bmAttributes);
        return;
    }
    if (endpoint->bEndpointAddress & USB_B_ENDPOINT_ADDRESS_EP_DIR_MASK)
    {
        Serial.println("(IN)endpoint processing");
        err = usb_host_transfer_alloc(endpoint->wMaxPacketSize, 0, &_KeyboardIn);
        if (err != ESP_OK)
        {
            _KeyboardIn = NULL;
            ESP_LOGI("", "usb_host_transfer_alloc In fail: %x", err);
            return;
        }
        _KeyboardIn->device_handle = _Device_Handle;
        _KeyboardIn->bEndpointAddress = endpoint->bEndpointAddress;
        _KeyboardIn->callback = _keyboard_transfer_cb;
        _KeyboardIn->context = NULL;
        _KeyboardIn->num_bytes = endpoint->wMaxPacketSize;

        esp_err_t err = usb_host_transfer_submit(_KeyboardIn);
        if (err != ESP_OK)
        {
            ESP_LOGI("", "usb_host_transfer_submit In fail: %x", err);
        }
        else
        {
            Serial.println("host transfet IN init");
        }

        _isKeyboardReady = true;
        Serial.printf("USB boot keyboard ready\n");
    }
    else
    {
        Serial.println("(OUT)endpoint processing");
        err = usb_host_transfer_alloc(endpoint->wMaxPacketSize * 8, 0, &_KeyboardOut);
        if (err != ESP_OK)
        {
            _KeyboardOut = NULL;
            ESP_LOGI("", "usb_host_transfer_alloc In fail: %x", err);
            return;
        }
        _KeyboardOut->device_handle = _Device_Handle;
        _KeyboardOut->bEndpointAddress = endpoint->bEndpointAddress;
        _KeyboardOut->callback = _keyboard_transfer_cb;
        _KeyboardOut->context = NULL;
        _isKeyboardReady = true;

        /*esp_err_t err = usb_host_transfer_submit(_KeyboardOut);
        if (err != ESP_OK)
        {
            ESP_LOGI("", "usb_host_transfer_submit Out fail: %x", err);
        }*/

        Serial.printf("USB boot keyboard ready\n");
    }
}

void MyKeyboard::_show_config_desc_full(const usb_config_desc_t *config_desc)
{
    // Full decode of config desc.
    const uint8_t *p = &config_desc->val[0];
    static uint8_t USB_Class = 0;
    uint8_t bLength;
    for (int i = 0; i < config_desc->wTotalLength; i += bLength, p += bLength)
    {
        bLength = *p;
        if ((i + bLength) <= config_desc->wTotalLength)
        {
            const uint8_t bDescriptorType = *(p + 1);
            switch (bDescriptorType)
            {
            case USB_B_DESCRIPTOR_TYPE_DEVICE:
                Serial.printf("USB Device Descriptor should not appear in config\n");
                break;
            case USB_B_DESCRIPTOR_TYPE_CONFIGURATION:
                Serial.println("USB_B_DESCRIPTOR_TYPE_CONFIGURATION");
                show_config_desc(p);
                break;
            case USB_B_DESCRIPTOR_TYPE_STRING:
                Serial.printf("USB string desc TBD\n");
                break;
            case USB_B_DESCRIPTOR_TYPE_INTERFACE:
                Serial.println("USB_B_DESCRIPTOR_TYPE_INTERFACE");
                USB_Class = show_interface_desc(p);
                _check_interface_desc_boot_keyboard(p);
                break;
            case USB_B_DESCRIPTOR_TYPE_ENDPOINT:
                Serial.println("USB_B_DESCRIPTOR_TYPE_ENDPOINT");
                show_endpoint_desc(p);
                // if (_isKeyboard && _KeyboardIn == NULL)
                _prepare_endpoint(p);
                break;
            case USB_B_DESCRIPTOR_TYPE_DEVICE_QUALIFIER:
                // Should not be config config?
                Serial.printf("USB device qual desc TBD\n");
                break;
            case USB_B_DESCRIPTOR_TYPE_OTHER_SPEED_CONFIGURATION:
                // Should not be config config?
                Serial.printf("USB Other Speed TBD\n");
                break;
            case USB_B_DESCRIPTOR_TYPE_INTERFACE_POWER:
                // Should not be config config?
                Serial.printf("USB Interface Power TBD\n");
                break;
            case 0x21:
                Serial.println("case 0x21");

                if (USB_Class == USB_CLASS_HID)
                {
                    Serial.println("USB_Class == USB_CLASS_HID");

                    show_hid_desc(p);
                }
                break;
            default:
                Serial.printf("Unknown USB Descriptor Type: 0x%x\n", bDescriptorType);
                break;
            }
        }
        else
        {
            Serial.printf("USB Descriptor invalid\n");
            return;
        }
    }
}

void MyKeyboard::loopUsb()
{
    // Serial.println("loopUsb()");

    _usbh_task();
    static unsigned long KeyboardTimer = millis();
    /*if (_isKeyboardReady && !_isKeyboardPolling && (millis() - KeyboardTimer > _KeyboardInterval))
    {
        Serial.println("_isKeyboardReady && !_isKeyboardPolling && (millis() - KeyboardTimer > _KeyboardInterval)");
        esp_err_t err = usb_host_transfer_submit(_KeyboardIn);
        if (err != ESP_OK)
        {
            Serial.printf("usb_host_transfer_submit In fail: %x\n", err);
        }
        _isKeyboardPolling = true;
        KeyboardTimer = 0;
    }*/
}

static uint32_t idTransaction = 0;

void MyKeyboard::checkEvent()
{
    idTransaction++;

    uint8_t bufEvent[24] = {};
    memset(_KeyboardOut->data_buffer, 0, 24);
    _KeyboardOut->num_bytes = 12;
    _KeyboardOut->data_buffer[0] = (_KeyboardOut->num_bytes & 0xff);
    _KeyboardOut->data_buffer[1] = (_KeyboardOut->num_bytes & 0xff00) >> 8;
    _KeyboardOut->data_buffer[2] = (_KeyboardOut->num_bytes & 0xff0000) >> 16;
    _KeyboardOut->data_buffer[3] = (_KeyboardOut->num_bytes & 0xff000000) >> 24;
    _KeyboardOut->data_buffer[4] = 0x01;
    _KeyboardOut->data_buffer[5] = 0x00;
    _KeyboardOut->data_buffer[6] = 0xC7;
    _KeyboardOut->data_buffer[7] = 0x90;
    _KeyboardOut->data_buffer[8] = (idTransaction & 0xff);
    _KeyboardOut->data_buffer[9] = (idTransaction & 0xff00) >> 8;
    _KeyboardOut->data_buffer[10] = (idTransaction & 0xff0000) >> 16;
    _KeyboardOut->data_buffer[11] = (idTransaction & 0xff000000) >> 24;

    esp_err_t err = usb_host_transfer_submit(_KeyboardOut);

    if (err != ESP_OK)
    {
        ESP_LOGI("", "(event) usb_host_transfer_submit In fail: %x", err);
    }
}

void MyKeyboard::openSession()
{
    idTransaction = 0;

    uint8_t bufEvent[16] = {};
    memset(_KeyboardOut->data_buffer, 0, 16);
    _KeyboardOut->num_bytes = 16;
    _KeyboardOut->data_buffer[0] = (_KeyboardOut->num_bytes & 0xff);
    _KeyboardOut->data_buffer[1] = (_KeyboardOut->num_bytes & 0xff00) >> 8;
    _KeyboardOut->data_buffer[2] = (_KeyboardOut->num_bytes & 0xff0000) >> 16;
    _KeyboardOut->data_buffer[3] = (_KeyboardOut->num_bytes & 0xff000000) >> 24;
    _KeyboardOut->data_buffer[4] = 0x01;
    _KeyboardOut->data_buffer[5] = 0x00;
    _KeyboardOut->data_buffer[6] = 0x02;
    _KeyboardOut->data_buffer[7] = 0x10;
    _KeyboardOut->data_buffer[8] = (idTransaction & 0xff);
    _KeyboardOut->data_buffer[9] = (idTransaction & 0xff00) >> 8;
    _KeyboardOut->data_buffer[10] = (idTransaction & 0xff0000) >> 16;
    _KeyboardOut->data_buffer[11] = (idTransaction & 0xff000000) >> 24;
    _KeyboardOut->data_buffer[12] = 0x01;
    _KeyboardOut->data_buffer[13] = 0x00;
    _KeyboardOut->data_buffer[14] = 0;
    _KeyboardOut->data_buffer[15] = 0;

    esp_err_t err = usb_host_transfer_submit(_KeyboardOut);

    if (err != ESP_OK)
    {
        ESP_LOGI("", "(open Session) usb_host_transfer_submit In fail: %x", err);
    }
}

void MyKeyboard::getAperture()
{
    idTransaction++;

    uint8_t bufEvent[16] = {};
    memset(_KeyboardOut->data_buffer, 0, 16);
    _KeyboardOut->num_bytes = 16;
    _KeyboardOut->data_buffer[0] = (_KeyboardOut->num_bytes & 0xff);
    _KeyboardOut->data_buffer[1] = (_KeyboardOut->num_bytes & 0xff00) >> 8;
    _KeyboardOut->data_buffer[2] = (_KeyboardOut->num_bytes & 0xff0000) >> 16;
    _KeyboardOut->data_buffer[3] = (_KeyboardOut->num_bytes & 0xff000000) >> 24;
    _KeyboardOut->data_buffer[4] = 0x01;
    _KeyboardOut->data_buffer[5] = 0x00;
    _KeyboardOut->data_buffer[6] = 0x15;
    _KeyboardOut->data_buffer[7] = 0x10;
    _KeyboardOut->data_buffer[8] = (idTransaction & 0xff);
    _KeyboardOut->data_buffer[9] = (idTransaction & 0xff00) >> 8;
    _KeyboardOut->data_buffer[10] = (idTransaction & 0xff0000) >> 16;
    _KeyboardOut->data_buffer[11] = (idTransaction & 0xff000000) >> 24;
    _KeyboardOut->data_buffer[12] = 0x0D;
    _KeyboardOut->data_buffer[13] = 0x50;
    _KeyboardOut->data_buffer[14] = 0;
    _KeyboardOut->data_buffer[15] = 0;

    esp_err_t err = usb_host_transfer_submit(_KeyboardOut);

    if (err != ESP_OK)
    {
        ESP_LOGI("", "(aperture) usb_host_transfer_submit In fail: %x", err);
    }
}

void MyKeyboard::_client_event_callback(const usb_host_client_event_msg_t *event_msg, void *arg)
{
    Serial.println("_client_event_callback");

    esp_err_t err;
    switch (event_msg->event)
    {
    /**< A new device has been enumerated and added to the USB Host Library */
    case USB_HOST_CLIENT_EVENT_NEW_DEV:
        ESP_LOGI("", "New device address: %d", event_msg->new_dev.address);
        err = usb_host_device_open(_Client_Handle, event_msg->new_dev.address, &_Device_Handle);
        if (err != ESP_OK)
            ESP_LOGI("", "usb_host_device_open: %x", err);

        usb_device_info_t dev_info;
        err = usb_host_device_info(_Device_Handle, &dev_info);
        if (err != ESP_OK)
            ESP_LOGI("", "usb_host_device_info: %x", err);
        ESP_LOGI("", "speed: %d dev_addr %d vMaxPacketSize0 %d bConfigurationValue %d",
                 dev_info.speed, dev_info.dev_addr, dev_info.bMaxPacketSize0,
                 dev_info.bConfigurationValue);

        const usb_device_desc_t *dev_desc;
        err = usb_host_get_device_descriptor(_Device_Handle, &dev_desc);
        if (err != ESP_OK)
            ESP_LOGI("", "usb_host_get_device_desc: %x", err);
        show_dev_desc(dev_desc);

        const usb_config_desc_t *config_desc;
        err = usb_host_get_active_config_descriptor(_Device_Handle, &config_desc);
        if (err != ESP_OK)
            ESP_LOGI("", "usb_host_get_config_desc: %x", err);
        (*_USB_host_enumerate)(config_desc);
        break;
    /**< A device opened by the client is now gone */
    case USB_HOST_CLIENT_EVENT_DEV_GONE:
        ESP_LOGI("", "Device Gone handle: %x", event_msg->dev_gone.dev_hdl);
        break;
    default:
        ESP_LOGI("", "Unknown value %d", event_msg->event);
        break;
    }
}

void MyKeyboard::_usbh_setup(usb_host_enum_cb_t enumeration_cb)
{
    Serial.println("_usbh_setup()");

    const usb_host_config_t config = {
        .intr_flags = ESP_INTR_FLAG_LEVEL1,
    };
    esp_err_t err = usb_host_install(&config);
    ESP_LOGI("", "usb_host_install: %x", err);

    usb_host_install(&config);
    usb_host_client_config_t client_config;
    client_config.async.client_event_callback = _client_event_callback;
    client_config.async.callback_arg = _Client_Handle;
    client_config.max_num_event_msg = 5;

    err = usb_host_client_register(&client_config, &_Client_Handle);
    ESP_LOGI("", "usb_host_client_register: %x", err);

    _USB_host_enumerate = enumeration_cb;
}

void MyKeyboard::_usbh_task(void)
{
    Serial.println("_usbh_task()");

    uint32_t event_flags;
    static bool all_clients_gone = false;
    static bool all_dev_free = false;

    esp_err_t err = usb_host_lib_handle_events(HOST_EVENT_TIMEOUT, &event_flags);
    if (err == ESP_OK)
    {
        if (event_flags & USB_HOST_LIB_EVENT_FLAGS_NO_CLIENTS)
        {
            ESP_LOGI("", "No more clients");
            all_clients_gone = true;
        }
        if (event_flags & USB_HOST_LIB_EVENT_FLAGS_ALL_FREE)
        {
            ESP_LOGI("", "No more devices");
            all_dev_free = true;
        }
    }
    else
    {
        if (err != ESP_ERR_TIMEOUT)
        {
            ESP_LOGI("", "usb_host_lib_handle_events: %x flags: %x", err, event_flags);
        }
    }

    err = usb_host_client_handle_events(_Client_Handle, CLIENT_EVENT_TIMEOUT);
    if ((err != ESP_OK) && (err != ESP_ERR_TIMEOUT))
    {
        ESP_LOGI("", "usb_host_client_handle_events: %x", err);
    }
}