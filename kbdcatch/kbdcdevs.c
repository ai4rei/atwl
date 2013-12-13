/* -----------------------------------------------------------------
// List of Known Devices
//
// ---------------------------------------------------------------*/

#include "kbdcatch.h"
#include "kbdcdevs.h"

const KBDCDEVICE g_KnownDevices[] =
{
    { L"HID\\VID_065A&PID_0001&REV_0900", "Opticon OPR-2001-UB", KBDC_DEVTYPE_BARCODE },
    { L"HID\\VID_08FF&PID_0009&REV_0008", "AuthenTec USB Reader", KBDC_DEVTYPE_RFID },
};
const ULONG KBDC_UNKNOWN_DEVICE_INDEX = _ARRAYSIZE(g_KnownDevices);

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, Kbdc_GetKnownDeviceIndex)
#endif

ULONG Kbdc_GetKnownDeviceIndex(IN PDEVICE_OBJECT PDO)
{
    ULONG Index = KBDC_UNKNOWN_DEVICE_INDEX;

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
            /* if no matching device is found, index ends up as KBDC_UNKNOWN_DEVICE_INDEX */

            ExFreePool(HardwareId);
            HardwareId = NULL;
        }
    }
    DBGLEAVE(Kbdc_GetKnownDeviceIndex);

    return Index;
}
