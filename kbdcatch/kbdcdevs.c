/* -----------------------------------------------------------------
// List of Known Devices
//
// ---------------------------------------------------------------*/

#include "kbdcatch.h"
#include "kbdcdevs.h"
#include "kbdctype.h"

const KBDCDEVICE g_KnownDevices[] =
{
    { L"HID\\VID_065A&PID_0001&REV_0900", "Opticon OPR-2001-UB/NLV-1001-U/OPN-2001", KBDC_DEVTYPE_BARCODE },
    { L"HID\\VID_065A&PID_0001&REV_0100", "Opticon OPR-2001-UB (2015)", KBDC_DEVTYPE_BARCODE },
    { L"HID\\VID_08FF&PID_0009&REV_0008", "AuthenTec USB Reader", KBDC_DEVTYPE_RFID },
    { L"HID\\VID_04D9&PID_1702&REV_0101&MI_00", "Evolve KeyLight LK-606", KBDC_DEVTYPE_UNKNOWN },  /* actually, it's a keyboard */
    { L"HID\\VID_04D9&PID_1505&REV_0300&COL01", "Genius Numpad i110", KBDC_DEVTYPE_NUMPAD },
};
const USHORT KBDC_UNKNOWN_DEVICE_INDEX = _ARRAYSIZE(g_KnownDevices);

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, Kbdc_GetKnownDeviceIndex)
#endif

USHORT Kbdc_GetKnownDeviceIndex(IN PDEVICE_OBJECT PDO)
{/* PASSIVE_LEVEL */
    USHORT Index = KBDC_UNKNOWN_DEVICE_INDEX;

    PAGED_CODE();

    DBGENTER(Kbdc_GetKnownDeviceIndex);
    {
        LPWSTR HardwareId = NULL;
        ULONG ulSize = 0;

        for(;;)
        {
            NTSTATUS Status;

            ulSize+= 1024;

            if(HardwareId)
            {
                ExFreePool(HardwareId);
                HardwareId = NULL;
            }

            if((HardwareId = ExAllocatePool(PagedPool, ulSize))==NULL)
            {
                DBGERROR(ExAllocatePoolWithTag, Kbdc_GetKnownDeviceIndex, STATUS_INSUFFICIENT_RESOURCES);
                break;
            }

            Status = IoGetDeviceProperty(PDO, DevicePropertyHardwareID, ulSize, HardwareId, &ulSize);

            if(NT_SUCCESS(Status))
            {
                break;
            }

            DBGERROR(IoGetDeviceProperty, Kbdc_GetKnownDeviceIndex, Status);
        }

        if(HardwareId)
        {
            UNICODE_STRING HwId;
            UNICODE_STRING HwIdIdx;

            RtlInitUnicodeString(&HwId, HardwareId);

            for(Index = 0; Index<_ARRAYSIZE(g_KnownDevices); Index++)
            {
                RtlInitUnicodeString(&HwIdIdx, g_KnownDevices[Index].HardwareId);

                if(RtlCompareUnicodeString(&HwId, &HwIdIdx, TRUE)==0)
                {
                    DbgPrint("Identified device '%s'\n", g_KnownDevices[Index].FriendlyName);
                    break;
                }
            }
            /*
                if no matching device is found, index ends up as
                KBDC_UNKNOWN_DEVICE_INDEX
            */

            ExFreePool(HardwareId);
            HardwareId = NULL;
        }
    }
    DBGLEAVE(Kbdc_GetKnownDeviceIndex);

    return Index;
}
