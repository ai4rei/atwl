/* -----------------------------------------------------------------
// Common Data Types
//
// ---------------------------------------------------------------*/

#ifndef _KBDCTYPE_H_
#define _KBDCTYPE_H_

typedef enum _KBDCDEVICETYPE
{
    KBDC_DEVTYPE_UNKNOWN, /* debug and devices that do not fit */
    KBDC_DEVTYPE_BARCODE,
    KBDC_DEVTYPE_RFID,
}
KBDCDEVICETYPE;

typedef struct _KBDCINPUTDATA
{
    USHORT DeviceType;  /* KBDCDEVICETYPE */
    USHORT MakeCode;    /* KEYBOARD_INPUT_DATA.MakeCode */
    USHORT Flags;       /* KEYBOARD_INPUT_DATA.Flags */
    USHORT Reserved;    /* padding */
}
KBDCINPUTDATA,* PKBDCINPUTDATA;

#endif  /* _KBDCTYPE_H_ */
