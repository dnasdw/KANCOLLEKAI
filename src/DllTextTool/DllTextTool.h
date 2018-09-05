#ifndef DLLTEXT_DLLTEXT_H_
#define DLLTEXT_DLLTEXT_H_

#include <sdw.h>

#if SDW_PLATFORM != SDW_PLATFORM_WINDOWS

#define IMAGE_DOS_SIGNATURE                 0x5A4D      // MZ
#define IMAGE_NT_SIGNATURE                  0x00004550  // PE00

#include SDW_MSC_PUSH_PACKED
typedef struct _IMAGE_DOS_HEADER {      // DOS .EXE header
    u16    e_magic;                     // Magic number
    u16    e_cblp;                      // Bytes on last page of file
    u16    e_cp;                        // Pages in file
    u16    e_crlc;                      // Relocations
    u16    e_cparhdr;                   // Size of header in paragraphs
    u16    e_minalloc;                  // Minimum extra paragraphs needed
    u16    e_maxalloc;                  // Maximum extra paragraphs needed
    u16    e_ss;                        // Initial (relative) SS value
    u16    e_sp;                        // Initial SP value
    u16    e_csum;                      // Checksum
    u16    e_ip;                        // Initial IP value
    u16    e_cs;                        // Initial (relative) CS value
    u16    e_lfarlc;                    // File address of relocation table
    u16    e_ovno;                      // Overlay number
    u16    e_res[4];                    // Reserved words
    u16    e_oemid;                     // OEM identifier (for e_oeminfo)
    u16    e_oeminfo;                   // OEM information; e_oemid specific
    u16    e_res2[10];                  // Reserved words
    n32    e_lfanew;                    // File address of new exe header
  } SDW_GNUC_PACKED IMAGE_DOS_HEADER, *PIMAGE_DOS_HEADER;

typedef struct _IMAGE_FILE_HEADER {
    u16     Machine;
    u16     NumberOfSections;
    u32     TimeDateStamp;
    u32     PointerToSymbolTable;
    u32     NumberOfSymbols;
    u16     SizeOfOptionalHeader;
    u16     Characteristics;
} SDW_GNUC_PACKED IMAGE_FILE_HEADER, *PIMAGE_FILE_HEADER;

#define IMAGE_FILE_32BIT_MACHINE             0x0100  // 32 bit word machine.

typedef struct _IMAGE_DATA_DIRECTORY {
    u32     VirtualAddress;
    u32     Size;
} SDW_GNUC_PACKED IMAGE_DATA_DIRECTORY, *PIMAGE_DATA_DIRECTORY;

#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES    16

typedef struct _IMAGE_OPTIONAL_HEADER {
    //
    // Standard fields.
    //

    u16     Magic;
    u8      MajorLinkerVersion;
    u8      MinorLinkerVersion;
    u32     SizeOfCode;
    u32     SizeOfInitializedData;
    u32     SizeOfUninitializedData;
    u32     AddressOfEntryPoint;
    u32     BaseOfCode;
    u32     BaseOfData;

    //
    // NT additional fields.
    //

    u32     ImageBase;
    u32     SectionAlignment;
    u32     FileAlignment;
    u16     MajorOperatingSystemVersion;
    u16     MinorOperatingSystemVersion;
    u16     MajorImageVersion;
    u16     MinorImageVersion;
    u16     MajorSubsystemVersion;
    u16     MinorSubsystemVersion;
    u32     Win32VersionValue;
    u32     SizeOfImage;
    u32     SizeOfHeaders;
    u32     CheckSum;
    u16     Subsystem;
    u16     DllCharacteristics;
    u32     SizeOfStackReserve;
    u32     SizeOfStackCommit;
    u32     SizeOfHeapReserve;
    u32     SizeOfHeapCommit;
    u32     LoaderFlags;
    u32     NumberOfRvaAndSizes;
    IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} SDW_GNUC_PACKED IMAGE_OPTIONAL_HEADER32, *PIMAGE_OPTIONAL_HEADER32;

#define IMAGE_NT_OPTIONAL_HDR32_MAGIC      0x10b

typedef struct _IMAGE_NT_HEADERS {
    u32 Signature;
    IMAGE_FILE_HEADER FileHeader;
    IMAGE_OPTIONAL_HEADER32 OptionalHeader;
} SDW_GNUC_PACKED IMAGE_NT_HEADERS32, *PIMAGE_NT_HEADERS32;

#define IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR 14   // COM Runtime descriptor

#define IMAGE_SIZEOF_SHORT_NAME              8

typedef struct _IMAGE_SECTION_HEADER {
    u8      Name[IMAGE_SIZEOF_SHORT_NAME];
    union {
            u32     PhysicalAddress;
            u32     VirtualSize;
    } Misc;
    u32     VirtualAddress;
    u32     SizeOfRawData;
    u32     PointerToRawData;
    u32     PointerToRelocations;
    u32     PointerToLinenumbers;
    u16     NumberOfRelocations;
    u16     NumberOfLinenumbers;
    u32     Characteristics;
} SDW_GNUC_PACKED IMAGE_SECTION_HEADER, *PIMAGE_SECTION_HEADER;

#ifndef __IMAGE_COR20_HEADER_DEFINED__
#define __IMAGE_COR20_HEADER_DEFINED__

typedef enum ReplacesCorHdrNumericDefines
{
// COM+ Header entry point flags.
    COMIMAGE_FLAGS_ILONLY               =0x00000001,
    COMIMAGE_FLAGS_32BITREQUIRED        =0x00000002,
    COMIMAGE_FLAGS_IL_LIBRARY           =0x00000004,
    COMIMAGE_FLAGS_STRONGNAMESIGNED     =0x00000008,
    COMIMAGE_FLAGS_NATIVE_ENTRYPOINT    =0x00000010,
    COMIMAGE_FLAGS_TRACKDEBUGDATA       =0x00010000,

// Version flags for image.
    COR_VERSION_MAJOR_V2                =2,
    COR_VERSION_MAJOR                   =COR_VERSION_MAJOR_V2,
    COR_VERSION_MINOR                   =0,
    COR_DELETED_NAME_LENGTH             =8,
    COR_VTABLEGAP_NAME_LENGTH           =8,

// Maximum size of a NativeType descriptor.
    NATIVE_TYPE_MAX_CB                  =1,
    COR_ILMETHOD_SECT_SMALL_MAX_DATASIZE=0xFF,

// #defines for the MIH FLAGS
    IMAGE_COR_MIH_METHODRVA             =0x01,
    IMAGE_COR_MIH_EHRVA                 =0x02,
    IMAGE_COR_MIH_BASICBLOCK            =0x08,

// V-table constants
    COR_VTABLE_32BIT                    =0x01,          // V-table slots are 32-bits in size.
    COR_VTABLE_64BIT                    =0x02,          // V-table slots are 64-bits in size.
    COR_VTABLE_FROM_UNMANAGED           =0x04,          // If set, transition from unmanaged.
    COR_VTABLE_FROM_UNMANAGED_RETAIN_APPDOMAIN  =0x08,  // If set, transition from unmanaged with keeping the current appdomain.
    COR_VTABLE_CALL_MOST_DERIVED        =0x10,          // Call most derived method described by

// EATJ constants
    IMAGE_COR_EATJ_THUNK_SIZE           =32,            // Size of a jump thunk reserved range.

// Max name lengths
    //@todo: Change to unlimited name lengths.
    MAX_CLASS_NAME                      =1024,
    MAX_PACKAGE_NAME                    =1024,
} ReplacesCorHdrNumericDefines;

// CLR 2.0 header structure.
typedef struct IMAGE_COR20_HEADER
{
    // Header versioning
    u32                     cb;
    u16                     MajorRuntimeVersion;
    u16                     MinorRuntimeVersion;

    // Symbol table and startup information
    IMAGE_DATA_DIRECTORY    MetaData;
    u32                     Flags;

    // If COMIMAGE_FLAGS_NATIVE_ENTRYPOINT is not set, EntryPointToken represents a managed entrypoint.
    // If COMIMAGE_FLAGS_NATIVE_ENTRYPOINT is set, EntryPointRVA represents an RVA to a native entrypoint.
    union {
        u32                 EntryPointToken;
        u32                 EntryPointRVA;
    } DUMMYUNIONNAME;

    // Binding information
    IMAGE_DATA_DIRECTORY    Resources;
    IMAGE_DATA_DIRECTORY    StrongNameSignature;

    // Regular fixup and binding information
    IMAGE_DATA_DIRECTORY    CodeManagerTable;
    IMAGE_DATA_DIRECTORY    VTableFixups;
    IMAGE_DATA_DIRECTORY    ExportAddressTableJumps;

    // Precompiled image info (internal use only - set to zero)
    IMAGE_DATA_DIRECTORY    ManagedNativeHeader;

} SDW_GNUC_PACKED IMAGE_COR20_HEADER, *PIMAGE_COR20_HEADER;

#endif // __IMAGE_COR20_HEADER_DEFINED__

#include SDW_MSC_POP_PACKED
#endif

#include SDW_MSC_PUSH_PACKED
struct MetadataHeader
{
	u32 Signature;
	u16 MajorVersion;
	u16 MinorVersion;
	u32 ExtraDataOffset;
	u32 VersionStringLength;
} SDW_GNUC_PACKED;

struct StorageHeader
{
	u8 Flags;
	u8 Reserved;
	u16 NumberofStreams;
} SDW_GNUC_PACKED;
#include SDW_MSC_POP_PACKED

#endif	// DLLTEXT_DLLTEXT_H_
