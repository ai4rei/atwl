/* -----------------------------------------------------------------
// List of Known Devices
//
// ---------------------------------------------------------------*/

#ifndef _KBDCDEVS_H_
#define _KBDCDEVS_H_

typedef enum KBDCDEVICETYPE
{
    KBDC_DEVTYPE_UNKNOWN, /* debug and devices that do not fit */
    KBDC_DEVTYPE_BARCODE,
    KBDC_DEVTYPE_RFID,
}
KBDCDEVICETYPE;

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

#endif  /* _KBDCDEVS_H_ */
