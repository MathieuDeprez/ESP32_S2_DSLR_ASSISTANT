#include "main.h"

const ValueTitle<uint16_t, 4> ApertureTitles[] =
	{
		{0x015E, "3.5"},
		{0x0190, "4.0"},
		{0x01E0, "4.8"},
		{0x01C2, "4.5"},
		{0x01F4, "5"},
		{0x0230, "5.6"},
		{0x0276, "6.3"},
		{0x02C6, "7.1"},
		{0x0320, "8"},
		{0x0384, "9"},
		{0x03E8, "10"},
		{0x044C, "11"},
		{0x0514, "13"},
		{0x0578, "14"},
		{0x0640, "16"},
		{0x0708, "18"},
		{0x07D0, "20"},
		{0x0898, "22"},
		{0x09C4, "25"},
		{0x0B54, "29"},
		{0x0C80, "32"},
		{0x0E10, "36"},
};

const ValueTitle<uint8_t, 5> FocusModeTitles[] =
	{
		{0x00, "AF-S"},
		{0x01, "AF-C"},
		{0x02, "AF-A"},
		{0x04, "MF"},
};

const ValueTitle<uint8_t, 6> CompressionTitles[] =
	{
		{0x00, "BASIC"},
		{0x01, "NORM"},
		{0x02, "FINE"},
		{0x03, "RAW"},
		{0x04, "RAW+B"},
};

const ValueTitle<uint32_t, 5> ShutterSpeedTitles[] =
	{
		{0x00000002, "4000"},
		{0x00000003, "3200"},
		{0x00000004, "2500"},
		{0x00000005, "2000"},
		{0x00000006, "1600"},
		{0x00000008, "1250"},
		{0x0000000A, "1000"},
		{0x0000000C, "800"},
		{0x0000000F, "640"},
		{0x00000014, "500"},
		{0x00000019, "400"},
		{0x0000001F, "320"},
		{0x00000028, "250"},
		{0x00000032, "200"},
		{0x0000003E, "160"},
		{0x00000050, "125"},
		{0x00000064, "100"},
		{0x0000007D, "80"},
		{0x000000A6, "60"},
		{0x000000C8, "50"},
		{0x000000FA, "40"},
		{0x0000014D, "30"},
		{0x00000190, "25"},
		{0x000001F4, "20"},
		{0x0000029A, "15"},
		{0x00000301, "13"},
		{0x000003E8, "10"},
		{0x000004E2, "8"},
		{0x00000682, "6"},
		{0x000007D0, "5"},
		{0x000009C4, "4"},
		{0x00000D05, "3"},
		{0x00000FA0, "2.5"},
		{0x00001388, "2"},
		{0x0000186A, "1.6"},
		{0x00001E0C, "1.3"},
		{0x00002710, "1"},
		{0x000032C8, "1.3"},
		{0x00003E80, "1.6"},
		{0x00004E20, "2"},
		{0x000061A8, "2.5"},
		{0x00007530, "3"},
		{0x00009C40, "4"},
		{0x0000C350, "5"},
		{0x0000EA60, "6"},
		{0x00013880, "8"},
		{0x000186A0, "10"},
		{0x0001FBD0, "13"},
		{0x000249F0, "15"},
		{0x00030D40, "20"},
		{0x0003D090, "25"},
		{0x000493E0, "30"},
		{0xFFFFFFFF, "BulB"},
};

const ValueTitle<uint16_t, 5> IsoTitles[] =
	{
		{0x0064, "100"},
		{0x007D, "125"},
		{0x00A0, "160"},
		{0x00C8, "200"},
		{0x00FA, "250"},
		{0x0140, "320"},
		{0x0190, "400"},
		{0x01F4, "500"},
		{0x0280, "640"},
		{0x0320, "800"},
		{0x03E8, "1000"},
		{0x04E2, "1250"},
		{0x0640, "1600"},
		{0x07D0, "2000"},
		{0x09C4, "2500"},
		{0x0C80, "3200"},
		{0x0FA0, "4000"},
		{0x1388, "5000"},
		{0x1900, "6400"},
		{0x1F40, "Hi03"},
		{0x2710, "Hi07"},
		{0x3200, "Hi 1"},
		{0x6400, "hi 2"},
};

// Exposure Compensation Title Array
const ValueTitle<uint16_t, 5> ExpCompTitles[] =
	{
		{0xEC78, "+5.0"},
		{0xEDC6, "+4.7"},
		{0xEF13, "+4.3"},
		{0xF060, "+4.0"},
		{0xF1AE, "+3.7"},
		{0xF2FB, "+3.3"},
		{0xF448, "+3.0"},
		{0xF596, "+2.7"},
		{0xF6E3, "+2.3"},
		{0xF830, "+2.0"},
		{0xF97E, "+1.7"},
		{0xFACB, "+1.3"},
		{0xFC18, "+1.0"},
		{0xFD66, "+0.7"},
		{0xFEB3, "+0.3"},
		{0x0000, "0"},
		{0x014D, "-0.3"},
		{0x029A, "-0.7"},
		{0x03E8, "-1.0"},
		{0x0535, "-1.3"},
		{0x0682, "-1.7"},
		{0x07D0, "-2.0"},
		{0x091D, "-2.3"},
		{0x0A6A, "-2.7"},
		{0x0BB8, "-3.0"},
		{0x0D05, "-3.3"},
		{0x0E52, "-3.7"},
		{0x0FA0, "-4.0"},
		{0x10ED, "-4.3"},
		{0x123A, "-4.7"},
		{0x1388, "-5.0"},
};

// White Balance Title Array
const ValueTitle<uint16_t, 4> WbTitles[] =
	{
		{0x0002, "AWB"}, // Auto White Balance
		{0x0004, "Day"}, // Daylight
		{0x0005, "Flr"}, // Fluoriscent
		{0x0006, "Tng"}, // Tungsteen 2
		{0x0007, "Str"}, // Strobe
		{0x8010, "Cld"}, // Clouds
		{0x8011, "Shd"}, // Shade
		{0x8013, "PRE"}, // Preset
};

void Nikon::PrintValue(const ValueType valueType, const uint8_t *data, const uint32_t len)
{
	// Serial.println("PrintValue()");
	switch (valueType)
	{
	case ValueTypeAperture:
	{
		uint16_t apertureValue = *(uint16_t *)&data[12];
		bool found = false;
		for (uint8_t i = 0; i < sizeof(ApertureTitles) / sizeof(ApertureTitles[0]); i++)
		{
			if (ApertureTitles[i].value == apertureValue)
			{
				Serial.printf("Aperture: F%s\n", ApertureTitles[i].title);
				found = true;
				break;
			}
		}
		if (!found)
		{
			Serial.printf("Aperture title not found.");
		}
	}
	break;
	case ValueTypeExposureTime:
	{
		uint16_t exposureValue = *(uint16_t *)&data[12];
		bool found = false;
		for (uint8_t i = 0; i < sizeof(ShutterSpeedTitles) / sizeof(ShutterSpeedTitles[0]); i++)
		{
			if (ShutterSpeedTitles[i].value == exposureValue)
			{
				Serial.printf("Shutter Speed: %s\n", ShutterSpeedTitles[i].title);
				found = true;
				break;
			}
		}
		if (!found)
		{
			Serial.printf("Shutter Speed title not found.");
		}
	}
	break;
	case ValueTypeLiveView:
	{
		uint8_t liveViewStatus = *(uint8_t *)&data[12];
		Serial.printf("liveViewStatus: %d\n", liveViewStatus);
	}
	break;
	case ValueTypeEvent:
	{
		for (uint32_t i = 0; i < len; i++)
		{
			Serial.printf("%02x ", data[i]);
		}
		Serial.println();
		uint16_t numberOfEvents = *(uint16_t *)&data[12];
		for (uint16_t i = 0; i < numberOfEvents; i++)
		{
			uint16_t eventCode = *(uint16_t *)&data[i * 6 + 14];
			if (eventCode == PTP_EC_DevicePropChanged)
			{
				uint16_t devicePropertiesCode = *(uint16_t *)&data[i * 6 + 16];
				DevProp::DevPropDesc *propDesc = DevProp::getDesc(devicePropertiesCode);
				if (propDesc != NULL)
				{
					Serial.println(propDesc->desc);
					myCamera.getDevProp(*propDesc);
				}
				else
				{
					Serial.printf("%04x", devicePropertiesCode);
					Serial.print(" (Property not defined)\r\n");
				}
			}
			else
			{
				Serial.printf("EventCode: %04x\n", eventCode);
			}
		}
	}
	break;
	default:
		if (valueType == ValueTypeNone)
			break;
		Serial.printf("default valueType: %04x\n", valueType);
		break;
	}
}