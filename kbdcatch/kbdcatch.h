/*++
Copyright (c) 1997  Microsoft Corporation

Module Name:

    kbfilter.h

Abstract:

    This module contains the common private declarations for the keyboard
    packet filter

Environment:

    kernel mode only

Notes:


Revision History:


--*/

#ifndef KBFILTER_H
#define KBFILTER_H

#include "ntddk.h"
#include "kbdmou.h"
#include <ntddkbd.h>
#include <ntdd8042.h>

#define KBDC_SYMLINK L"\\DosDevices\\KbdCatch"
#define KBDC_DEVNAME L"\\Device\\KbdCatch"

#define KBFILTER_POOL_TAG (ULONG) 'CdbK'
#undef ExAllocatePool
#define ExAllocatePool(type, size) \
            ExAllocatePoolWithTag (type, size, KBFILTER_POOL_TAG)
#undef ExFreePool
#define ExFreePool(ptr) \
            ExFreePoolWithTag(ptr, KBFILTER_POOL_TAG)

#if DBG

#define TRAP()                      DbgBreakPoint()
#define DbgRaiseIrql(_x_,_y_)       KeRaiseIrql(_x_,_y_)
#define DbgLowerIrql(_x_)           KeLowerIrql(_x_)

#define DebugPrint(_x_) DbgPrint _x_

#define DBGIDENT "KbdC"
#define DBGENTER(_x_) DbgPrint(DBGIDENT": Enter '%s' (0x%p), Level: %u\n", #_x_, &_x_, KeGetCurrentIrql())
#define DBGLEAVE(_x_) DbgPrint(DBGIDENT": Leave '%s' (0x%p), Level: %u\n", #_x_, &_x_, KeGetCurrentIrql())
#define DBGERROR(_x_,_y_,_z_) DbgPrint(DBGIDENT": Call to '%s' (0x%p) failed in '%s' (0x%p) with status %l (0x%08x)\n", #_x_, &_x_, #_y_, &_y_, _z_, _z_)

#else   // DBG

#define TRAP()
#define DbgRaiseIrql(_x_,_y_)
#define DbgLowerIrql(_x_)

#define DebugPrint(_x_)

#define DBGENTER(_x_)
#define DBGLEAVE(_x_)
#define DBGERROR(_x_,_y_,_z_)

#endif

#define MIN(_A_,_B_) (((_A_) < (_B_)) ? (_A_) : (_B_))

#ifndef _ARRAYSIZE
    #define _ARRAYSIZE(_x_) (sizeof(_x_)/sizeof((_x_)[0]))
#endif  /* _ARRAYSIZE */

typedef struct _DEVICE_EXTENSION
{
    //
    // A backpointer to the device object for which this is the extension
    //
    PDEVICE_OBJECT  Self;

    //
    // "THE PDO"  (ejected by the root bus or ACPI)
    //
    PDEVICE_OBJECT  PDO;

    //
    // The top of the stack before this filter was added.  AKA the location
    // to which all IRPS should be directed.
    //
    PDEVICE_OBJECT  TopOfStack;

    //
    // Number of creates sent down
    //
    LONG EnableCount;

    //
    // The real connect data that this driver reports to
    //
    CONNECT_DATA UpperConnectData;

    //
    // Previous initialization and hook routines (and context)
    //
    PVOID UpperContext;
    PI8042_KEYBOARD_INITIALIZATION_ROUTINE UpperInitializationRoutine;
    PI8042_KEYBOARD_ISR UpperIsrHook;

    //
    // Write function from within KbFilter_IsrHook
    //
    IN PI8042_ISR_WRITE_PORT IsrWritePort;

    //
    // Queue the current packet (ie the one passed into KbFilter_IsrHook)
    //
    IN PI8042_QUEUE_PACKET QueueKeyboardPacket;

    //
    // Context for IsrWritePort, QueueKeyboardPacket
    //
    IN PVOID CallContext;

    //
    // current power state of the device
    //
    DEVICE_POWER_STATE  DeviceState;

    BOOLEAN         Started;
    BOOLEAN         SurpriseRemoved;
    BOOLEAN         Removed;

    //
    // identification of the PDO
    //
    USHORT KnownDeviceIndex;

    //
    // communication interface DO
    //
    PDEVICE_OBJECT Coif;

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

//
// OACR/PreFast
//

#ifdef _PREFAST_
DRIVER_INITIALIZE       DriverEntry;
DRIVER_ADD_DEVICE       KbFilter_AddDevice;
DRIVER_DISPATCH         KbFilter_CreateClose;
DRIVER_DISPATCH         KbFilter_DispatchPassThrough;
DRIVER_DISPATCH         KbFilter_InternIoCtl;
//DRIVER_DISPATCH         KbFilter_IoCtl;
DRIVER_DISPATCH         KbFilter_PnP;
DRIVER_DISPATCH         KbFilter_Power;
DRIVER_UNLOAD           KbFilter_Unload;
IO_COMPLETION_ROUTINE   KbFilter_Complete;
#endif  /* _PREFAST_ */

//
// Prototypes
//

NTSTATUS
KbFilter_AddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT BusDeviceObject
    );

NTSTATUS
KbFilter_CreateClose (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
KbFilter_DispatchPassThrough(
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
        );

NTSTATUS
KbFilter_InternIoCtl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
KbFilter_IoCtl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
KbFilter_PnP (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
KbFilter_Power (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
KbFilter_InitializationRoutine(
    IN PDEVICE_OBJECT                 DeviceObject,    // InitializationContext
    IN PVOID                           SynchFuncContext,
    IN PI8042_SYNCH_READ_PORT          ReadPort,
    IN PI8042_SYNCH_WRITE_PORT         WritePort,
    OUT PBOOLEAN                       TurnTranslationOn
    );

BOOLEAN
KbFilter_IsrHook(
    PDEVICE_OBJECT         DeviceObject,               // IsrContext
    PKEYBOARD_INPUT_DATA   CurrentInput,
    POUTPUT_PACKET         CurrentOutput,
    UCHAR                  StatusByte,
    PUCHAR                 DataByte,
    PBOOLEAN               ContinueProcessing,
    PKEYBOARD_SCAN_STATE   ScanState
    );

VOID
KbFilter_ServiceCallback(
    IN PDEVICE_OBJECT DeviceObject,
    IN PKEYBOARD_INPUT_DATA InputDataStart,
    IN PKEYBOARD_INPUT_DATA InputDataEnd,
    IN OUT PULONG InputDataConsumed
    );

VOID
KbFilter_Unload (
    IN PDRIVER_OBJECT DriverObject
    );

#endif  // KBFILTER_H
