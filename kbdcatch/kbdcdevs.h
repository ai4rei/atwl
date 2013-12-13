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

typedef struct KBDCDEVICE
{
    LPCWSTR         HardwareId;
    LPCSTR          FriendlyName;
    KBDCDEVICETYPE  nDeviceType;
}
KBDCDEVICE,* LPKBDCDEVICE;
typedef const KBDCDEVICE* LPCKBDCDEVICE;

extern const KBDCDEVICE g_KnownDevices[];
extern const ULONG KBDC_UNKNOWN_DEVICE_INDEX;

ULONG Kbdc_GetKnownDeviceIndex(IN PDEVICE_OBJECT PDO);

#endif  /* _KBDCDEVS_H_ */
