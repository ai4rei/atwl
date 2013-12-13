/* -----------------------------------------------------------------
// List of Known Devices
//
// ---------------------------------------------------------------*/

#ifndef _KBDCDEVS_H_
#define _KBDCDEVS_H_

typedef enum KBDCDEVICETYPE
{
    KBDC_DEVTYPE_BARCODE,
    KBDC_DEVTYPE_RFID,
}
KBDCDEVICETYPE;

typedef struct KBDCDATA
{
    USHORT MakeCode;
    USHORT SourceType;
}
KBDCDATA,* LPKBDCDATA;
typedef const KBDCDATA* LPCKBDCDATA;

typedef struct KBDCDEVICE
{
    LPCWSTR HardwareId;
    LPCSTR  FriendlyName;
    USHORT  DeviceType;
}
KBDCDEVICE,* LPKBDCDEVICE;
typedef const KBDCDEVICE* LPCKBDCDEVICE;

extern const KBDCDEVICE g_KnownDevices[];
extern const USHORT KBDC_UNKNOWN_DEVICE_INDEX;

USHORT Kbdc_GetKnownDeviceIndex(IN PDEVICE_OBJECT PDO);
VOID Kbdc_QueuePackets(IN PDEVICE_EXTENSION FDOExt, IN PKEYBOARD_INPUT_DATA InputDataStart, IN PKEYBOARD_INPUT_DATA InputDataEnd);

#endif  /* _KBDCDEVS_H_ */
