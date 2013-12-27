/*
    Based on: Sample WDM IRP-handling code presented in MSJ Jan '99
              issue by Ervin Peretz.
*/
#ifndef _IRPQUE_H_
#define _IRPQUE_H_

/*
    This structure should be part of the DO's DeviceExtension.
*/
typedef struct _IRPQUEUE IRPQUEUE;
typedef IRPQUEUE* PIRPQUEUE;

struct _IRPQUEUE
{
    LIST_ENTRY ListHead;
    KSPIN_LOCK SpinLock;
};

PIRP IrpQueueDequeue(PIRPQUEUE IrpQueue);
NTSTATUS IrpQueueEnqueue(PIRPQUEUE IrpQueue, PIRP Irp);
VOID IrpQueueInit(PIRPQUEUE IrpQueue);

#endif  /* _IRPQUE_H_ */
