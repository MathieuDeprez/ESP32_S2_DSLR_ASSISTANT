#include "../include/myCamera.h"

bool MyCamera::_isCameraPolling = false;
bool MyCamera::_isCamera = false;
usb_transfer_t *MyCamera::_CameraIn = NULL;
usb_transfer_t *MyCamera::_CameraOut = NULL;
usb_transfer_t *MyCamera::_CameraInterrupt = NULL;
uint8_t CameraInterval;
bool MyCamera::isCameraReady = false;

QueueHandle_t xQueue1;

usb_host_client_handle_t MyCamera::_Client_Handle;
usb_device_handle_t MyCamera::_Device_Handle;

usb_host_enum_cb_t MyCamera::_USB_host_enumerate;

#define PTP_USB_CONTAINER_UNDEFINED 0x0000
#define PTP_USB_CONTAINER_COMMAND 0x0001
#define PTP_USB_CONTAINER_DATA 0x0002
#define PTP_USB_CONTAINER_RESPONSE 0x0003
#define PTP_USB_CONTAINER_EVENT 0x0004

bool isBiDirectional = false;

MyCamera myCamera;

void MyCamera::init()
{
    Serial.println("MyCamera init");
    xQueue1 = xQueueCreate(10, sizeof(PtpTransfer));
    _usbh_setup(_show_config_desc_full);
}

void MyCamera::_camera_transfer_cb(usb_transfer_t *transfer)
{
    // Serial.println("_camera_transfer_cb");

    static uint32_t dataOffset = 0;
    static uint32_t total = 0;

    if (_Device_Handle == transfer->device_handle)
    {
        _isCameraPolling = false;
        int in_xfer = transfer->bEndpointAddress & USB_B_ENDPOINT_ADDRESS_EP_DIR_MASK;
        uint8_t *const p = transfer->data_buffer;

        if (transfer->bEndpointAddress == _CameraInterrupt->bEndpointAddress)
        {
            /*Serial.printf("callback interrupt. in_xfer: %d, number: %d\n", in_xfer, transfer->actual_num_bytes);
            for (uint8_t i = 0; i < transfer->actual_num_bytes; i++)
            {
                Serial.printf("%02x ", p[i]);
            }
            Serial.println();*/
        }
        else if (transfer->bEndpointAddress == _CameraOut->bEndpointAddress)
        {
            /*Serial.printf("transfer sent to %02x: ", transfer->bEndpointAddress);
            for (uint8_t i = 0; i < transfer->actual_num_bytes; i++)
            {
                Serial.printf("%02x ", p[i]);
            }
            Serial.println();*/
        }
        else if (transfer->bEndpointAddress == _CameraIn->bEndpointAddress && transfer->status == 0)
        {
            // if (transfer->actual_num_bytes >= 8)
            //{

            /*Serial.printf("transfer received from %02x: ", transfer->bEndpointAddress);
            for (uint8_t i = 0; i < transfer->actual_num_bytes; i++)
            {
                Serial.printf("%02x ", p[i]);
            }
            Serial.println();*/

            uint16_t container_type = p[4] | (p[5] << 8);

            if (container_type == PTP_USB_CONTAINER_DATA || total != 0)
            {

                // Serial.println("PTP_USB_CONTAINER_DATA");
                {
                    uint16_t operation_code = p[6] | (p[7] << 8);
                    if (operation_code == 0x1015)
                    {
                        uint32_t value = p[12] | (p[13] << 8) | (p[14] << 16) | (p[15] << 24);
                        // Serial.printf("PTP_OC_GetDevicePropValue with value: %08x\n", value);
                    }
                    /*else if (operation_code == 0x90c7)
                    {
                        uint8_t number_of_events = p[12] | (p[13] << 8);
                        for (uint8_t i = 0; i < number_of_events; i++)
                        {
                            uint16_t event_code = p[14 + i * 6] | (p[15 + i * 6] << 8);
                            uint16_t property_code = p[16 + i * 6] | (p[17 + i * 6] << 8);
                            Serial.printf("event(%d): code: %04x, property: %04x\n", i + 1, event_code, property_code);
                        }
                    }*/
                    if (transfer->actual_num_bytes > 12 || total != 0)
                    {
                        PtpTransfer myTransfer;
                        memcpy(myTransfer.data, p, transfer->actual_num_bytes);
                        myTransfer.len = transfer->actual_num_bytes;
                        if (dataOffset == 0)
                            total = p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
                        myTransfer.total = total;
                        myTransfer.offset = dataOffset;
                        BaseType_t status = xQueueSend(xQueue1, &myTransfer, 0);
                        if (status != pdPASS)
                        {
                            Serial.println("failed to send to Queue1");
                        }
                        /*else
                        {
                            Serial.println("send queue data");
                        }*/
                        dataOffset += transfer->actual_num_bytes;
                        if (dataOffset == total)
                        {
                            total = 0;
                        }
                    }
                }
            }
            else if (container_type == PTP_USB_CONTAINER_COMMAND)
            {

                Serial.println("PTP_USB_CONTAINER_COMMAND");
                for (uint8_t i = 0; i < transfer->actual_num_bytes; i++)
                {
                    Serial.printf("%02x ", p[i]);
                }
                Serial.println();
            }
            else if (container_type == PTP_USB_CONTAINER_RESPONSE)
            {
                Serial.println("PTP_USB_CONTAINER_RESPONSE");
                {
                    total = 0;
                    dataOffset = 0;
                    PtpTransfer myTransfer;
                    memcpy(myTransfer.data, p, transfer->actual_num_bytes);
                    myTransfer.len = transfer->actual_num_bytes;
                    BaseType_t status = xQueueSend(xQueue1, &myTransfer, 0);
                    if (status != pdPASS)
                    {
                        Serial.println("failed to send to Queue1");
                    }
                    /*else
                    {
                        Serial.println("send queue response");
                    }*/
                    /*if (response_code == 0x2001)
                    {
                        Serial.println("Response code OK");
                    }
                    else
                    {
                        Serial.printf("Response code error: %04x\n", response_code);
                    }*/
                }
            }
            else if (container_type == PTP_USB_CONTAINER_EVENT)
            {
                Serial.println("PTP_USB_CONTAINER_EVENT");
            }
            else
            {
                Serial.printf("PTP_USB_CONTAINER_UNDEFINED: %04x\n", container_type);
            }

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

void MyCamera::_check_interface_desc_camera(const void *p)
{
    const usb_intf_desc_t *intf = (const usb_intf_desc_t *)p;

    /*if ((intf->bInterfaceClass == USB_CLASS_HID) &&
        (intf->bInterfaceSubClass == 1) &&
        (intf->bInterfaceProtocol == 1))
    {
        _isCamera = true;
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
                    _isCamera = false;
                    return;
                }
            }
            else
            {
                if (intf->bNumEndpoints < 1)
                {
                    _isCamera = false;
                    return;
                }
            }
            _isCamera = true;
            ESP_LOGI("", "Claiming a %s-directional camera!", (isBiDirectional) ? "bi" : "uni");
            esp_err_t err = usb_host_interface_claim(_Client_Handle, _Device_Handle,
                                                     intf->bInterfaceNumber, intf->bAlternateSetting);
            if (err != ESP_OK)
                ESP_LOGI("", "usb_host_interface_claim failed: %x", err);
        }
    }
    else
    {
        Serial.printf("Not Claiming a boot camera!\n-bInterfaceClass: 0x%02x\n-bInterfaceSubClass: %d\n-bInterfaceProtocol: %d\n",
                      intf->bInterfaceClass, intf->bInterfaceSubClass, intf->bInterfaceProtocol);
    }
}

void MyCamera::_prepare_endpoint(const void *p)
{
    Serial.println("_prepare_endpoint");
    const usb_ep_desc_t *endpoint = (const usb_ep_desc_t *)p;
    esp_err_t err;

    Serial.printf("bmAttributes: 0x%02x, bEndpointAddress: 0x%02x\n", endpoint->bmAttributes, endpoint->bEndpointAddress);

    // must be interrupt for HID
    // if ((endpoint->bmAttributes & USB_BM_ATTRIBUTES_XFERTYPE_MASK) != USB_BM_ATTRIBUTES_XFER_INT)
    if ((endpoint->bmAttributes & USB_BM_ATTRIBUTES_XFERTYPE_MASK) != USB_BM_ATTRIBUTES_XFER_BULK)
    {
        if ((endpoint->bmAttributes & USB_BM_ATTRIBUTES_XFERTYPE_MASK) == USB_BM_ATTRIBUTES_XFER_INT)
        {
            Serial.printf("Interrupt endpoint: 0x%02x\n", endpoint->bmAttributes);
            if (endpoint->bEndpointAddress & USB_B_ENDPOINT_ADDRESS_EP_DIR_MASK)
            {
                err = usb_host_transfer_alloc(endpoint->wMaxPacketSize * 8, 0, &_CameraInterrupt);
                if (err != ESP_OK)
                {
                    _CameraInterrupt = NULL;
                    ESP_LOGI("", "usb_host_transfer_alloc In fail: %x", err);
                    return;
                }
                _CameraInterrupt->device_handle = _Device_Handle;
                _CameraInterrupt->bEndpointAddress = endpoint->bEndpointAddress;
                _CameraInterrupt->callback = _camera_transfer_cb;
                _CameraInterrupt->context = NULL;
                isCameraReady = true;
                CameraInterval = endpoint->bInterval;
                Serial.printf("Interrupt interval: %d\n", CameraInterval);
                ESP_LOGI("", "USB boot Camera ready");
            }
            else
            {
                ESP_LOGI("", "Ignoring interrupt Out endpoint");
            }
        }

        Serial.printf("Not interrupt endpoint: 0x%02x\n", endpoint->bmAttributes);
        return;
    }
    if (endpoint->bEndpointAddress & USB_B_ENDPOINT_ADDRESS_EP_DIR_MASK)
    {
        Serial.println("(IN)endpoint processing");
        err = usb_host_transfer_alloc(endpoint->wMaxPacketSize, 0, &_CameraIn);
        if (err != ESP_OK)
        {
            _CameraIn = NULL;
            ESP_LOGI("", "usb_host_transfer_alloc In fail: %x", err);
            return;
        }
        _CameraIn->device_handle = _Device_Handle;
        _CameraIn->bEndpointAddress = endpoint->bEndpointAddress;
        _CameraIn->callback = _camera_transfer_cb;
        _CameraIn->context = NULL;
        _CameraIn->num_bytes = endpoint->wMaxPacketSize;

        esp_err_t err = usb_host_transfer_submit(_CameraIn);
        if (err != ESP_OK)
        {
            ESP_LOGI("", "usb_host_transfer_submit In fail: %x", err);
        }
        else
        {
            Serial.println("host transfet IN init");
        }

        Serial.printf("USB camera ready\n");
    }
    else
    {
        Serial.println("(OUT)endpoint processing");
        err = usb_host_transfer_alloc(endpoint->wMaxPacketSize * 8, 0, &_CameraOut);
        if (err != ESP_OK)
        {
            _CameraOut = NULL;
            ESP_LOGI("", "usb_host_transfer_alloc In fail: %x", err);
            return;
        }
        _CameraOut->device_handle = _Device_Handle;
        _CameraOut->bEndpointAddress = endpoint->bEndpointAddress;
        _CameraOut->callback = _camera_transfer_cb;
        _CameraOut->context = NULL;

        /*esp_err_t err = usb_host_transfer_submit(_CameraOut);
        if (err != ESP_OK)
        {
            ESP_LOGI("", "usb_host_transfer_submit Out fail: %x", err);
        }*/

        Serial.printf("USB camera ready\n");
    }
}

void MyCamera::_show_config_desc_full(const usb_config_desc_t *config_desc)
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
                _check_interface_desc_camera(p);
                break;
            case USB_B_DESCRIPTOR_TYPE_ENDPOINT:
                Serial.println("USB_B_DESCRIPTOR_TYPE_ENDPOINT");
                show_endpoint_desc(p);
                // if (_isCamera && _CameraIn == NULL)
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

void MyCamera::loopUsb()
{
    // Serial.println("loopUsb()");

    _usbh_task();
    /*static unsigned long InterruptTimer = millis();
    if (millis() > 5000 && isCameraReady && !_isCameraPolling && (millis() - InterruptTimer > CameraInterval))
    {
        // Serial.println("isCameraReady && !_isCameraPolling && (millis() - InterruptTimer > _CameraInterval)");
        _CameraInterrupt->num_bytes = 8;
        esp_err_t err = usb_host_transfer_submit(_CameraInterrupt);
        if (err != ESP_OK)
        {
            Serial.printf("(Interrupt) usb_host_transfer_submit In fail: %x\n", err);
        }
        _isCameraPolling = true;
        InterruptTimer = 0;
    }*/
}

void MyCamera::flushQueue()
{
    PtpTransfer transfer;
    uint8_t number = 0;
    _usbh_task();
    _usbh_task();
    while (xQueueReceive(xQueue1, &transfer, 0) == pdTRUE)
    {
        number++;
        _usbh_task();
        _usbh_task();
    }
    if (number)
        Serial.printf("flush %d message\n", number);
}
uint16_t MyCamera::getResponseCode(uint32_t timeout, uint8_t *&data, uint32_t &total)
{
    uint16_t responseCode = 0;
    PtpTransfer transfer;

    _usbh_task();
    _usbh_task();

    uint8_t number = 0;
    uint32_t pending = 0;
    unsigned long timerGet = millis();

    while (millis() - timerGet < timeout) // 5200-5000<200
    {
        if (xQueueReceive(xQueue1, &transfer, 5) == pdTRUE)
        {
            number++;
            timerGet = millis() - timeout + 100;
            if (transfer.data[4] == PTP_USB_CONTAINER_DATA || pending != 0)
            {
                // Serial.printf("%d+%d=%d/%d\n", transfer.offset, transfer.len, transfer.offset + transfer.len, transfer.total);
                if (total == 0)
                {
                    total = transfer.total;
                    data = new uint8_t[total];
                    if (data == NULL)
                    {
                        Serial.printf("new failed: %d !!!!", total);
                        delay(5000);
                    }
                    memset(data, 0, total);
                }
                memcpy(data + transfer.offset, transfer.data, transfer.len);

                pending = total - (transfer.offset + transfer.len);
            }
            else if (transfer.data[4] == PTP_USB_CONTAINER_RESPONSE)
            {
                /*Serial.print("response: ");
                for (uint8_t i = 0; i < transfer.len; i++)
                {
                    Serial.printf("%02x ", transfer.data[i]);
                }
                Serial.println();*/
                responseCode = transfer.data[6] | (transfer.data[7] << 8);
                /*if (responseCode == 0x2001)
                {
                    Serial.printf("responseCode: %04x\n", responseCode);
                }*/
                return responseCode;
            }
        }
        _usbh_task();
    }

    Serial.printf("get %d messages\n", number);
    return responseCode;
}

const char *const MyCamera::prNames[] =
    {
        msgUndefined,
        msgBatteryLevel,
        msgFunctionalMode,
        msgImageSize,
        msgCompressionSetting,
        msgWhiteBalance,
        msgRGBGain,
        msgFNumber,
        msgFocalLength,
        msgFocusDistance,
        msgFocusMode,
        msgExposureMeteringMode,
        msgFlashMode,
        msgExposureTime,
        msgExposureProgramMode,
        msgExposureIndex,
        msgExposureBiasCompensation,
        msgDateTime,
        msgCaptureDelay,
        msgStillCaptureMode,
        msgContrast,
        msgSharpness,
        msgDigitalZoom,
        msgEffectMode,
        msgBurstNumber,
        msgBurstInterval,
        msgTimelapseNumber,
        msgTimelapseInterval,
        msgFocusMeteringMode,
        msgUploadURL,
        msgArtist,
        msgCopyrightInfo};

/*void MyCamera::PrintDevProp(uint8_t **pp, uint16_t *pcntdn)
{
    uint16_t op = *((uint16_t *)(*pp));

    Serial.print("\r\nDevice Property:\t");

    if ((((op >> 8) & 0xFF) == 0x50) && ((op & 0xFF) <= (PTP_DPC_CopyrightInfo & 0xFF)))
    {
        Serial.printf("%04x", op);
        Serial.print("\t");
        Serial.print(prNames[(op & 0xFF)]);
        Serial.print("\r\n");
    }
    else
    {
        Serial.printf("%04x", op);
        Serial.print(" (Vendor defined)\r\n");
    }
    (*pp) += 2;
    (*pcntdn) -= 2;
}*/

/**
 * Output of a Nikon D3300:
    0xd015 PS_DPC_AFMode
    0xd01f PS_DPC_ExpCompensation
    0xd032 PS_DPC_CameraModel
    0xd037 PS_DPC_DispAv
    0xd05d PTP_DPC_NIKON_LiveViewAFArea
    0xd06b PTP_DPC_NIKON_LongExposureNoiseReduction
    0xd08a PTP_DPC_NIKON_NoCFCard
    0xd08f PTP_DPC_NIKON_CleanImageSensor
    0xd09c PTP_DPC_NIKON_RetractableLensWarning
    0xd0a0 PTP_DPC_NIKON_MovScreenSize
    0xd0a4 PTP_DPC_NIKON_MovRecProhibitCondition
    0xd0b5 PTP_DPC_NIKON_ISOControlSensitivity
    0xd0e0 PTP_DPC_NIKON_LensID
    0xd0e1 PTP_DPC_NIKON_LensSort
    0xd0e2 PTP_DPC_NIKON_LensType
    0xd0e3 PTP_DPC_NIKON_FocalLengthMin
    0xd0e4 PTP_DPC_NIKON_FocalLengthMax
    0xd100 PTP_DPC_NIKON_ExposureTime
    0xd101 PTP_DPC_NIKON_ACPower
    0xd102 PTP_DPC_NIKON_WarningStatus
    0xd10b PTP_DPC_NIKON_RecordingMedia
    0xd14e PTP_DPC_NIKON_ActiveDLighting
    0xd161 PTP_DPC_NIKON_AutofocusMode
    0xd163 PTP_DPC_NIKON_AFAssist
    0xd16a PTP_DPC_NIKON_ISO_Auto
    0xd1a2 PTP_DPC_NIKON_LiveViewStatus
    0xd1a3 PTP_DPC_NIKON_LiveViewImageZoomRatio
    0xd1a4 PTP_DPC_NIKON_LiveViewProhibitCondition
    0xd1ac PTP_DPC_NIKON_LiveViewMovieMode
    0xd1b5 PTP_DPC_CANON_EOS_EVFClickWBCoeffs
    0xd1f0 PTP_DPC_NIKON_ApplicationMode
    0xd1f1 PTP_DPC_NIKON_ExposureRemaining
    0xd200 PTP_DPC_NIKON_ActivePicCtrlItem
    0xd201 PTP_DPC_NIKON_ChangePicCtrlItem
    0xd303 PTP_DPC_NIKON_UseDeviceStageFlag
    0xd406 PTP_DPC_MTP_SessionInitiatorInfo
    0xd407 PTP_DPC_MTP_PerceivedDeviceType
    0x5001 PTP_DPC_BatteryLevel
    0x5003 PTP_DPC_ImageSize
    0x5004 PTP_DPC_CompressionSetting
    0x5005 PTP_DPC_WhiteBalance
    0x5007 PTP_DPC_FNumber
    0x5008 PTP_DPC_FocalLength
    0x500a PTP_DPC_FocusMode
    0x500b PTP_DPC_ExposureMeteringMode
    0x500c PTP_DPC_FlashMode
    0x500d PTP_DPC_ExposureTime
    0x500e PTP_DPC_ExposureProgramMode
    0x500f PTP_DPC_ExposureIndex
    0x5010 PTP_DPC_ExposureBiasCompensation
    0x5011 PTP_DPC_DateTime
    0x5013 PTP_DPC_StillCaptureMode
    0x5018 PTP_DPC_BurstNumber
    0x501c PTP_DPC_FocusMeteringMode
 *
 */

void MyCamera::getDevProps()
{

    uint16_t propCount = 0;
    uint16_t prefix[] = {0xD000, 0xD080, 0xD100, 0xD180, 0xD200, 0xD300, 0xD400, 0x5000};

    for (uint8_t i = 0; i < sizeof(prefix) / sizeof(prefix[0]); i++)
    {
        for (uint8_t j = 0; j < 128; j++)
        {

            uint32_t param = (prefix[i] | j);
            Operation(PTP_OC_GetDevicePropDesc, 1, &param);

            uint8_t *data = NULL;
            uint32_t total = 0;
            uint16_t responseCode = getResponseCode(200, data, total);

            if (data != NULL)
            {
                devProp.Decode(total, data);
                propCount++;
                delete[] data;
            }
        }
    }

    Serial.printf("getDevProps(%d) DONE !\n", propCount);
}

void MyCamera::Operation(uint16_t opCode, uint8_t nbrParams, uint32_t *params)
{
    flushQueue();

    static uint32_t idTransaction = 0;

    idTransaction++;
    if (opCode == PTP_OC_OpenSession)
    {
        idTransaction = 0;
    }

    uint16_t operationLen = _min(64, 12 + nbrParams * 4);

    memset(_CameraOut->data_buffer, 0, operationLen);
    _CameraOut->num_bytes = operationLen;
    _CameraOut->data_buffer[0] = (_CameraOut->num_bytes & 0xff);
    _CameraOut->data_buffer[1] = (_CameraOut->num_bytes & 0xff00) >> 8;
    _CameraOut->data_buffer[2] = (_CameraOut->num_bytes & 0xff0000) >> 16;
    _CameraOut->data_buffer[3] = (_CameraOut->num_bytes & 0xff000000) >> 24;
    _CameraOut->data_buffer[4] = (PTP_USB_CONTAINER_COMMAND & 0xff);
    _CameraOut->data_buffer[5] = (PTP_USB_CONTAINER_COMMAND & 0xff00) >> 8;
    _CameraOut->data_buffer[6] = (opCode & 0xff);
    _CameraOut->data_buffer[7] = (opCode & 0xff00) >> 8;
    _CameraOut->data_buffer[8] = (idTransaction & 0xff);
    _CameraOut->data_buffer[9] = (idTransaction & 0xff00) >> 8;
    _CameraOut->data_buffer[10] = (idTransaction & 0xff0000) >> 16;
    _CameraOut->data_buffer[11] = (idTransaction & 0xff000000) >> 24;
    _CameraOut->data_buffer[12] = 0x01;
    _CameraOut->data_buffer[13] = 0x00;
    _CameraOut->data_buffer[14] = 0;
    _CameraOut->data_buffer[15] = 0;
    for (uint8_t i = 0; i < nbrParams; i++)
    {
        _CameraOut->data_buffer[12 + i * 4] = (params[i] & 0xff);
        _CameraOut->data_buffer[13 + i * 4] = (params[i] & 0xff00) >> 8;
        _CameraOut->data_buffer[14 + i * 4] = (params[i] & 0xff0000) >> 16;
        _CameraOut->data_buffer[15 + i * 4] = (params[i] & 0xff000000) >> 24;
    }

    esp_err_t err = usb_host_transfer_submit(_CameraOut);

    if (err != ESP_OK)
    {
        Serial.printf("usb_host_transfer_submit fail. opCode: %04x, err: %x\n", opCode, err);
    }
}

void MyCamera::getImage()
{
    static unsigned long timerGetImage = millis();
    static bool liveViewStatus = getLiveViewStatus();
    if (liveViewStatus)
    {
        Operation(PTP_OC_NIKON_GetLiveViewImg, 0, NULL);

        uint8_t *data = NULL;
        uint32_t total = 0;
        uint16_t responseCode = getResponseCode(200, data, total);
        Serial.printf("total Image LiveView: %d\n", total);

        if (responseCode == PTP_RC_OK && data != NULL)
        {
            /*Serial.println("LiveView image:");
            for (uint32_t i = 0; i < total; i++)
            {
                Serial.printf("%02x", data[i]);
                if ((i + 1) % 32 == 0)
                {
                    Serial.println();
                }
            }
            Serial.println();*/
            Serial.printf("image received in %lums.\n", millis() - timerGetImage);
            timerGetImage = millis();
        }
        else
        {
            Serial.println("Error getting liveView image");
        }

        if (data != NULL)
        {
            delete[] data;
        }
    }
    else
    {
        setLiveViewStatus(1);
        liveViewStatus = getLiveViewStatus();
    }
}

bool MyCamera::getLiveViewStatus()
{
    DevProp::DevPropDesc propDesc = *DevProp::getDesc(PTP_DPC_NIKON_LiveViewStatus);
    uint32_t valueReturned = getDevProp(propDesc);
    uint8_t liveViewStatus = *(uint8_t *)&valueReturned;
    return liveViewStatus;
}

void MyCamera::setLiveViewStatus(bool status)
{
    Serial.printf("Setting liveView to %d\n", status);
    uint16_t opCode = status ? PTP_OC_NIKON_StartLiveView : PTP_OC_NIKON_EndLiveView;
    Operation(PTP_OC_NIKON_StartLiveView, 0, NULL);
    uint8_t *data = NULL;
    uint32_t total = 0;
    uint16_t responseCode = getResponseCode(1000, data, total);

    if (responseCode != PTP_RC_OK)
    {
        Serial.printf("fail to setLiveView %04x\n", responseCode);
    }
    if (data != NULL)
    {
        delete[] data;
    }
}

void MyCamera::checkEvent()
{
    Operation(NK_OC_CheckEvent, 0, NULL);
    uint8_t *data = NULL;
    uint32_t total = 0;
    uint16_t responseCode = getResponseCode(200, data, total);

    if (responseCode == PTP_RC_OK && data != NULL)
    {
        Nikon::PrintValue(ValueTypeEvent, data, total);
    }
    else
    {
        Serial.println("Error getting Event");
    }

    if (data != NULL)
    {
        delete[] data;
    }
}

void MyCamera::openSession()
{
    uint32_t param = 1;
    Operation(PTP_OC_OpenSession, 1, &param);
}

uint32_t MyCamera::getDevProp(DevProp::DevPropDesc propDesc)
{
    uint32_t param = propDesc.op;
    Operation(PTP_OC_GetDevicePropValue, 1, &param);
    uint8_t *data = NULL;
    uint32_t total = 0;
    uint16_t responseCode = getResponseCode(200, data, total);

    uint32_t data32 = 0;
    if (total > 12)
    {
        for (uint8_t i = 0; i < total - 12; i++)
        {
            ((uint8_t *)&data32)[i] = data[12 + i];
        }
    }
    if (responseCode == PTP_RC_OK && data != NULL)
    {
        Nikon::PrintValue(propDesc.type, data, total);
    }
    else
    {
        Serial.printf("Error getting prop: %04x, responseCode: %04x\n", propDesc.op, responseCode);
    }
    if (data != NULL)
    {
        delete[] data;
    }
    return data32;
}

void MyCamera::getAperture()
{
    Serial.println("getAperture()");
    DevProp::DevPropDesc *propDesc = DevProp::getDesc(PTP_DPC_FNumber);
    getDevProp(*propDesc);
    /*uint32_t param = PTP_DPC_FNumber;
    Operation(PTP_OC_GetDevicePropValue, 1, &param);

    uint8_t *data = NULL;
    uint32_t total = 0;
    uint16_t responseCode = getResponseCode(200, data, total);

    if (responseCode == PTP_RC_OK && data != NULL)
    {
        Nikon::PrintValue(ValueTypeAperture, data, total);
    }
    else
    {
        Serial.println("Error getting Aperture");
    }

    if (data != NULL)
    {
        delete[] data;
    }*/
}

void MyCamera::_client_event_callback(const usb_host_client_event_msg_t *event_msg, void *arg)
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

void MyCamera::_usbh_setup(usb_host_enum_cb_t enumeration_cb)
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

void MyCamera::_usbh_task(void)
{
    // Serial.println("_usbh_task()");

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