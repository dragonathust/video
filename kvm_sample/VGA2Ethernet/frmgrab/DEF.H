#define Watcom 1
//#define Linux 1
//#define Windows 1


#ifdef Windows
#include <wchar.h>
#include <ntddk.h>
#include <ntdef.h>
#endif

#define    FLONG                    unsigned long
#define    BYTE                     unsigned char
#define    INT                      int
#define    VOID                     void
#define    BOOLEAN                  unsigned short

#ifndef Windows
#define    ULONG                    unsigned long
#define    USHORT                   unsigned short
#define    UCHAR                    unsigned char
#define    CHAR                     char
#define    LONG                     long
#define    PUCHAR                   UCHAR *
#define    PULONG                   ULONG *
#endif

#define    PCI                      1
#define    PCIE                     2
#define    AGP                      3
#define    ACTIVE                   4
#define    PASS                     0
//#define    TRUE                     0
//#define    FALSE                    1
#define    FAIL                     1
//#define    FIX                      0
#define    STEP                     1
#define    SEQ_ADDRESS_REGISTER     0x3C4
#define    SEQ_DATA_REGISTER        0x3C5
#define    CRTC_ADDRESS_REGISTER    0x3D4
#define    CRTC_DATA_REGISTER       0x3D5
#define    DAC_INDEX_REGISTER       0x3C8
#define    DAC_DATA_REGISTER        0x3C9
//PCI information define
#define    HOST_BRIDGE_MIN                             0x80000000
#define    HOST_BRIDGE_MAX                             0x80060000
#define    PCI_SLOT_STEP                               0x800
#define    AGP_SLOT_MIN                                0x80010000
#define    AGP_SLOT_MAX                                0x8001F800
#define    PCI_ADDR_PORT                               0x0CF8
#define    PCI_DATA_PORT                               0x0CFC
#define    REG_VENENDER_ID_DEVICE_ID                   0
#define    REG_PCI_STATUS_COMMAND                      0x04
#define    REG_PCI_CLASS_REVISION_ID                   0x08
#define    REG_CAPABILITY_POINTER                      0x34
#define    PhysicalBase_Offset                         0x10
#define    MMIO_Offset                                 0x14
#define    RelocateIO_Offset                           0x18

//  Registers
#define    VIDEOBASE_OFFSET                            0x10000
#define    KEY_CONTROL                                 0x00 + VIDEOBASE_OFFSET
#define    VIDEOENGINE_SEQUENCE_CONTROL                0x04 + VIDEOBASE_OFFSET
#define    VIDEOENGINE_PASS1_CONTROL                   0x08 + VIDEOBASE_OFFSET
#define    VIDEOENGINE_MODEDETECTIONSETTING_H          0x0C + VIDEOBASE_OFFSET
#define    VIDEOENGINE_MODEDETECTIONSETTING_V          0x10 + VIDEOBASE_OFFSET
#define    SCALE_FACTOR_REGISTER                       0x14 + VIDEOBASE_OFFSET
#define    SCALING_FILTER_PARAMETERS_1                 0x18 + VIDEOBASE_OFFSET
#define    SCALING_FILTER_PARAMETERS_2                 0x1C + VIDEOBASE_OFFSET
#define    SCALING_FILTER_PARAMETERS_3                 0x20 + VIDEOBASE_OFFSET
#define    SCALING_FILTER_PARAMETERS_4                 0x24 + VIDEOBASE_OFFSET
#define    MODEDETECTION_STATUS_READBACK               0x28 + VIDEOBASE_OFFSET
#define    VIDEOPROCESSING_CONTROL                     0x2C + VIDEOBASE_OFFSET
#define    VIDEO_PROCESS_WINDOW_SETTING                0x30 + VIDEOBASE_OFFSET
#define    VIDEO_COMPRESS_WINDOW_SETTING               0x34 + VIDEOBASE_OFFSET
#define    VIDEO_IN_BUFFER_BASEADDRESS                 0x38 + VIDEOBASE_OFFSET
#define    VIDEO_IN_BUFFER_OFFSET                      0x3C + VIDEOBASE_OFFSET
#define    VIDEOPROCESS_BUFFER_BASEADDRESS             0x40 + VIDEOBASE_OFFSET 
#define    VIDEOCOMPRESS_SOURCE_BUFFER_BASEADDRESS     0x44 + VIDEOBASE_OFFSET
#define    VIDEOPROCESS_OFFSET                         0x48 + VIDEOBASE_OFFSET
#define    VIDEOPROCESS_REFERENCE_BUFFER_BASEADDRESS   0x4C + VIDEOBASE_OFFSET
#define    FLAG_BUFFER_BASEADDRESS                     0x50 + VIDEOBASE_OFFSET
#define    VIDEO_COMPRESS_DESTINATION_BASEADDRESS      0x54 + VIDEOBASE_OFFSET
#define    DOWN_SCALE_LINES_REGISTER                   0x58 + VIDEOBASE_OFFSET
#define    VIDEO_CAPTURE_BOUND_REGISTER                0x5C + VIDEOBASE_OFFSET
#define    VIDEO_COMPRESS_CONTROL                      0x60 + VIDEOBASE_OFFSET
#define    BLOCK_SHARPNESS_DETECTION_CONTROL           0x64 + VIDEOBASE_OFFSET
#define    POST_WRITE_BUFFER_DRAM_THRESHOLD            0x68 + VIDEOBASE_OFFSET
#define    VIDEO_INTERRUPT_CONTROL                     0x6C + VIDEOBASE_OFFSET
#define    COMPRESS_DATA_COUNT_REGISTER                0x70 + VIDEOBASE_OFFSET
#define    COMPRESS_BLOCK_COUNT_REGISTER               0x74 + VIDEOBASE_OFFSET
#define    VQ_Y_VECTOR_CODE_REGISTER_START             0x80 + VIDEOBASE_OFFSET
#define    VQ_U_VECTOR_CODE_REGISTER_START             0x90 + VIDEOBASE_OFFSET
#define    VQ_V_VECTOR_CODE_REGISTER_START             0xB0 + VIDEOBASE_OFFSET
#define    U_SUM_HISTOGRAM                             0x500 + VIDEOBASE_OFFSET
#define    U_COUNT_HISTOGRAM                           0x480 + VIDEOBASE_OFFSET
#define    V_SUM_HISTOGRAM                             0x400 + VIDEOBASE_OFFSET
#define    V_COUNT_HISTOGRAM                           0x480 + VIDEOBASE_OFFSET
#define    Y_SUM_HISTOGRAM                             0x580 + VIDEOBASE_OFFSET
#define    Y_COUNT_HISTOGRAM                           0x5C0 + VIDEOBASE_OFFSET
#define    RC4KEYS_REGISTER                            0x600 + VIDEOBASE_OFFSET
#define    VQHUFFMAN_TABLE_REGISTER                    0x300 + VIDEOBASE_OFFSET


//  Parameters
#define    SAMPLE_RATE                                 12000000.0
#define    MODEDETECTION_VERTICAL_STABLE_MAXIMUM       0x4
#define    MODEDETECTION_HORIZONTAL_STABLE_MAXIMUM     0x4
#define    MODEDETECTION_VERTICAL_STABLE_THRESHOLD     0x4
#define    MODEDETECTION_HORIZONTAL_STABLE_THRESHOLD   0x8
#define    HORIZONTAL_SCALING_FILTER_PARAMETERS_LOW    0xFFFFFFFF
#define    HORIZONTAL_SCALING_FILTER_PARAMETERS_HIGH   0xFFFFFFFF
#define    VIDEO_WRITE_BACK_BUFFER_THRESHOLD_LOW       0x08
#define    VIDEO_WRITE_BACK_BUFFER_THRESHOLD_HIGH      0x04
#define    VQ_Y_LEVELS                                 0x10
#define    VQ_UV_LEVELS                                0x05
#define    EXTERNAL_VIDEO_HSYNC_POLARITY               0x01
#define    EXTERNAL_VIDEO_VSYNC_POLARITY               0x01
#define    VIDEO_SOURCE_FROM                           0x01
#define    EXTERNAL_ANALOG_SOURCE                      0x01
#define    USE_INTERNAL_TIMING_GENERATOR               0x01
#define    WRITE_DATA_FORMAT                           0x00
#define    SET_BCD_TO_WHOLE_FRAME                      0x01
#define    ENABLE_VERTICAL_DOWN_SCALING                0x01
#define    BCD_TOLERENCE                               0xFF
#define    BCD_START_BLOCK_XY                          0x0
#define    BCD_END_BLOCK_XY                            0x3FFF
#define    COLOR_DEPTH                                 16
#define    BLOCK_SHARPNESS_DETECTION_HIGH_THRESHOLD    0xFF
#define    BLOCK_SHARPNESS_DETECTION_LOE_THRESHOLD     0xFF
#define    BLOCK_SHARPNESS_DETECTION_HIGH_COUNTS_THRESHOLD   0x3F
#define    BLOCK_SHARPNESS_DETECTION_LOW_COUNTS_THRESHOLD    0x1F
#define    VQTABLE_AUTO_GENERATE_BY_HARDWARE           0x0
#define    VQTABLE_SELECTION                           0x0
#define    JPEG_COMPRESS_ONLY                          0x0
#define    DUAL_MODE_COMPRESS                          0x1
#define    BSD_H_AND_V                                 0x0
#define    ENABLE_RC4_ENCRYPTION                       0x1
#define    BSD_ENABLE_HIGH_THRESHOLD_CHECK             0x0
#define    VIDEO_PROCESS_AUTO_TRIGGER                  0x0
#define    VIDEO_COMPRESS_AUTO_TRIGGER                 0x0
#define    VIDEO_IN_BUFFER_ADDRESS                     0x800000
#define    VIDEO_PROCESS_BUFFER_ADDRESS                0x200000
#define    VIDEO_REFERENCE_BUFFER_ADDRESS              0xC00000
#define    VIDEO_COMPRESS_SOURCE_BUFFER_ADDRESS        0x200000
#define    VIDEO_COMPRESS_DESTINATION_BUFFER_ADDRESS   0x3C00000
#define    VIDEO_FLAG_BUFFER_ADDRESS                   0xF00000


#define    ReadMemoryBYTE(baseaddress,offset)        *(BYTE *)((ULONG)(baseaddress)+(ULONG)(offset))
#define    ReadMemoryLong(baseaddress,offset)        *(ULONG *)((ULONG)(baseaddress)+(ULONG)(offset))
#define    ReadMemoryShort(baseaddress,offset)       *(USHORT *)((ULONG)(baseaddress)+(ULONG)(offset))
#define    WriteMemoryBYTE(baseaddress,offset,data)  *(BYTE *)((ULONG)(baseaddress)+(ULONG)(offset)) = (BYTE)(data)
#define    WriteMemoryLong(baseaddress,offset,data)  *(ULONG *)((ULONG)(baseaddress)+(ULONG)(offset))=(ULONG)(data)
#define    WriteMemoryShort(baseaddress,offset,data) *(USHORT *)((ULONG)(baseaddress)+(ULONG)(offset))=(USHORT)(data)
#define    WriteMemoryLongWithANDData(baseaddress, offset, anddata, data)  *(ULONG *)((ULONG)(baseaddress)+(ULONG)(offset)) = *(ULONG *)((ULONG)(baseaddress)+(ULONG)(offset)) & (ULONG)(anddata) | (ULONG)(data)

#define    intfunc      int386

#ifndef  Linux
#define    outdwport         outpd
#define    indwport          inpd
#define    outport           outp
#define    inport            inp
#endif

#ifdef   Linux
#include <asm/io.h>
#include <linux/pci_ids.h>
#define    u8                unsigned char
#define    u16               unsigned short
#define    u32               unsigned int

#define    outdwport(p,v)    outl((u32)(v),(u16)(p))
#define    indwport(p)       inl((u16)(p))
#define    outport(p,v)      outb((u8)(v),(u16)(p))
#define    inport(p)         inb((u16)(p))
#endif

#ifndef  _STRUCTURE_INFO
#define  _STRUCTURE_INFO

//Buffer structure
typedef struct _XMS_BUFFER
{
    ULONG     handle;
    ULONG     size;
    ULONG     physical_address;
    ULONG     virtual_address;
} XMS_BUFFER, *PXMS_BUFFER;

//PCI info structure
typedef struct
{
    USHORT    usVendorID;
    USHORT    usDeviceID;
    ULONG     ulPCIConfigurationBaseAddress;
    BYTE      jAGPStatusPort;
    BYTE      jAGPCommandPort;
    BYTE      jAGPVersion;
} PCI_INFO;

typedef struct _DEVICE_PCI_INFO
{
    USHORT    usVendorID;
    USHORT    usDeviceID;
    ULONG     ulPCIConfigurationBaseAddress;
    ULONG     ulPhysicalBaseAddress;
    ULONG     ulMMIOBaseAddress;
    USHORT    usRelocateIO;
} DEVICE_PCI_INFO, *PDEVICE_PCI_INFO;

typedef struct _VIDEO_MODE_INFO
{
    USHORT    X;
    USHORT    Y;
    USHORT    ColorDepth;
    USHORT    RefreshRate;
    BYTE      ModeIndex;
} VIDEO_MODE_INFO, *PVIDEO_MODE_INFO;

typedef struct _VQ_INFO {
    BYTE    Y[16];
    BYTE    U[32];
    BYTE    V[32];
    BYTE    NumberOfY;
    BYTE    NumberOfUV;
    BYTE    NumberOfInner;
    BYTE    NumberOfOuter;
} VQ_INFO, *PVQ_INFO;

typedef struct _HUFFMAN_TABLE {
    ULONG  HuffmanCode[32];
} HUFFMAN_TABLE, *PHUFFMAN_TABLE;

typedef struct _FRAME_HEADER {
    ULONG     StartCode;
    ULONG     FrameNumber;
    USHORT    HSize;
    USHORT    VSize;
    ULONG     Reserved[2];
    BYTE      CompressionMode;
    BYTE      JPEGScaleFactor;
    BYTE      Y_JPEGTableSelector;
    BYTE      UV_JPEGTableSelector;
    BYTE      JPEGYUVTableMapping;
    BYTE      SharpModeSelection;
    BYTE      AdvanceTableSelector;
    BYTE      AdvanceScaleFactor;
    ULONG     NumberOfMB;
    BYTE      VQ_YLevel;
    BYTE      VQ_UVLevel;
    VQ_INFO   VQVectors;
    BYTE      RC4Enable;
    BYTE      RC4Reset;
} FRAME_HEADER, *PFRAME_HEADER;

typedef struct _INF_DATA {
    BYTE    DownScalingMethod;
    BYTE    DifferentialSetting;
    USHORT  AnalogDifferentialThreshold;
    USHORT  DigitalDifferentialThreshold;
    BYTE    ExternalSignalEnable;
    BYTE    AutoMode;
    double  Gamma1Parameter;
    double  Gamma2Parameter;
    double  Gamma1_Gamma2_Seperate;
} INF_DATA, *PINF_DATA;

typedef struct _TIME_DATA {
    ULONG    UnitTimeLow;
    ULONG    UnitTimeHigh;
    ULONG    StartTimeLow;
    ULONG    StartTimeHigh;
    ULONG    TimeUsed;
} TIME_DATA, *PTIME_DATA;

typedef struct _COMPRESS_DATA {
    ULONG   SourceFrameSize;
    ULONG   CompressSize;
    ULONG   HDebug;
    ULONG   VDebug;
} COMPRESS_DATA, *PCOMPRESS_DATA;

//VIDEO Engine Info
typedef struct _VIDEO_ENGINE_INFO {
    USHORT             iEngVersion;
    DEVICE_PCI_INFO    VGAPCIInfo;
    XMS_BUFFER         VideoINBuffer;
    XMS_BUFFER         VideoProcessBuffer;
    XMS_BUFFER         VideoProcessReferenceBuffer;
    XMS_BUFFER         FlagBuffer;
    XMS_BUFFER         VideoCompressBuffer;
    XMS_BUFFER         VideoOutputBuffer;
    VIDEO_MODE_INFO    SourceModeInfo;
    VIDEO_MODE_INFO    DestinationModeInfo;
    VQ_INFO            VQInfo;
    FRAME_HEADER       FrameHeader;
    INF_DATA           INFData;
    COMPRESS_DATA      CompressData;
    TIME_DATA          TimeData;
    BYTE               InputSignal;
} VIDEO_ENGINE_INFO, *PVIDEO_ENGINE_INFO;

typedef struct _TRANSFER_HEADER {
    ULONG     Data_Length;
    ULONG     Blocks_Changed;
    USHORT    User_Width;
    USHORT    User_Height;
#if 1	
BYTE	Frist_frame;	
BYTE	Compress_type;	//0:aspeed mode, 1:jpeg mode
BYTE	Trigger_mode;	//0:capture, 1: compression, 2: buffer
BYTE	Data_format;	//0:DCT, 1:DCTwVQ2 color, 2:DCTwVQ4 color
#else
    USHORT    Source_Width;
    USHORT    Source_Height;
#endif	
    BYTE      RC4_Enable;
    BYTE      RC4_Reset;
    BYTE      Y_Table;
    BYTE      UV_Table;
    BYTE      Mode_420;
    BYTE      Direct_Mode;
    BYTE      VQ_Mode;
    BYTE      Disable_VGA;
    BYTE      Differential_Enable;
    BYTE      Auto_Mode;
    BYTE      VGA_Status;
    BYTE      RC4State;
    BYTE      Advance_Table;
} TRANSFER_HEADER, *PTRANSFER_HEADER;

typedef struct {
    USHORT    HorizontalTotal;
    USHORT    VerticalTotal;
    USHORT    HorizontalActive;
    USHORT    VerticalActive;
    BYTE      RefreshRate;
    double    HorizontalFrequency;
    USHORT    HSyncTime;
    USHORT    HBackPorch;
    USHORT    VSyncTime;
    USHORT    VBackPorch;
    USHORT    HLeftBorder;
    USHORT    HRightBorder;
    USHORT    VBottomBorder;
    USHORT    VTopBorder;
} VESA_MODE;

typedef struct {
    USHORT    HorizontalActive;
    USHORT    VerticalActive;
    USHORT    RefreshRate;
    BYTE      ADCIndex1;
    BYTE      ADCIndex2;
    BYTE      ADCIndex3;
    BYTE      ADCIndex5;
    BYTE      ADCIndex6;
    BYTE      ADCIndex7;
    BYTE      ADCIndex8;
    BYTE      ADCIndex9;
    BYTE      ADCIndexA;
    BYTE      ADCIndexF;
    BYTE      ADCIndex15;
    int       HorizontalShift;
    int       VerticalShift;
} ADC_MODE;

struct COLOR_CACHE {
	unsigned long Color[4];
	unsigned char Index[4];
	unsigned char BitMapBits;
};

struct RGB {
	unsigned char B;
	unsigned char G;
	unsigned char R;
	unsigned char Reserved;
};

struct YUV444 {
	unsigned char U;
	unsigned char Y;
	unsigned char V;
};

struct YUV422 {
	unsigned char Y0;
	unsigned char U;
	unsigned char Y1;
	unsigned char V;
};

//  RC4 structure
struct rc4_state
{
    int x;
    int y;
    int m[256];
};

//  RC4 keys. Current keys are fedcba98765432210
static unsigned char EncodeKeys[256];
static unsigned char DecodeKeys[256] = "fedcba9876543210";

#endif
