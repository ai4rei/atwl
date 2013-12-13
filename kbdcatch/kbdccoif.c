#include "kbdcatch.h"
#include "kbdcdevs.h"
#include "kbdccoif.h"

NTSTATUS
KbdcCoif_CreateClose (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
{
    NTSTATUS Status = Irp->IoStatus.Status;
    PCOIF_EXTENSION Ext = (PCOIF_EXTENSION)DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION Isl = IoGetCurrentIrpStackLocation(Irp);

    DBGENTER(KbdcCoif_CreateClose);

    switch(Isl->MajorFunction)
    {
        case IRP_MJ_CREATE:
            if(Ext->CreateCount>0)
            {
                Status = STATUS_INVALID_DEVICE_STATE;
                break;
            }
            else if(KeReadStateEvent(&Ext->DeletedEvent))
            {
                Status = STATUS_DEVICE_REMOVED;
                break;
            }

            InterlockedIncrement(&Ext->CreateCount);

            break;
        case IRP_MJ_CLOSE:
            if(Ext->CreateCount>0)
            {
                InterlockedDecrement(&Ext->CreateCount);

                break;
            }

            Status = STATUS_INVALID_DEVICE_STATE;
            break;
    }

    Irp->IoStatus.Status = Status;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    DBGLEAVE(KbdcCoif_CreateClose);

    return Status;
}
//STATUS_INVALID_DEVICE_REQUEST