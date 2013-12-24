/* -----------------------------------------------------------------
// Output Device
//
// ---------------------------------------------------------------*/

#include "kbdcatch.h"
#include "kbdcodev.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, Kbdc_CreateOutputDevice)
#pragma alloc_text (PAGE, Kbdc_DestroyOutputDevice)
#pragma alloc_text (PAGE, Kbdc_IsOutputDeviceIrp)
#endif

typedef struct _OUTPUT_DEVICE_EXTENSION
{
    int Unused;
}
OUTPUT_DEVICE_EXTENSION,* POUTPUT_DEVICE_EXTENSION;

/*
    NOTE: This device is considered OPTIONAL. Even though it is
    actually necessary for the purpose of the filter driver, we
    cannot fail the driver if the device fails to create, otherwise
    we render all keyboards in the system unusable. Thus every code
    part that uses the output device must check, whether or not it
    actually exists.
*/
PDEVICE_OBJECT l_OutputDevice = NULL;

VOID Kbdc_CreateOutputDevice(IN PDRIVER_OBJECT DriverObject)
{/* PASSIVE_LEVEL */
    DBGENTER(Kbdc_CreateOutputDevice);
    {
        NTSTATUS Status;
        PDEVICE_OBJECT OutputDevice;
        POUTPUT_DEVICE_EXTENSION OutDevExt;
        UNICODE_STRING DeviceName;
        UNICODE_STRING DevSymLink;

        RtlInitUnicodeString(&DeviceName, KBDC_DEVNAME);
        RtlInitUnicodeString(&DevSymLink, KBDC_SYMLINK);

        Status = IoCreateDevice(DriverObject, sizeof(OutDevExt[0]), &DeviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN|FILE_READ_ONLY_DEVICE, TRUE, &OutputDevice);

        if(NT_SUCCESS(Status))
        {
            Status = IoCreateSymbolicLink(&DevSymLink, &DeviceName);

            if(NT_SUCCESS(Status))
            {
                OutDevExt = (POUTPUT_DEVICE_EXTENSION)OutputDevice->DeviceExtension;

                OutputDevice->Flags|= DO_BUFFERED_IO|DO_POWER_PAGABLE;
                OutputDevice->Flags&=~DO_DEVICE_INITIALIZING;

                /* the device is ready */
                l_OutputDevice = OutputDevice;
                return;
            }

            IoDeleteDevice(OutputDevice);
            OutputDevice = NULL;
        }
    }
    DBGLEAVE(Kbdc_CreateOutputDevice);
}

VOID Kbdc_DestroyOutputDevice(IN PDRIVER_OBJECT DriverObject)
{/* PASSIVE_LEVEL */
    PAGED_CODE();

    DBGENTER(Kbdc_DestroyOutputDevice);
    {
        if(l_OutputDevice)
        {
            PDEVICE_OBJECT OutputDevice = l_OutputDevice;
            UNICODE_STRING DevSymLink;

            /*
                disable further access to the output device
            */
            l_OutputDevice = NULL;

            /* TODO: Outstanding Irps? */

            RtlInitUnicodeString(&DevSymLink, KBDC_SYMLINK);

            IoDeleteSymbolicLink(&DevSymLink);
            IoDeleteDevice(OutputDevice);
        }
    }
    DBGLEAVE(Kbdc_DestroyOutputDevice);
}

BOOLEAN Kbdc_IsOutputDeviceIrp(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, OUT PNTSTATUS Status)
{/* DISPATCH_LEVEL */
    BOOLEAN IrpHandled = FALSE;

    PAGED_CODE();

    DBGENTER(Kbdc_IsOutputDeviceIrp);
    {
        if(l_OutputDevice && DeviceObject==l_OutputDevice)
        {
            PIO_STACK_LOCATION Isl = IoGetCurrentIrpStackLocation(Irp);

            switch(Isl->MajorFunction)
            {
                default:
                    Status[0] = Irp->IoStatus.Status;
                    IoCompleteRequest(Irp, IO_NO_INCREMENT);
            }

            /*
                target was the output device
            */
            IrpHandled = TRUE;
        }
    }
    DBGLEAVE(Kbdc_IsOutputDeviceIrp);

    return IrpHandled;
}
