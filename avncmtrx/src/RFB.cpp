#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <btypes.h>
#include <bvendian.h>

namespace RFB
{
    #define RFB_SENDPACKET(NAME) struct PACKET_##NAME Packet = { MESSAGE_TYPE_##NAME }

    const ubyte_t SECURITY_TYPE_INVALID            = 0U;
    const ubyte_t SECURITY_TYPE_NONE               = 1U;
    const ubyte_t SECURITY_TYPE_VNC_AUTHENTICATION = 2U;

    const ubyte_t POINTER_EVENT_BUTTON_PRIMARY   = 0x01U;
    const ubyte_t POINTER_EVENT_BUTTON_MIDDLE    = 0x02U;
    const ubyte_t POINTER_EVENT_BUTTON_SECONDARY = 0x04U;
    const ubyte_t POINTER_EVENT_BUTTON_WHEELUP   = 0x08U;
    const ubyte_t POINTER_EVENT_BUTTON_WHEELDOWN = 0x10U;
    const ubyte_t POINTER_EVENT_BUTTON_PREVIOUS  = 0x20U;
    const ubyte_t POINTER_EVENT_BUTTON_NEXT      = 0x40U;
    const ubyte_t POINTER_EVENT_BUTTON_8         = 0x80;

    const slong_t ENCODING_TYPE_RAW         = +0;
    const slong_t ENCODING_TYPE_COPYRECT    = +1;
    const slong_t ENCODING_TYPE_RRE         = +2;
    const slong_t ENCODING_TYPE_HEXTILE     = +5;
    const slong_t ENCODING_TYPE_TRLE        = +15;
    const slong_t ENCODING_TYPE_ZRLE        = +16;
    const slong_t ENCODING_TYPE_CURSOR      = -239;
    const slong_t ENCODING_TYPE_DESKTOPSIZE = -223;

    BEGINSTRUCT(PIXELFORMAT)
    {
        ubyte_t ucBitsPerPixel;
        ubyte_t ucBitDepth;
        bool bIsBigEndian;
        bool bIsTrueColor;
        uword_t uwRMax;
        uword_t uwGMax;
        uword_t uwBMax;
        ubyte_t ucRShift;
        ubyte_t ucGShift;
        ubyte_t ucBShift;
    }
    CLOSESTRUCT(PIXELFORMAT);

    BEGINSTRUCT(CLIENTINIT)
    {
        bool bIsShared;
    }
    CLOSESTRUCT(CLIENTINIT);

    BEGINSTRUCT(SERVERINIT)
    {
        uword_t uwWidth;
        uword_t uwHeight;
        PIXELFORMAT PixelFormat;
        char* lpszName;
    }
    CLOSESTRUCT(SERVERINIT);

    class CPacketFactory
    {
    private:
        typedef bool (__WDECL* LPFNPACKETREAD)
        (
            void* lpBuffer,
            size_t uBufferSize,
            void* lpContext
        );
        typedef bool (__WDECL* LPFNPACKETSEND)
        (
            const void* lpData,
            size_t uDataLength,
            void* lpContext
        );
        enum PACKET_LENGTHS
        {
            PROTOCOL_VERSION_LENGTH = 12U,
            VNC_AUTHENTICATION_CHALLENGE_LENGTH = 16U,
            VNC_AUTHENTICATION_RESPONSE_LENGTH = 16U,
        };
#pragma pack(push,1)
        struct PACKET__HEAD
        {
            ubyte_t ucMessageType;
        };
        struct PACKET__PIXELFORMAT
        {
            ubyte_t ucBitsPerPixel;
            ubyte_t ucBitDepth;
            ubyte_t ucBigEndianFlag;
            ubyte_t ucTrueColorFlag;
            uword_t uwRMax;
            uword_t uwGMax;
            uword_t uwBMax;
            ubyte_t ucRShift;
            ubyte_t ucGShift;
            ubyte_t ucBShift;
            ubyte_t ucPadding[3];
        };
        struct PACKET__FRAMEBUFFERUPDATERECTANGLE
        {
            uword_t uwXPosition;
            uword_t uwYPosition;
            uword_t uwWidth;
            uword_t uwHeight;
            slong_t lEncodingType;
        };
        struct PACKET__SETCOLORMAPENTRIESCOLOR
        {
            uword_t uwR;
            uword_t uwG;
            uword_t uwB;
        };
        struct PACKET_CLIENTINIT
        {
            ubyte_t ucSharedFlag;
        };
        struct PACKET_SERVERINIT
        {
            uword_t uwWidth;
            uword_t uwHeight;
            PACKET__PIXELFORMAT PixelFormat;
            ulong_t ulNameLength;
            // ubyte_t ucName[];
        };
        struct PACKET_SETPIXELFORMAT
        {
            ubyte_t ucMessageType;
            ubyte_t ucPadding[3];
            PACKET__PIXELFORMAT PixelFormat;
        };
        struct PACKET_SETENCODINGS
        {
            ubyte_t ucMessageType;
            ubyte_t ucPadding;
            uword_t uwEncodingCount;
            // slong_t lEncodingType[];
        };
        struct PACKET_FRAMEBUFFERUPDATEREQUEST
        {
            ubyte_t ucMessageType;
            ubyte_t ucIncremental;
            uword_t uwXPosition;
            uword_t uwYPosition;
            uword_t uwWidth;
            uword_t uwHeight;
        };
        struct PACKET_KEYEVENT
        {
            ubyte_t ucMessageType;
            ubyte_t ucDownFlag;
            ubyte_t ucPadding[2];
            ulong_t ulKey;
        };
        struct PACKET_POINTEREVENT
        {
            ubyte_t ucMessageType;
            ubyte_t ucButtonMask;  // POINTER_EVENT_BUTTON_*
            uword_t uwXPosition;
            uword_t uwYPosition;
        };
        struct PACKET_CLIENTCUTTEXT
        {
            ubyte_t ucMessageType;
            ubyte_t ucPadding[3];
            ulong_t ulLength;
            // ubyte_t ucText[];
        };
        struct PACKET_FRAMEBUFFERUPDATE
        {
            ubyte_t ucMessageType;
            ubyte_t ucPadding;
            uword_t uwRectCount;
            // PACKET__FRAMEBUFFERUPDATERECTANGLE Rect[];
        };
        struct PACKET_SETCOLORMAPENTRIES
        {
            ubyte_t ucMessageType;
            ubyte_t ucPadding;
            uword_t uwFirstColor;
            uword_t uwColorCount;
            // PACKET__SETCOLORMAPENTRIESCOLOR Color[];
        };
        struct PACKET_BELL
        {
            ubyte_t ucMessageType;
        };
        struct PACKET_SERVERCUTTEXT
        {
            ubyte_t ucMessageType;
            ubyte_t ucPadding[3];
            ulong_t ulLength;
            // ubyte_t ucText[];
        };
#pragma pack(pop)

    private:
        // CS message types
        static const ubyte_t MESSAGE_TYPE_SETPIXELFORMAT = 0U;
        static const ubyte_t MESSAGE_TYPE_SETENCODINGS = 2U;
        static const ubyte_t MESSAGE_TYPE_FRAMEBUFFERUPDATEREQUEST = 3U;
        static const ubyte_t MESSAGE_TYPE_KEYEVENT = 4U;
        static const ubyte_t MESSAGE_TYPE_POINTEREVENT = 5U;
        static const ubyte_t MESSAGE_TYPE_CLIENTCUTTEXT = 6U;
        // SC message types
        static const ubyte_t MESSAGE_TYPE_FRAMEBUFFERUPDATE = 0U;
        static const ubyte_t MESSAGE_TYPE_SETCOLORMAPENTRIES = 1U;
        static const ubyte_t MESSAGE_TYPE_BELL = 2U;
        static const ubyte_t MESSAGE_TYPE_SERVERCUTTEXT = 3U;

        LPFNPACKETREAD m_lpfnPeek;
        LPFNPACKETREAD m_lpfnRead;
        LPFNPACKETSEND m_lpfnSend;
        void* m_lpContext;

    protected:
        bool __WDECL Peek(void* const lpBuffer, const size_t uBufferSize)
        {
            return m_lpfnPeek(lpBuffer, uBufferSize, m_lpContext);
        }
        bool __WDECL Read(void* const lpBuffer, const size_t uBufferSize)
        {
            return m_lpfnRead(lpBuffer, uBufferSize, m_lpContext);
        }
        bool __WDECL Send(const void* const lpData, const size_t uDataLength)
        {
            return m_lpfnSend(lpData, uDataLength, m_lpContext);
        }
        template< typename T > bool __WDECL AllocateBuffer(const size_t uLength, T** const lppBuffer)
        {
            return T_ALLOC(uLength, &malloc, lppBuffer);
        }

    public:
        // Constructor
        //
        CPacketFactory(const LPFNPACKETREAD lpfnPeek, const LPFNPACKETREAD lpfnRead, const LPFNPACKETSEND lpfnSend, void* const lpContext)
        {
            m_lpfnPeek  = lpfnPeek;
            m_lpfnRead  = lpfnRead;
            m_lpfnSend  = lpfnSend;
            m_lpContext = lpContext;
        }
        // Dynamic Memory Releaser
        //
        template< typename T > void __WDECL ReleaseBuffer(T** const lppBuffer)
        {
            T_FREE(&free, lppBuffer);
        }
        // Head
        //
        bool __WDECL PeekPacket(ubyte_t* const lpucMessageType)
        {
            struct PACKET__HEAD Packet;

            if(Peek(&Packet, sizeof(Packet)))
            {
                lpucMessageType[0] = Packet.ucMessageType;

                return true;
            }

            return false;
        }
        // Reason
        //
        bool __WDECL SendReason(const char* const lpszReason)
        {
            size_t uLength = strlen(lpszReason);
            ulong_t uBELength = htob32(uLength);

            return Send(&uBELength, sizeof(uBELength)) && Send(lpszReason, sizeof(lpszReason[0])*uLength);
        }
        bool __WDECL ReadReason(char** const lppszReason)
        {
            ulong_t uBELength;

            if(Read(&uBELength, sizeof(uBELength)))
            {
                char* lpszReason;
                size_t uLength = btoh32(uBELength);

                if(AllocateBuffer(sizeof(lpszReason[0])*(uLength+1U), &lpszReason))
                {
                    if(Read(lpszReason, sizeof(lpszReason[0])*uLength))
                    {
                        // zero-terminate
                        lpszReason[uLength] = 0;

                        lppszReason[0] = lpszReason;

                        return true;
                    }

                    ReleaseBuffer(&lpszReason);
                }
            }

            return false;
        }
        // ProtocolVersion
        //
        bool __WDECL SendProtocolVersion(const unsigned int uMajor, const unsigned int uMinor)
        {
            char szBuffer[PROTOCOL_VERSION_LENGTH+1U];

            AssertHere(uMajor<1000U);
            AssertHere(uMinor<1000U);

            snprintf(szBuffer, __ARRAYSIZE(szBuffer), "RFB %03u.%03u\n", uMajor, uMinor);

            return Send(szBuffer, sizeof(szBuffer)-1U);
        }
        bool __WDECL ReadProtocolVersion(unsigned int* const lpuMajor, unsigned int* const lpuMinor)
        {
            char szBuffer[PROTOCOL_VERSION_LENGTH+1U];

            if(Read(szBuffer, sizeof(szBuffer)-1U))
            {
                int nPosition = 0;

                // zero-terminate
                szBuffer[sizeof(szBuffer)-1U] = 0;

                if(sscanf(szBuffer, "RFB %03u.%03u\n%n", lpuMajor, lpuMinor, &nPosition)==3 && nPosition==sizeof(szBuffer)-1U)
                {
                    return true;
                }
            }

            return false;
        }
        // Security
        //
        bool __WDECL SendServerSecurity(const ubyte_t* const lpucSecurityList, const ubyte_t ucCount)
        {
            return Send(&ucCount, sizeof(ucCount)) && Send(lpucSecurityList, sizeof(lpucSecurityList[0])*ucCount);
        }
        bool __WDECL ReadServerSecurity(ubyte_t** const lppucSecurityList, size_t* const lpuCount)
        {
            ubyte_t ucCount;

            if(Read(&ucCount, sizeof(ucCount)))
            {
                ubyte_t* lpucSecurityList;

                if(AllocateBuffer(sizeof(lpucSecurityList[0])*ucCount, &lpucSecurityList))
                {
                    if(Read(lpucSecurityList, sizeof(lpucSecurityList[0])*ucCount))
                    {
                        lppucSecurityList[0] = lpucSecurityList;
                        lpuCount[0] = ucCount;

                        return true;
                    }

                    ReleaseBuffer(&lpucSecurityList);
                }
            }

            return false;
        }
        bool __WDECL SendClientSecurity(const ubyte_t ucSecurity)
        {
            return Send(&ucSecurity, sizeof(ucSecurity));
        }
        bool __WDECL ReadClientSecurity(ubyte_t* const lpucSecurity)
        {
            return Read(lpucSecurity, sizeof(lpucSecurity[0]));
        }
        // SecurityResult
        //
        bool __WDECL SendSecurityResult(const ulong_t ulResult)
        {
            return Send(&ulResult, sizeof(ulResult));
        }
        bool __WDECL ReadSecurityResult(ulong_t* const lpulResult)
        {
            return Read(lpulResult, sizeof(lpulResult[0]));
        }
        // VNC Authentication
        //
        bool __WDECL SendServerVNCAuthentication(const void* const lpChallenge, const size_t uChallengeLength)
        {
            if(uChallengeLength!=VNC_AUTHENTICATION_CHALLENGE_LENGTH)
            {
                return false;
            }

            return Send(lpChallenge, uChallengeLength);
        }
        bool __WDECL ReadServerVNCAuthentication(void* const lpChallenge, const size_t uChallengeSize)
        {
            if(uChallengeSize<VNC_AUTHENTICATION_CHALLENGE_LENGTH)
            {
                return false;
            }

            return Read(lpChallenge, VNC_AUTHENTICATION_CHALLENGE_LENGTH);
        }
        bool __WDECL SendClientVNCAuthentication(const void* const lpResponse, const size_t uResponseLength)
        {
            if(uResponseLength!=VNC_AUTHENTICATION_RESPONSE_LENGTH)
            {
                return false;
            }

            return Send(lpResponse, uResponseLength);
        }
        bool __WDECL ReadClientVNCAuthentication(void* const lpResponse, const size_t uResponseSize)
        {
            if(uResponseSize<VNC_AUTHENTICATION_RESPONSE_LENGTH)
            {
                return false;
            }

            return Read(lpResponse, VNC_AUTHENTICATION_RESPONSE_LENGTH);
        }
        // ClientInit
        //
        bool __WDECL SendClientInit(LPCCLIENTINIT const lpClientInit)
        {
            struct PACKET_CLIENTINIT Packet = { 0 };

            Packet.ucSharedFlag = lpClientInit->bIsShared;

            return Send(&Packet, sizeof(Packet));
        }
        bool __WDECL ReadClientInit(LPCLIENTINIT const lpClientInit)
        {
            struct PACKET_CLIENTINIT Packet;

            if(Read(&Packet, sizeof(Packet)))
            {
                lpClientInit->bIsShared  = Packet.ucSharedFlag!=0U;

                return true;
            }

            return false;
        }
        // ServerInit
        //
        bool __WDECL SendServerInit(LPCSERVERINIT const lpServerInit)
        {
            size_t uNameLength = strlen(lpServerInit->lpszName);
            struct PACKET_SERVERINIT Packet = { 0 };

            Packet.uwWidth  = htob16(lpServerInit->uwWidth);
            Packet.uwHeight = htob16(lpServerInit->uwHeight);
            Packet.PixelFormat.ucBitsPerPixel  = lpServerInit->PixelFormat.ucBitsPerPixel;
            Packet.PixelFormat.ucBitDepth      = lpServerInit->PixelFormat.ucBitDepth;
            Packet.PixelFormat.ucBigEndianFlag = lpServerInit->PixelFormat.bIsBigEndian;
            Packet.PixelFormat.ucTrueColorFlag = lpServerInit->PixelFormat.bIsTrueColor;

            if(lpServerInit->PixelFormat.bIsTrueColor)
            {
                Packet.PixelFormat.uwRMax   = htob16(lpServerInit->PixelFormat.uwRMax);
                Packet.PixelFormat.uwGMax   = htob16(lpServerInit->PixelFormat.uwGMax);
                Packet.PixelFormat.uwBMax   = htob16(lpServerInit->PixelFormat.uwBMax);
                Packet.PixelFormat.ucRShift = lpServerInit->PixelFormat.ucRShift;
                Packet.PixelFormat.ucGShift = lpServerInit->PixelFormat.ucGShift;
                Packet.PixelFormat.ucBShift = lpServerInit->PixelFormat.ucBShift;
            }

            Packet.ulNameLength = htob32(uNameLength);

            return Send(&Packet, sizeof(Packet)) && Send(lpServerInit->lpszName, sizeof(lpServerInit->lpszName[0])*uNameLength);
        }
        bool __WDECL ReadServerInit(LPSERVERINIT* const lppServerInit)
        {
            struct PACKET_SERVERINIT Packet;

            if(Read(&Packet, sizeof(Packet)))
            {
                size_t uNameLength = btoh32(Packet.ulNameLength);
                LPSERVERINIT lpServerInit;

                if(AllocateBuffer(sizeof(lpServerInit[0])+sizeof(lpServerInit->lpszName[0])*(uNameLength+1U), &lpServerInit))
                {
                    if(Read(&lpServerInit[1], sizeof(lpServerInit->lpszName[0])*uNameLength))
                    {
                        lpServerInit->uwWidth  = btoh16(Packet.uwWidth);
                        lpServerInit->uwHeight = btoh16(Packet.uwHeight);
                        lpServerInit->PixelFormat.ucBitsPerPixel = Packet.PixelFormat.ucBitsPerPixel;
                        lpServerInit->PixelFormat.ucBitDepth     = Packet.PixelFormat.ucBitDepth;
                        lpServerInit->PixelFormat.bIsBigEndian   = Packet.PixelFormat.ucBigEndianFlag!=0U;
                        lpServerInit->PixelFormat.bIsTrueColor   = Packet.PixelFormat.ucTrueColorFlag!=0U;

                        if(lpServerInit->PixelFormat.bIsTrueColor)
                        {
                            lpServerInit->PixelFormat.uwRMax   = btoh16(Packet.PixelFormat.uwRMax);
                            lpServerInit->PixelFormat.uwGMax   = btoh16(Packet.PixelFormat.uwGMax);
                            lpServerInit->PixelFormat.uwBMax   = btoh16(Packet.PixelFormat.uwBMax);
                            lpServerInit->PixelFormat.ucRShift = Packet.PixelFormat.ucRShift;
                            lpServerInit->PixelFormat.ucGShift = Packet.PixelFormat.ucGShift;
                            lpServerInit->PixelFormat.ucBShift = Packet.PixelFormat.ucBShift;
                        }

                        lpServerInit->lpszName = static_cast< char* >(static_cast< void* >(&lpServerInit[1]));

                        // zero-terminate
                        lpServerInit->lpszName[uNameLength] = 0;

                        lppServerInit[0] = lpServerInit;

                        return true;
                    }

                    ReleaseBuffer(&lpServerInit);
                }
            }

            return false;
        }
        // SetPixelFormat
        //
        bool __WDECL SendSetPixelFormat(LPCPIXELFORMAT const lpPixelFormat)
        {
            RFB_SENDPACKET(SETPIXELFORMAT);

            Packet.PixelFormat.ucBitsPerPixel  = lpPixelFormat->ucBitsPerPixel;
            Packet.PixelFormat.ucBitDepth      = lpPixelFormat->ucBitDepth;
            Packet.PixelFormat.ucBigEndianFlag = lpPixelFormat->bIsBigEndian;
            Packet.PixelFormat.ucTrueColorFlag = lpPixelFormat->bIsTrueColor;

            if(lpPixelFormat->bIsTrueColor)
            {
                Packet.PixelFormat.uwRMax   = htob16(lpPixelFormat->uwRMax);
                Packet.PixelFormat.uwGMax   = htob16(lpPixelFormat->uwGMax);
                Packet.PixelFormat.uwBMax   = htob16(lpPixelFormat->uwBMax);
                Packet.PixelFormat.ucRShift = lpPixelFormat->ucRShift;
                Packet.PixelFormat.ucGShift = lpPixelFormat->ucGShift;
                Packet.PixelFormat.ucBShift = lpPixelFormat->ucBShift;
            }

            return Send(&Packet, sizeof(Packet));
        }
        bool __WDECL ReadSetPixelFormat(LPPIXELFORMAT const lpPixelFormat)
        {
            struct PACKET_SETPIXELFORMAT Packet;

            if(Read(&Packet, sizeof(Packet)))
            {
                AssertHere(Packet.ucMessageType==MESSAGE_TYPE_SETPIXELFORMAT);

                lpPixelFormat->ucBitsPerPixel = Packet.PixelFormat.ucBitsPerPixel;
                lpPixelFormat->ucBitDepth     = Packet.PixelFormat.ucBitDepth;
                lpPixelFormat->bIsBigEndian   = Packet.PixelFormat.ucBigEndianFlag!=0U;
                lpPixelFormat->bIsTrueColor   = Packet.PixelFormat.ucTrueColorFlag!=0U;

                if(lpPixelFormat->bIsTrueColor)
                {
                    lpPixelFormat->uwRMax   = btoh16(Packet.PixelFormat.uwRMax);
                    lpPixelFormat->uwGMax   = btoh16(Packet.PixelFormat.uwGMax);
                    lpPixelFormat->uwBMax   = btoh16(Packet.PixelFormat.uwBMax);
                    lpPixelFormat->ucRShift = Packet.PixelFormat.ucRShift;
                    lpPixelFormat->ucGShift = Packet.PixelFormat.ucGShift;
                    lpPixelFormat->ucBShift = Packet.PixelFormat.ucBShift;
                }

                return true;
            }

            return false;
        }
    };
}
