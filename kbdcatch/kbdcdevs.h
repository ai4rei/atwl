/* -----------------------------------------------------------------
// List of Known Devices
//
// ---------------------------------------------------------------*/

#ifndef _KBDCDEVS_H_
#define _KBDCDEVS_H_

typedef struct KBDCDATA
{
    USHORT MakeCode;
    KBDCDEVICETYPE SourceType;
}
KBDCDATA,* LPKBDCDATA;
typedef const KBDCDATA* LPCKBDCDATA;

typedef enum KBDCDEVICETYPE
{
    KBDC_DEVTYPE_BARCODE,
    KBDC_DEVTYPE_RFID,
}
KBDCDEVICETYPE;

typedef struct KBDCDEVICE
{
    LPCWSTR         HardwareId;
    LPCSTR          FriendlyName;
    KBDCDEVICETYPE  DeviceType;
}
KBDCDEVICE,* LPKBDCDEVICE;
typedef const KBDCDEVICE* LPCKBDCDEVICE;

extern const KBDCDEVICE g_KnownDevices[];
extern const ULONG KBDC_UNKNOWN_DEVICE_INDEX;

ULONG Kbdc_GetKnownDeviceIndex(IN PDEVICE_OBJECT PDO);
VOID Kbdc_QueuePackets(IN PDEVICE_EXTENSION FDOExt, IN PKEYBOARD_INPUT_DATA InputDataStart, IN PKEYBOARD_INPUT_DATA InputDataEnd);

#endif  /* _KBDCDEVS_H_ */
