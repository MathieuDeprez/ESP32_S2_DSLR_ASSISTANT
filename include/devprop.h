#ifndef __DEVPROPPARSER_H__
#define __DEVPROPPARSER_H__

#include <ptpconst.h>
#include "nikon.h"

// Device properties
const char msgUndefined[] = "Undefined";
const char msgBatteryLevel[] = "BatteryLevel";
const char msgFunctionalMode[] = "FunctionalMode";
const char msgImageSize[] = "ImageSize";
const char msgCompressionSetting[] = "CompressionSetting";
const char msgWhiteBalance[] = "WhiteBalance";
const char msgRGBGain[] = "RGBGain";
const char msgFNumber[] = "FNumber";
const char msgFocalLength[] = "FocalLength";
const char msgFocusDistance[] = "FocusDistance";
const char msgFocusMode[] = "FocusMode";
const char msgExposureMeteringMode[] = "ExposureMeteringMode";
const char msgFlashMode[] = "FlashMode";
const char msgExposureTime[] = "ExposureTime";
const char msgExposureProgramMode[] = "ExposureProgramMode";
const char msgExposureIndex[] = "ExposureIndex";
const char msgExposureBiasCompensation[] = "ExposureBiasCompensation";
const char msgDateTime[] = "DateTime";
const char msgCaptureDelay[] = "CaptureDelay";
const char msgStillCaptureMode[] = "StillCaptureMode";
const char msgContrast[] = "Contrast";
const char msgSharpness[] = "Sharpness";
const char msgDigitalZoom[] = "DigitalZoom";
const char msgEffectMode[] = "EffectMode";
const char msgBurstNumber[] = "BurstNumber";
const char msgBurstInterval[] = "BurstInterval";
const char msgTimelapseNumber[] = "TimelapseNumber";
const char msgTimelapseInterval[] = "TimelapseInterval";
const char msgFocusMeteringMode[] = "FocusMeteringMode";
const char msgUploadURL[] = "UploadURL";
const char msgArtist[] = "Artist";
const char msgCopyrightInfo[] = "CopyrightInfo";
const char msgExpoCompensation[] = "ExpoCompensation";
const char msgCameraModel[] = "CameraModel";
const char msgDispAv[] = "DispAv";
const char msgLongExposureNoiseReduction[] = "LongExposureNoiseReduction";
const char msgNoCFCard[] = "NoCFCard";
const char msgLensID[] = "LensID";
const char msgFocalLengthMin[] = "FocalLengthMin";
const char msgFocalLengthMax[] = "FocalLengthMax";
const char msgACPower[] = "ACPower";
const char msgAutofocusMode[] = "AutofocusMode";
const char msgAFAssist[] = "AFAssist";

// Data Types
const char msgUNDEF[] = "UNDEF";
const char msgINT8[] = "INT8";
const char msgUINT8[] = "UINT8";
const char msgINT16[] = "INT16";
const char msgUINT16[] = "UINT16";
const char msgINT32[] = "INT32";
const char msgUINT32[] = "UINT32";
const char msgINT64[] = "INT64";
const char msgUINT64[] = "UINT64";
const char msgINT128[] = "INT128";
const char msgUINT128[] = "UINT128";
const char msgAINT8[] = "AINT8";
const char msgAUINT8[] = "AUINT8";
const char msgAINT16[] = "AINT16";
const char msgAUINT16[] = "AUINT16";
const char msgAINT32[] = "AINT32";
const char msgAUINT32[] = "AUINT32";
const char msgAINT64[] = "AINT64";
const char msgAUINT64[] = "AUINT64";
const char msgAINT128[] = "AINT128";
const char msgAUINT128[] = "AUINT128";
const char msgSTR[] = "STR";

class DevProp
{
    typedef struct
    {
        uint16_t op;
        const char *desc;
    } DevPropDesc;

    static DevPropDesc devPropDesc[];
    static const char *const dtNames1[];
    static const char *const dtNames2[];
    static const char *const prNames[];

    uint16_t dataType;
    uint8_t bytesSize;

    void PrintDevProp(uint8_t **pp, uint16_t *pcntdn);
    void PrintDataType(uint8_t **pp, uint16_t *pcntdn);
    void PrintGetSet(uint8_t **pp, uint16_t *pcntdn);
    bool PrintValue(uint8_t **pp, uint16_t *pcntdn);
    bool PrintEnum(uint8_t **pp, uint16_t *pcntdn);
    bool ParseEnum(uint8_t **pp, uint16_t *pcntdn);
    bool ParseEnumSingle(uint8_t **pp, uint16_t *pcntdn);
    bool ParseEnumArray(uint8_t **pp, uint16_t *pcntdn);

    uint8_t GetDataSize();

public:
    void Decode(const uint16_t len, const uint8_t *const pbuf);
    void printEnum(uint8_t len_size, uint8_t val_size, const uint8_t *p);
};

#endif // __DEVPROPPARSER_H__
