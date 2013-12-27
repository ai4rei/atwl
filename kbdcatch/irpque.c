/*
    Based on: Sample WDM IRP-handling code presented in MSJ Jan '99
              issue by Ervin Peretz.
*/

#include <wdm.h>

#include "irpque.h"

PIRPQUEUE DriverDeviceIrpQueue(IN PDEVICE_OBJECT DeviceObject);

static VOID IrpQueueCancelRoutine(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
    BOOLEAN bComplete = FALSE;
    KIRQL PrevIrql;
    PIRP FirstIrp = NULL, ThisIrp;
    /*
        The driver has to provide this routine, since we have no
        idea where in the DeviceExtension the IrpQueue might be
        located.
    */
    PIRPQUEUE IrpQueue = DriverDeviceIrpQueue(DeviceObject);

    /*
        In this implementation, I don't assume that the IRP being
        cancelled is in the queue; I only complete the IRP if it IS
        in the queue.
    */
    KeAcquireSpinLock(&IrpQueue->SpinLock, &PrevIrql);

    while( !IsListEmpty(&IrpQueue->ListHead) )
    {
        PLIST_ENTRY ListEntry = RemoveHeadList(&IrpQueue->ListHead);

        ThisIrp = CONTAINING_RECORD(ListEntry, IRP, Tail.Overlay.ListEntry);

        if( ThisIrp==Irp )
        {
            /*
                This is the IRP being cancelled; we'll complete it
                after we release the spinlock.
            */
            ASSERT(ThisIrp->Cancel);

            bComplete = TRUE;

            /*
                Keep looping so that order of the remaining IRPs is
                preserved. (Don't drop the spinlock and complete the
                irp here because the structure of the list could
                change in the meantime; if the IRP pointed to by
                FirstIrp were to be dequeued while we completed this
                IRP, for example, we might loop forever).
            */
        }
        else
        {
            /*
                This is not the IRP being cancelled, so put it back.
            */
            if( ThisIrp==FirstIrp )
            {/* finished going through the list */
                InsertHeadList(&IrpQueue->ListHead, ListEntry);
                break;
            }
            else
            {
                InsertTailList(&IrpQueue->ListHead, ListEntry);

                if( FirstIrp==NULL )
                {
                    FirstIrp = ThisIrp;
                }
            }
        }
    }

    KeReleaseSpinLock(&IrpQueue->SpinLock, PrevIrql);

    /*
        Finally, release the global cancel spinlock whether or not
        we are completing this IRP.
        Do this after releasing the local spinlock so that we exit
        the cancel routine at IRQL equal to Irp->CancelIrql.
    */
    IoReleaseCancelSpinLock(Irp->CancelIrql);

    if( bComplete )
    {
        /*
            complete this cancelled IRP only if it was in the list
        */
        Irp->IoStatus.Status = STATUS_CANCELLED;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }
}

PIRP IrpQueueDequeue(PIRPQUEUE IrpQueue)
{
    KIRQL PrevIrql;
    PIRP NextIrp = NULL;

    KeAcquireSpinLock(&IrpQueue->SpinLock, &PrevIrql);

    while( NextIrp==NULL && !IsListEmpty(&IrpQueue->ListHead) )
    {
        PLIST_ENTRY ListEntry = RemoveHeadList(&IrpQueue->ListHead);

        NextIrp = CONTAINING_RECORD(ListEntry, IRP, Tail.Overlay.ListEntry);

        /*
            clear the cancel routine while holding the spinlock
        */
        IoSetCancelRoutine(NextIrp, NULL);

        if( NextIrp->Cancel )
        {
            /*
                This IRP was just cancelled. The cancel routine may
                or may not have been called, but it doesn't matter
                because it will not find the IRP in the list.
                Must release the spinlock when calling outside the
                driver to complete the IRP.
            */
            KeReleaseSpinLock(&IrpQueue->SpinLock, PrevIrql);

            NextIrp->IoStatus.Status = STATUS_CANCELLED;
            IoCompleteRequest(NextIrp, IO_NO_INCREMENT);

            KeAcquireSpinLock(&IrpQueue->SpinLock, &PrevIrql);

            /*
                try next one, if any
            */
            NextIrp = NULL;
        }
    }

    KeReleaseSpinLock(&IrpQueue->SpinLock, PrevIrql);

    return NextIrp;
}

NTSTATUS IrpQueueEnqueue(PIRPQUEUE IrpQueue, PIRP Irp)
{
    KIRQL PrevIrql;
    NTSTATUS Status;
    PDRIVER_CANCEL PrevCancelRoutine;

    KeAcquireSpinLock(&IrpQueue->SpinLock, &PrevIrql);

    /*
        must set a cancel routine before checking the Cancel flag
    */
    PrevCancelRoutine = IoSetCancelRoutine(Irp, &IrpQueueCancelRoutine);
    ASSERT(PrevCancelRoutine==NULL);

    if( Irp->Cancel )
    {
        /*
            This IRP has already been cancelled, so complete it now.
            We must clear the cancel routine before completing the
            IRP. We must release the spinlock before calling out of
            the driver.
        */
        IoSetCancelRoutine(Irp, NULL);

        KeReleaseSpinLock(&IrpQueue->SpinLock, PrevIrql);

        Status = Irp->IoStatus.Status = STATUS_CANCELLED;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }
    else
    {
        /*
            This macro sets a bit in the current stack location to
            indicate that the IRP may complete on a different
            thread.
        */
        IoMarkIrpPending(Irp);

        InsertTailList(&IrpQueue->ListHead, &Irp->Tail.Overlay.ListEntry);

        KeReleaseSpinLock(&IrpQueue->SpinLock, PrevIrql);

        Status = STATUS_SUCCESS;
    }

    return Status;
}

VOID IrpQueueInit(PIRPQUEUE IrpQueue)
{
    InitializeListHead(&IrpQueue->ListHead);
    KeInitializeSpinLock(&IrpQueue->SpinLock);
}
