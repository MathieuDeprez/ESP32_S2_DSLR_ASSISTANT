#include "devprop.h"

DevProp devProp;

const char *const DevProp::dtNames1[] =
    {
        msgUNDEF,
        msgINT8,
        msgUINT8,
        msgINT16,
        msgUINT16,
        msgINT32,
        msgUINT32,
        msgINT64,
        msgUINT64,
        msgINT128,
        msgUINT128};

const char *const DevProp::dtNames2[] =
    {
        msgUNDEF,
        msgAINT8,
        msgAUINT8,
        msgAINT16,
        msgAUINT16,
        msgAINT32,
        msgAUINT32,
        msgAINT64,
        msgAUINT64,
        msgAINT128,
        msgAUINT128};

DevProp::DevPropDesc DevProp::devPropDesc[] =
    {
        {PTP_DPC_Undefined, msgUndefined, ValueTypeNone},
        {PTP_DPC_BatteryLevel, msgBatteryLevel, ValueTypeNone},
        {PTP_DPC_FunctionalMode, msgFunctionalMode, ValueTypeNone},
        {PTP_DPC_ImageSize, msgImageSize, ValueTypeNone},
        {PTP_DPC_CompressionSetting, msgCompressionSetting, ValueTypeNone},
        {PTP_DPC_WhiteBalance, msgWhiteBalance, ValueTypeNone},
        {PTP_DPC_RGBGain, msgRGBGain, ValueTypeNone},
        {PTP_DPC_FNumber, msgFNumber, ValueTypeAperture},
        {PTP_DPC_FocalLength, msgFocalLength, ValueTypeNone},
        {PTP_DPC_FocusDistance, msgFocusDistance, ValueTypeNone},
        {PTP_DPC_FocusMode, msgFocusMode, ValueTypeNone},
        {PTP_DPC_ExposureMeteringMode, msgExposureMeteringMode, ValueTypeNone},
        {PTP_DPC_FlashMode, msgFlashMode, ValueTypeNone},
        {PTP_DPC_ExposureTime, msgExposureTime, ValueTypeExposureTime},
        {NK_DPC_ExposureTime, msgExposureTime, ValueTypeNone},
        {PTP_DPC_ExposureProgramMode, msgExposureProgramMode, ValueTypeNone},
        {PTP_DPC_ExposureIndex, msgExposureIndex, ValueTypeNone},
        {PTP_DPC_ExposureBiasCompensation, msgExposureBiasCompensation, ValueTypeNone},
        {PTP_DPC_DateTime, msgDateTime, ValueTypeNone},
        {PTP_DPC_CaptureDelay, msgCaptureDelay, ValueTypeNone},
        {PTP_DPC_StillCaptureMode, msgStillCaptureMode, ValueTypeNone},
        {PTP_DPC_Contrast, msgContrast, ValueTypeNone},
        {PTP_DPC_Sharpness, msgSharpness, ValueTypeNone},
        {PTP_DPC_DigitalZoom, msgDigitalZoom, ValueTypeNone},
        {PTP_DPC_EffectMode, msgEffectMode, ValueTypeNone},
        {PTP_DPC_BurstNumber, msgBurstNumber, ValueTypeNone},
        {PTP_DPC_BurstInterval, msgBurstInterval, ValueTypeNone},
        {PTP_DPC_TimelapseNumber, msgTimelapseNumber, ValueTypeNone},
        {PTP_DPC_TimelapseInterval, msgTimelapseInterval, ValueTypeNone},
        {PTP_DPC_FocusMeteringMode, msgFocusMeteringMode, ValueTypeNone},
        {PTP_DPC_UploadURL, msgUploadURL, ValueTypeNone},
        {PTP_DPC_Artist, msgArtist, ValueTypeNone},
        {PTP_DPC_CopyrightInfo, msgCopyrightInfo, ValueTypeNone},
        {PS_DPC_AFMode, msgAutofocusMode, ValueTypeNone},
        {PS_DPC_ExpCompensation, msgExpoCompensation, ValueTypeNone},
        {PS_DPC_CameraModel, msgCameraModel, ValueTypeNone},
        {PS_DPC_DispAv, msgDispAv, ValueTypeNone},
        {NK_DPC_LongExposureNoiseReduction, msgLongExposureNoiseReduction, ValueTypeNone},
        {NK_DPC_NoCFCard, msgNoCFCard, ValueTypeNone},
        {NK_DPC_LensID, msgLensID, ValueTypeNone},
        {NK_DPC_FocalLengthMin, msgFocalLengthMin, ValueTypeNone},
        {NK_DPC_FocalLengthMax, msgFocalLengthMax, ValueTypeNone},
        {NK_DPC_ACPower, msgACPower, ValueTypeNone},
        {NK_DPC_AutofocusMode, msgAutofocusMode, ValueTypeNone},
        {NK_DPC_AFAssist, msgAFAssist, ValueTypeNone},
        {PTP_DPC_NIKON_LiveViewStatus, msgLiveView, ValueTypeLiveView},

};

const char *const DevProp::prNames[] =
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

void DevProp::PrintDataType(uint8_t **pp, uint16_t *pcntdn)
{
    dataType = *((uint16_t *)(*pp));
    bytesSize = GetDataSize();

    Serial.print("Data Type:\t\t");

    switch (((dataType >> 8) & 0xFF))
    {
    case 0x00:
        if ((dataType & 0xFF) <= (PTP_DTC_UINT128 & 0xFF))
            Serial.print(dtNames1[(dataType & 0xFF)]);
        break;
    case 0x40:
        if ((dataType & 0xFF) <= (PTP_DTC_AUINT128 & 0xFF))
            Serial.print(dtNames2[(dataType & 0xFF)]);
        break;
    case 0xFF:
        Serial.print("STR");
        break;
    default:
        Serial.print("Unknown");
    }
    Serial.print("\r\n");
    (*pp) += 2;
    (*pcntdn) -= 2;
}

void DevProp::PrintDevProp(uint8_t **pp, uint16_t *pcntdn)
{
    uint16_t op = *((uint16_t *)(*pp));

    Serial.print("\r\nDevice Property:\t");

    /*if ((((op >> 8) & 0xFF) == 0x50) && ((op & 0xFF) <= (PTP_DPC_CopyrightInfo & 0xFF)))
    {
        Serial.printf("%04x", op);
        Serial.print("\t");
        Serial.print(prNames[(op & 0xFF)]);
        Serial.print("\r\n");
    }
    else
    {*/

    DevPropDesc *propDesc = getDesc(op);
    if (propDesc != NULL)
    {
        Serial.printf("%04x", op);
        Serial.print("\t");
        Serial.print(propDesc->desc);
        Serial.print("\r\n");
    }
    else
    {
        Serial.printf("%04x", op);
        Serial.print(" (Vendor defined)\r\n");
    }
    //}
    (*pp) += 2;
    (*pcntdn) -= 2;
}

DevProp::DevPropDesc *DevProp::getDesc(uint16_t op)
{
    bool descFound = false;
    for (uint16_t i = 0; i < sizeof(devPropDesc) / sizeof(devPropDesc[0]); i++)
    {
        if (devPropDesc[i].op == op)
        {
            /*Serial.printf("%04x", op);
            Serial.print("\t");
            Serial.print(devPropDesc[i].desc);
            Serial.print("\r\n");*/
            return &devPropDesc[i];
        }
    }
    return NULL;
}

void DevProp::PrintGetSet(uint8_t **pp, uint16_t *pcntdn)
{
    Serial.print("Get/Set:\t\t");
    Serial.print(((**pp) == 0x01) ? "Get/Set\r\n" : (!(**pp)) ? "Get\r\n"
                                                              : "Illegal\r\n");
    (*pp)++;
    (*pcntdn)--;
}

uint8_t DevProp::GetDataSize()
{
    switch (dataType)
    {
    case PTP_DTC_INT8:
    case PTP_DTC_UINT8:
        bytesSize = 1;
        break;
    case PTP_DTC_AINT8:
    case PTP_DTC_AUINT8:
        bytesSize = 1;
        // enumParser.Initialize(4, bytesSize, &theBuffer);
        break;
    case PTP_DTC_INT16:
    case PTP_DTC_UINT16:
        bytesSize = 2;
        // enumParser.Initialize(4, bytesSize, &theBuffer);
        break;
    case PTP_DTC_AINT16:
    case PTP_DTC_AUINT16:
        bytesSize = 2;
        break;
    case PTP_DTC_STR:
        bytesSize = 2;
        // enumParser.Initialize(1, bytesSize, &theBuffer);
        break;
    case PTP_DTC_INT32:
    case PTP_DTC_UINT32:
        bytesSize = 4;
        break;
    case PTP_DTC_AINT32:
    case PTP_DTC_AUINT32:
        bytesSize = 4;
        // enumParser.Initialize(4, bytesSize, &theBuffer);
        break;
    case PTP_DTC_INT64:
    case PTP_DTC_UINT64:
        bytesSize = 8;
        break;
    case PTP_DTC_AINT64:
    case PTP_DTC_AUINT64:
        bytesSize = 8;
        // enumParser.Initialize(4, bytesSize, &theBuffer);
        break;
    case PTP_DTC_INT128:
    case PTP_DTC_UINT128:
        bytesSize = 16;
        break;
    case PTP_DTC_AINT128:
    case PTP_DTC_AUINT128:
        bytesSize = 16;
        // enumParser.Initialize(4, bytesSize, &theBuffer);
        break;
    }
    return bytesSize;
}

bool DevProp::PrintValue(uint8_t **pp, uint16_t *pcntdn)
{
    switch (dataType)
    {
    case PTP_DTC_INT8:
    case PTP_DTC_UINT8:
        Serial.printf("%02x", **pp);
        (*pp)++;
        (*pcntdn)--;
        break;
    case PTP_DTC_AINT8:
    case PTP_DTC_AUINT8:
        /*if (!enumParser.Parse(pp, pcntdn, (PTP_ARRAY_EL_FUNC)&PrintByte))
            return false;*/
        break;
    case PTP_DTC_INT16:
    case PTP_DTC_UINT16:
        Serial.printf("%04x", *(uint16_t *)&**pp);
        (*pp) += 2;
        (*pcntdn) -= 2;
        break;
    case PTP_DTC_AINT16:
    case PTP_DTC_AUINT16:
        /*if (!enumParser.Parse(pp, pcntdn, (PTP_ARRAY_EL_FUNC)&PrintTwoBytes))
            return false;*/
        break;
    case PTP_DTC_INT32:
    case PTP_DTC_UINT32:
        Serial.printf("%08x", *(uint32_t *)&**pp);
        (*pp) += 4;
        (*pcntdn) -= 4;
        break;
    case PTP_DTC_AINT32:
    case PTP_DTC_AUINT32:
        /*if (!enumParser.Parse(pp, pcntdn, (PTP_ARRAY_EL_FUNC)&PrintFourBytes))
            return false;*/
        break;
    case PTP_DTC_INT64:
    case PTP_DTC_UINT64:
        (*pp) += 8;
        (*pcntdn) -= 8;
        break;
    case PTP_DTC_AINT64:
    case PTP_DTC_AUINT64:
        /*if (!enumParser.Parse(pp, pcntdn, (PTP_ARRAY_EL_FUNC)&PrintEightBytes))
            return false;*/
        break;
    case PTP_DTC_INT128:
    case PTP_DTC_UINT128:
        (*pp) += 16;
        (*pcntdn) -= 16;
        break;
    case PTP_DTC_AINT128:
    case PTP_DTC_AUINT128:
        /*if (!enumParser.Parse(pp, pcntdn, NULL))
            return false;*/
        break;
    case PTP_DTC_STR:
        /*if (!enumParser.Parse(pp, pcntdn, (PTP_ARRAY_EL_FUNC)&PrintChar))
            return false;*/
        char *text = (char *)&**pp;
        for (uint16_t i = 0; *pcntdn; (*pcntdn)--, i++)
        {
            Serial.print(text[i]);
        }
        break;
    }
    return true;
}

bool DevProp::ParseEnum(uint8_t **pp, uint16_t *pcntdn)
{
    if ((dataType & 0xFF00) == 0)
        return ParseEnumSingle(pp, pcntdn);
    else
        return ParseEnumArray(pp, pcntdn);
}

bool DevProp::ParseEnumArray(uint8_t **pp, uint16_t *pcntdn)
{
    Serial.printf("%08x", *(uint32_t *)&**pp);
    return true;
    /*
    switch (enStage)
    {
    case 0:
        theBuffer.valueSize = 2;
        valParser.Initialize(&theBuffer);
        enStage = 1;
    case 1:
        if (!valParser.Parse(pp, pcntdn))
            return false;
        enLen = 0;
        enLen = *((uint16_t *)theBuffer.pValue);
        enLenCntdn = enLen;
        enumParser.Initialize(4, bytesSize, &theBuffer);
        enStage = 2;
    case 2:
        for (; enLenCntdn; enLenCntdn--)
        {
            if (!enumParser.Parse(pp, pcntdn, (PTP_ARRAY_EL_FUNC)&PrintEnumValue) )
                return false;

            Serial.print("\r\n");
            enumParser.Initialize(4, bytesSize, &theBuffer);
        }
        enStage = 0;
    } // switch
    return true;*/
}

bool DevProp::ParseEnumSingle(uint8_t **pp, uint16_t *pcntdn)
{
    uint16_t number_of_enum = *(uint16_t *)&**pp;
    uint8_t *p = (uint8_t *)&**pp;
    p += 2;

    // Serial.printf("number_of_enum:%d,%d.", number_of_enum, bytesSize);

    for (uint8_t i = 0; i < number_of_enum; i++)
    {
        uint32_t val = 0;
        for (uint8_t y = 0; y < bytesSize; y++)
        {
            val |= p[i * bytesSize + y] << (y * 8);
        }
        Serial.printf(bytesSize == 1 ? "%02x" : bytesSize == 2 ? "%04x"
                                                               : "%08x",
                      val);
        if (i < number_of_enum - 1)
        {
            Serial.print(",");
        }
    }

    return true;
    /*switch (enStage)
    {
    case 0:
        enumParser.Initialize(2, bytesSize, &theBuffer);
        //        Serial.println(bytesSize, DEC);
        enStage = 1;
    case 1:
        if (!enumParser.Parse(pp, pcntdn, (PTP_ARRAY_EL_FUNC)&PrintEnumValue, this))
            return false;
        enStage = 0;
    } // switch
    return true;*/
}

void DevProp::printEnum(uint8_t len_size, uint8_t val_size, const uint8_t *p)
{
    for (uint8_t i = 0; i < len_size; i += val_size)
    {
        uint32_t val = 0;
        for (uint8_t y = 0; y < val_size; y++)
        {
            val |= p[i * val_size + y] << (y * 8);
        }
        Serial.printf(val_size == 1 ? "%02x" : val_size == 2 ? "%04x"
                                                             : "%08%",
                      val);
        if (i < len_size - 1)
        {
            Serial.print(",");
        }
    }
    // Serial.printf("printEnum: %d, %d", len_size, val_size);
}

void DevProp::Decode(const uint16_t len, const uint8_t *pbuf)
{
    uint16_t cntdn = (uint16_t)len;
    uint8_t *p = (uint8_t *)pbuf;
    uint8_t nStage = 0;
    uint8_t formFlag = 0;

    switch (nStage)
    {
    case 0:
        p += 12;
        cntdn -= 12;
        nStage = 1;
    case 1:
        PrintDevProp(&p, &cntdn);
        nStage = 2; //++;
    case 2:
        PrintDataType(&p, &cntdn);
        nStage = 3; //++;
    case 3:
        PrintGetSet(&p, &cntdn);
        nStage = 4; //++;
    case 4:
        Serial.print("Default Value:\t\t");
        nStage = 5; //++;
    case 5:
        if (!PrintValue(&p, &cntdn))
            return;
        Serial.print("\r\n");
        nStage = 6; //++;
    case 6:
        Serial.print("Current Value:\t\t");
        nStage = 7; //++;
    case 7:
        if (!PrintValue(&p, &cntdn))
            return;
        Serial.print("\r\n");
        nStage = 8; //++;
    case 8:
        formFlag = (*p);
        p++;
        cntdn--;
        nStage = 9; //++;
    case 9:
        if (formFlag == 1)
        {
            Serial.print("Range (Min,Max,Step):\t\t{");
            // enumParser.Initialize(2, bytesSize, &theBuffer, PTPListParser::modeRange);
        }
        if (formFlag == 2)
            Serial.print("Enumeration:\t\t{");
        nStage = 10; //++;
    case 10:
        if (formFlag == 1)
            printEnum(3, bytesSize, p);
        /*if (!enumParser.Parse(&p, &cntdn, (PTP_ARRAY_EL_FUNC)&PrintEnumValue))
            return;*/

        if (formFlag == 2)
            if (!ParseEnum(&p, &cntdn))
                return;

        if (formFlag)
            Serial.print("}\r\n");

        nStage = 0;
    }
}
