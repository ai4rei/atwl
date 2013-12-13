#ifndef _KBDCCOIF_H_
#define _KBDCCOIF_H_

typedef struct _COIF_EXTENSION
{
    //
    // Whether or not there currently is a client available
    //
    ULONG CreateCount;

    //
    // Whether or not the object is pending deletion
    //
    KEVENT DeletedEvent;

    KEVENT DataClearEvent;
    KEVENT DataAvailEvent;
    KBDCDATA Data;

} COIF_EXTENSION, *PCOIF_EXTENSION;

NTSTATUS
KbdcCoif_CreateClose (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    );

#endif  /* _KBDCCOIF_H_ */
