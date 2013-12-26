/* -----------------------------------------------------------------
// Output Device
//
// ---------------------------------------------------------------*/

#ifndef _KBDCODEV_H_
#define _KBDCODEV_H_

VOID Kbdc_CreateOutputDevice(IN PDRIVER_OBJECT DriverObject);
VOID Kbdc_DestroyOutputDevice(IN PDRIVER_OBJECT DriverObject);
BOOLEAN Kbdc_IsOutputDeviceIrp(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, OUT PNTSTATUS Status);
VOID Kbdc_QueuePackets(IN PDEVICE_EXTENSION FDOExt, IN PKEYBOARD_INPUT_DATA InputDataStart, IN PKEYBOARD_INPUT_DATA InputDataEnd);

#endif  /* _KBDCODEV_H_ */
