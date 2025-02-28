#ifndef __LINUXTYPES_H__
#define __LINUXTYPES_H__

#include <efi.h>

typedef struct
{
    UINTN Size;
    EFI_MEMORY_DESCRIPTOR *Map;
    UINTN Key;
    UINTN DescriptorSize;
    UINT32 DescriptorVersion;
} MEMORY_MAP;

typedef struct
{
    UINT64 Address; /* start of memory segment */
    UINT64 Size;    /* size of memory segment */
    UINT32 Type;    /* type of memory segment */
} __attribute__((packed)) E820_ENTRY;

typedef struct
{
    E820_ENTRY E820Table[128];
    E820_ENTRY *E820TableExt;
    UINT32 E820EntriesCount;
} E820_MEMORY_MAP;

typedef struct
{
    UINT64 Next;
    UINT32 Type;
    UINT32 Len;
    UINT8 Data[];
} SETUP_DATA;

typedef struct
{
    UINT8 orig_x;             /* 0x00 */
    UINT8 orig_y;             /* 0x01 */
    UINT16 ext_mem_k;         /* 0x02 */
    UINT16 orig_video_page;   /* 0x04 */
    UINT8 orig_video_mode;    /* 0x06 */
    UINT8 orig_video_cols;    /* 0x07 */
    UINT8 flags;              /* 0x08 */
    UINT8 unused2;            /* 0x09 */
    UINT16 orig_video_ega_bx; /* 0x0a */
    UINT16 unused3;           /* 0x0c */
    UINT8 orig_video_lines;   /* 0x0e */
    UINT8 orig_video_isVGA;   /* 0x0f */
    UINT16 orig_video_points; /* 0x10 */

    /* VESA graphic mode -- linear frame buffer */
    UINT16 lfb_width;           /* 0x12 */
    UINT16 lfb_height;          /* 0x14 */
    UINT16 lfb_depth;           /* 0x16 */
    UINT32 lfb_base;            /* 0x18 */
    UINT32 lfb_size;            /* 0x1c */
    UINT16 cl_magic, cl_offset; /* 0x20 */
    UINT16 lfb_linelength;      /* 0x24 */
    UINT8 red_size;             /* 0x26 */
    UINT8 red_pos;              /* 0x27 */
    UINT8 green_size;           /* 0x28 */
    UINT8 green_pos;            /* 0x29 */
    UINT8 blue_size;            /* 0x2a */
    UINT8 blue_pos;             /* 0x2b */
    UINT8 rsvd_size;            /* 0x2c */
    UINT8 rsvd_pos;             /* 0x2d */
    UINT16 vesapm_seg;          /* 0x2e */
    UINT16 vesapm_off;          /* 0x30 */
    UINT16 pages;               /* 0x32 */
    UINT16 vesa_attributes;     /* 0x34 */
    UINT32 capabilities;        /* 0x36 */
    UINT32 ext_lfb_base;        /* 0x3a */
    UINT8 _reserved[2];         /* 0x3e */
} __attribute__((packed)) SCREEN_INFO;

typedef struct
{
    UINT8 setup_sects; /* Sectors for setup code */
    UINT16 root_flags;
    UINT32 syssize;
    UINT16 ram_size;
    UINT16 vid_mode;
    UINT16 root_dev;
    UINT16 boot_flag; /* Boot signature */
    UINT16 jump;
    UINT32 header;
    UINT16 version;
    UINT16 realmode_swtch;
    UINT16 start_sys_seg;
    UINT16 start_sys;
    UINT16 kernel_version;
    UINT8 type_of_loader;
    UINT8 loadflags;
    UINT16 setup_move_size;
    UINT32 code32_start;  /* Start of code loaded high */
    UINT32 ramdisk_image; /* Start of initial ramdisk */
    UINT32 ramdisk_size;  /* Lenght of initial ramdisk */
    UINT32 bootsect_kludge;
    UINT16 heap_end_ptr;
    UINT8 ext_loader_ver;     /* Extended boot loader version */
    UINT8 ext_loader_type;    /* Extended boot loader ID */
    UINT32 cmd_line_ptr;      /* 32-bit pointer to the kernel command line */
    UINT32 initrd_addr_max;   /* Highest legal initrd address */
    UINT32 kernel_alignment;  /* Physical addr alignment required for kernel */
    UINT8 relocatable_kernel; /* Whether kernel is relocatable or not */
    UINT8 min_alignment;
    UINT16 xloadflags;
    UINT32 cmdline_size;
    UINT32 hardware_subarch;
    UINT64 hardware_subarch_data;
    UINT32 payload_offset;
    UINT32 payload_length;
    UINT64 setup_data;
    UINT64 pref_address;
    UINT32 init_size;
    UINT32 handover_offset;
    UINT32 kernel_info_offset;
} __attribute__((packed)) SETUP_HEADER;

typedef struct
{
    UINT32 efi_loager_signature;
    UINT32 efi_systab;
    UINT32 efi_memdesc_size;
    UINT32 efi_memdesc_version;
    UINT32 efi_memmap;
    UINT32 efi_memmap_size;
    UINT32 efi_systab_hi;
    UINT32 efi_memmap_hi;
} EFI_INFO;

typedef struct
{
    SCREEN_INFO screen_info;   //
    UINT8 apm_bios_info[0x14]; //
    UINT8 _pad2[4];            //
    UINT64 tboot_addr;         //
    UINT8 ist_info[0x10];
    UINT64 acpi_rsdp_addr;       //
    UINT8 _pad3[8];              //
    UINT8 hd0_info[16];          //
    UINT8 hd1_info[16];          //
    UINT8 sys_desc_table[0x10];  //
    UINT8 olpc_ofw_header[0x10]; //
    UINT32 ext_ramdisk_image;    //!!!!
    UINT32 ext_ramdisk_size;     //!!!!
    UINT32 ext_cmd_line_ptr;     //!!!!
    UINT8 _pad4[112];            //
    UINT32 cc_blob_address;
    UINT8 edid_info[0x80];         //
    EFI_INFO efi_info;             //!!!!!
    UINT32 alt_mem_k;              //!!!!!!
    UINT32 scratch;                //
    UINT8 e820_entries;            //!!!!!
    UINT8 eddbuf_entries;          //
    UINT8 edd_mbr_sig_buf_entries; //
    UINT8 kmd_status;
    UINT8 secure_boot;
    UINT8 _pad5[2]; //
    UINT8 sentinel;
    UINT8 _pad6[1];
    SETUP_HEADER hdr;                                  //!!!!
    UINT8 _pad7[0x290 - 0x1f1 - sizeof(SETUP_HEADER)]; ///
    UINT32 edd_mbr_sig_buffer[16];                     //
    E820_ENTRY e820_table[128];                        //!!!!!
    UINT8 _pad8[48];                                   ///
    UINT8 eddbuf[0x1ec];                               //
    UINT8 _pad9[276];                                  //
} BOOT_PARAMS;

#endif