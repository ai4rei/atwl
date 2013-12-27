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

typedef enum _KBDCINPUTDATAFLAGS
{
    KIDF_BREAK           = 0x01,
    KIDF_E0              = 0x02,
    KIDF_E1              = 0x04,
    KIDF_TERMSRV_SET_LED = 0x08,
    KIDF_TERMSRV_SHADOW  = 0x10,
}
KBDCINPUTDATAFLAGS;

typedef struct _KBDCINPUTDATA
{
    USHORT DeviceType;  /* KBDCDEVICETYPE */
    USHORT MakeCode;    /* KEYBOARD_INPUT_DATA.MakeCode */
    USHORT Flags;       /* KEYBOARD_INPUT_DATA.Flags */
    USHORT Reserved;    /* padding */
}
KBDCINPUTDATA,* PKBDCINPUTDATA;

#endif  /* _KBDCTYPE_H_ */
