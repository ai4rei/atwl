/* -----------------------------------------------------------------
// Output Device
//
// ---------------------------------------------------------------*/

#ifndef _KBDCODEV_H_
#define _KBDCODEV_H_

VOID Kbdc_CreateOutputDevice(IN PDRIVER_OBJECT DriverObject);
VOID Kbdc_DestroyOutputDevice(IN PDRIVER_OBJECT DriverObject);
BOOLEAN Kbdc_IsOutputDeviceIrp(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, OUT PNTSTATUS Status);

#endif  /* _KBDCODEV_H_ */
