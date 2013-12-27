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
    /*
        back-pointer to DO
    */
    PDEVICE_OBJECT Self;

    /*
        queue for read irps on output device
    */
    IRPQUEUE IrpQueue;

    /*
        buffer for keyboard packets
    */
    KEYBOARD_INPUT_DATA Kid[KBDC_BUFSIZE];

    /*
        entries currently in buffer
    */
    ULONG KidCount;

    /*
        spin lock for protecting Kid[]
    */
    KSPIN_LOCK KidSpinLock;

    /*
        reference count [0-1]
    */
    ULONG CreateCount;
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

PIRPQUEUE DriverDeviceIrpQueue(IN PDEVICE_OBJECT DeviceObject)
{/* DISPATCH_LEVEL */
    PIRPQUEUE IrpQueue = NULL;

    DBGENTER(DriverDeviceIrpQueue);
    {
        if(l_OutputDevice && DeviceObject==l_OutputDevice)
        {
            POUTPUT_DEVICE_EXTENSION OutDevExt = (POUTPUT_DEVICE_EXTENSION)DeviceObject->DeviceExtension;

            IrpQueue = &OutDevExt->IrpQueue;
        }
    }
    DBGLEAVE(DriverDeviceIrpQueue);

    return IrpQueue;
}

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
                OutDevExt->Self = OutputDevice;
                IrpQueueInit(&OutDevExt->IrpQueue);
                KeInitializeSpinLock(&OutDevExt->KidSpinLock);

                OutputDevice->Flags|= DO_BUFFERED_IO|DO_POWER_PAGABLE;
                OutputDevice->Flags&=~DO_DEVICE_INITIALIZING;

                /* the device is ready */
                l_OutputDevice = OutputDevice;
                return;
            }
            else
            {
                DBGERROR(IoCreateSymbolicLink, Kbdc_CreateOutputDevice, Status);
            }

            IoDeleteDevice(OutputDevice);
            OutputDevice = NULL;
        }
        else
        {
            DBGERROR(IoCreateDevice, Kbdc_CreateOutputDevice, Status);
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
            POUTPUT_DEVICE_EXTENSION OutDevExt = (POUTPUT_DEVICE_EXTENSION)OutputDevice->DeviceExtension;
            UNICODE_STRING DevSymLink;

            /*
                disable further access to the output device
            */
            l_OutputDevice = NULL;

            /*
                cancel pending irps
            */
            for(;;)
            {
                PIRP ReadIrp = IrpQueueDequeue(&OutDevExt->IrpQueue);

                if(!ReadIrp)
                {
                    break;
                }

                ReadIrp->IoStatus.Status = STATUS_CANCELLED;
                ReadIrp->IoStatus.Information = 0;
                IoCompleteRequest(ReadIrp, IO_NO_INCREMENT);
            }

            RtlInitUnicodeString(&DevSymLink, KBDC_SYMLINK);

            IoDeleteSymbolicLink(&DevSymLink);
            IoDeleteDevice(OutputDevice);
        }
    }
    DBGLEAVE(Kbdc_DestroyOutputDevice);
}

static NTSTATUS Kbdc_P_OutputDeviceDispatchCreate(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    DBGENTER(Kbdc_P_OutputDeviceDispatchCreate);
    {
        PIO_STACK_LOCATION Isl = IoGetCurrentIrpStackLocation(Irp);
        POUTPUT_DEVICE_EXTENSION OutDevExt = (POUTPUT_DEVICE_EXTENSION)DeviceObject->DeviceExtension;

        if(InterlockedIncrement(&OutDevExt->CreateCount)!=1)
        {/* exclusive use */
            TRAP();
            InterlockedDecrement(&OutDevExt->CreateCount);
            Status = STATUS_INVALID_DEVICE_STATE;
        }
        else
        {
            Status = STATUS_SUCCESS;
        }

        Irp->IoStatus.Status = Status;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }
    DBGLEAVE(Kbdc_P_OutputDeviceDispatchCreate);

    return Status;
}

static NTSTATUS Kbdc_P_OutputDeviceDispatchClose(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    DBGENTER(Kbdc_P_OutputDeviceDispatchClose);
    {
        KIRQL PrevIrql;
        POUTPUT_DEVICE_EXTENSION OutDevExt = (POUTPUT_DEVICE_EXTENSION)DeviceObject->DeviceExtension;

        InterlockedDecrement(&OutDevExt->CreateCount);

        /*
            cancel pending irps (though there should not be any)
        */
        for(;;)
        {
            PIRP ReadIrp = IrpQueueDequeue(&OutDevExt->IrpQueue);

            if(!ReadIrp)
            {
                break;
            }

            ReadIrp->IoStatus.Status = STATUS_CANCELLED;
            ReadIrp->IoStatus.Information = 0;
            IoCompleteRequest(ReadIrp, IO_NO_INCREMENT);
        }

        KeAcquireSpinLock(&OutDevExt->KidSpinLock, &PrevIrql);

            /*
                reset the queue
            */
            OutDevExt->KidCount = 0;

        KeReleaseSpinLock(&OutDevExt->KidSpinLock, PrevIrql);

        Status = STATUS_SUCCESS;

        Irp->IoStatus.Status = Status;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }
    DBGLEAVE(Kbdc_P_OutputDeviceDispatchClose);

    return Status;
}

static NTSTATUS Kbdc_P_OutputDeviceDispatchRead(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    DBGENTER(Kbdc_P_OutputDeviceDispatchRead);
    {
        KIRQL PrevIrql;
        PIO_STACK_LOCATION Isl = IoGetCurrentIrpStackLocation(Irp);
        POUTPUT_DEVICE_EXTENSION OutDevExt = (POUTPUT_DEVICE_EXTENSION)DeviceObject->DeviceExtension;
        ULONG ReadCount = 0, ReadAvail = Isl->Parameters.Read.Length/sizeof(OutDevExt->Kid[0]);

        if(!Isl->Parameters.Read.Length)
        {/* there is no desire to read anything */
            Status = STATUS_SUCCESS;
        }
        else
        {
            KeAcquireSpinLock(&OutDevExt->KidSpinLock, &PrevIrql);

                for(;;)
                {
                    ReadCount = OutDevExt->KidCount;

                    if(!ReadCount)
                    {/* nothing to offer, queue */
                        Status = STATUS_PENDING;
                        break;
                    }

                    ReadCount = ReadCount>ReadAvail ? ReadAvail : ReadCount;

                    if(!ReadCount)
                    {/* too small buffer */
                        Status = STATUS_BUFFER_OVERFLOW;
                        break;
                    }

                    /*
                        copy data to user-mode
                    */
                    RtlCopyMemory(Irp->AssociatedIrp.SystemBuffer, &OutDevExt->Kid[0], ReadCount*sizeof(OutDevExt->Kid[0]));

                    /*
                        shift remaining entries
                    */
                    if(ReadCount<OutDevExt->KidCount)
                    {
                        RtlCopyMemory(&OutDevExt->Kid[0], &OutDevExt->Kid[ReadCount], (OutDevExt->KidCount-ReadCount)*sizeof(OutDevExt->Kid[0]));
                    }

                    OutDevExt->KidCount-= ReadCount;
                    Status = STATUS_SUCCESS;
                    break;
                }

            KeReleaseSpinLock(&OutDevExt->KidSpinLock, PrevIrql);
        }

        if(Status==STATUS_PENDING)
        {
            Status = IrpQueueEnqueue(&OutDevExt->IrpQueue, Irp);

            if(NT_SUCCESS(Status))
            {/* queued */
                return STATUS_PENDING;
            }
        }
        else
        {
            Irp->IoStatus.Status = Status;
            Irp->IoStatus.Information = ReadCount*sizeof(OutDevExt->Kid[0]);
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
        }
    }
    DBGLEAVE(Kbdc_P_OutputDeviceDispatchRead);

    return Status;
}

BOOLEAN Kbdc_IsOutputDeviceIrp(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, OUT PNTSTATUS Status)
{/* PASSIVE_LEVEL~DISPATCH_LEVEL */
    BOOLEAN IrpHandled = FALSE;

    PAGED_CODE();

    DBGENTER(Kbdc_IsOutputDeviceIrp);
    {
        if(l_OutputDevice && DeviceObject==l_OutputDevice)
        {
            PIO_STACK_LOCATION Isl = IoGetCurrentIrpStackLocation(Irp);

            switch(Isl->MajorFunction)
            {
                case IRP_MJ_CREATE:
                    Status[0] = Kbdc_P_OutputDeviceDispatchCreate(DeviceObject, Irp);
                    break;
                case IRP_MJ_CLOSE:
                    Status[0] = Kbdc_P_OutputDeviceDispatchClose(DeviceObject, Irp);
                    break;
                case IRP_MJ_READ:
                    Status[0] = Kbdc_P_OutputDeviceDispatchRead(DeviceObject, Irp);
                    break;
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

VOID Kbdc_QueuePackets(IN PDEVICE_EXTENSION FDOExt, IN PKEYBOARD_INPUT_DATA InputDataStart, IN PKEYBOARD_INPUT_DATA InputDataEnd)
{/* DISPATCH_LEVEL */
    DBGENTER(Kbdc_QueuePackets);
    {
        if(l_OutputDevice)
        {
            KIRQL PrevIrql;
            PKEYBOARD_INPUT_DATA InputData;
            POUTPUT_DEVICE_EXTENSION OutDevExt = (POUTPUT_DEVICE_EXTENSION)l_OutputDevice->DeviceExtension;

            /*
                bother with it only when someone listens
            */
            if(OutDevExt->CreateCount)
            {
                for(InputData = InputDataStart; InputData!=InputDataEnd; InputData++)
                {
                    KeAcquireSpinLock(&OutDevExt->KidSpinLock, &PrevIrql);

                        if(OutDevExt->KidCount>=_ARRAYSIZE(OutDevExt->Kid))
                        {
                            /*
                                drop oldest entry
                            */
                            RtlMoveMemory(&OutDevExt->Kid[0], &OutDevExt->Kid[1], (_ARRAYSIZE(OutDevExt->Kid)-1)*sizeof(OutDevExt->Kid[0]));
                            OutDevExt->KidCount--;
                        }

                        /*
                            enqueue
                        */
                        RtlCopyMemory(&OutDevExt->Kid[OutDevExt->KidCount], InputData, sizeof(OutDevExt->Kid[0]));
                        OutDevExt->KidCount++;

                    KeReleaseSpinLock(&OutDevExt->KidSpinLock, PrevIrql);
                }

                /*
                    process any queued irps
                */
                for(;;)
                {
                    PIRP ReadIrp = IrpQueueDequeue(&OutDevExt->IrpQueue);

                    if(!ReadIrp)
                    {
                        break;
                    }

                    Kbdc_P_OutputDeviceDispatchRead(l_OutputDevice, ReadIrp);

                    if(!OutDevExt->KidCount)
                    {
                        break;
                    }
                }
            }
        }
    }
    DBGLEAVE(Kbdc_QueuePackets);
}
