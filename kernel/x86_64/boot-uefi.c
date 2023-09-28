#include <efi.h>
#include <efilib.h>
#include <elf.h>

#define MAX(a, b) (a < b ? b : a)
#define MIN(a, b) (a < b ? a : b)

#define align_up(x, align)	\
	((__typeof__(x))(((uintptr_t)(x)+((align)-1))&(~((align)-1))))

static void print_efi_memory_descriptor(EFI_MEMORY_DESCRIPTOR *d) {
    Print(
        L"%x %x %x %x\n",
        d->Type,
        d->PhysicalStart,
        d->NumberOfPages,
        d->Attribute
    );
}

// I don't know why, but gnu-efi's CopyMem didn't copy anything, so I use this memcpy
static void *memcpy(void *dst, const void *src, size_t n) {
    uint8_t *d = (uint8_t *) dst;
    const uint8_t *s = (const uint8_t *) src;
    while (n--)
        *d++ = *s++;
    return dst;
}

EFI_STATUS
EFIAPI
efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    InitializeLib(ImageHandle, SystemTable);

    // clear screen
    SIMPLE_TEXT_OUTPUT_INTERFACE *conout = ST->ConOut;
    uefi_call_wrapper(conout->ClearScreen, 1, conout);

    Print(L"Booting...\n");

    // print memory map
    UINTN num_entry, map_key, desc_size;
    UINT32 desc_version;

    EFI_MEMORY_DESCRIPTOR *map = LibMemoryMap(
        &num_entry,
        &map_key,
        &desc_size,
        &desc_version
    );
    ASSERT(map != NULL);

    Print(L"Type PhysicalStart NumberOfPages Attribute\n");
    for(UINTN i = 0 ; i < num_entry; i++) {
        EFI_MEMORY_DESCRIPTOR *d = (EFI_MEMORY_DESCRIPTOR *)((char *)map + desc_size * i);
        if(d->Type == EfiConventionalMemory)
            print_efi_memory_descriptor(d);
    }
    FreePool(map);

    EFI_STATUS status;

    // open root dir
    EFI_GUID    lip_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_LOADED_IMAGE_PROTOCOL *lip;
    status = uefi_call_wrapper(
        BS->OpenProtocol, 6, 
        ImageHandle, 
        &lip_guid,
        (void **) &lip,
        ImageHandle, 
        NULL,
        EFI_OPEN_PROTOCOL_GET_PROTOCOL
    );
    ASSERT(!EFI_ERROR(status));

    EFI_FILE_HANDLE root = LibOpenRoot(lip->DeviceHandle);
    ASSERT(root != NULL);

    // open file
    EFI_FILE_HANDLE fhandle;
    status = uefi_call_wrapper(
        root->Open,
        5, root, &fhandle, L"kernel.elf", EFI_FILE_MODE_READ, 0
    );
    ASSERT(!EFI_ERROR(status));

    // get file size
    EFI_FILE_INFO *info = LibFileInfo(fhandle);
    UINT64 fsize = info->FileSize;
    FreePool(info);

    // allocate pool
    Elf64_Ehdr *ehdr = AllocatePool(fsize);
    ASSERT(ehdr != NULL);
    
    // read file
    status = uefi_call_wrapper(
        fhandle->Read, 3, 
        fhandle, 
        &fsize, 
        ehdr
    );
    ASSERT(!EFI_ERROR(status));
    //DumpHex(4, 0, fsize, ehdr);

    // calcurate address range
    UINT64 start = 0, end = 0;
    for(Elf64_Half i = 0; i < ehdr->e_phnum; i++) {
        Elf64_Phdr *phdr = (Elf64_Phdr *)((char *)ehdr + ehdr->e_phoff + ehdr->e_phentsize * i);
        if(phdr->p_type != PT_LOAD) continue;

        if(!start) {
            start = phdr->p_paddr;
            end = phdr->p_paddr + phdr->p_memsz;
        }

        start = MIN(start, phdr->p_paddr);
        end = MAX(end, phdr->p_paddr + phdr->p_memsz);
    }

    EFI_PHYSICAL_ADDRESS base = 0x100000;
    
    // allocate pool
    UINTN num_pages = align_up(end - start, 0x1000) / 0x1000;
    Print(L"start = %x, end = %x, num_pages = %x\n", start, end, num_pages);

    status = uefi_call_wrapper(
        BS->AllocatePages, 4, 
        AllocateAddress, 
        EfiLoaderData,
        num_pages,
        &base
    );
    ASSERT(!EFI_ERROR(status));
    Print(L"base = %x\n", base);

    // init
    ZeroMem((VOID *)base, num_pages);

    // copy LOAD segments
    for(Elf64_Half i = 0; i < ehdr->e_phnum; i++) {
        Elf64_Phdr *phdr = (Elf64_Phdr *)((char *)ehdr + ehdr->e_phoff + ehdr->e_phentsize * i);
        if(phdr->p_type != PT_LOAD) continue;
        //DumpHex(4, 0, phdr->p_filesz, (char *)ehdr + phdr->p_offset);
        memcpy(
            (VOID *)phdr->p_paddr, 
            (char *)ehdr + phdr->p_offset, 
            phdr->p_filesz
        );
        //DumpHex(4, 0, phdr->p_memsz, (VOID *)phdr->p_paddr);
    }

    // get entry point
    typedef void entry_t (void);
    entry_t *e_entry = (entry_t *)ehdr->e_entry;
    Print(L"entry = %x\n", e_entry);

    // free pool
    FreePool(ehdr);

    // exit boot services
    LibMemoryMap(
        &num_entry,
        &map_key,
        &desc_size,
        &desc_version
    );
    status = uefi_call_wrapper(
        BS->ExitBootServices, 2, 
        ImageHandle,
        map_key
    );
    ASSERT(!EFI_ERROR(status));
    
    // jmp to entry
    __asm__ __volatile__ ("jmp %0\n" :: "r"(e_entry));

    // never reach here
    while(1);

    return EFI_SUCCESS;
}