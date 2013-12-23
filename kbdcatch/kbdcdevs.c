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
const USHORT KBDC_UNKNOWN_DEVICE_INDEX = _ARRAYSIZE(g_KnownDevices);

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, Kbdc_GetKnownDeviceIndex)
#endif

USHORT Kbdc_GetKnownDeviceIndex(IN PDEVICE_OBJECT PDO)
{
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

VOID Kbdc_QueuePackets(IN PDEVICE_EXTENSION FDOExt, IN PKEYBOARD_INPUT_DATA InputDataStart, IN PKEYBOARD_INPUT_DATA InputDataEnd)
{
    PKEYBOARD_INPUT_DATA InputData;

    for(InputData = InputDataStart; InputData!=InputDataEnd; InputData++)
    {
        /*
            consider only releases in the assumption that keyboard
            emulating devices do not use the repeat feature
        */
        if(InputData->Flags&KEY_BREAK)
        {
            /*
                TODO: Queue key presses into local buffer and look
                for an Irp in the IrpQueue to feed it with once the
                ENTER key is pressed
            */
        }
    }
}
