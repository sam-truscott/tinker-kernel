/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2016] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#include "loader/loader.h"
#include "elfload.h"
#include "utils/util_memcpy.h"
#include "utils/util_memset.h"
#include "process/process.h"
#if defined(ELF_LOAD_DEBUGGING)
#include "console/print_out.h"
#endif

static bool fpread(el_ctx *ctx, void *dest, size_t nb, size_t offset)
{
	util_memcpy(dest, ((uint8_t*)ctx->user_param) + offset, nb);
	return true;
}

static void *alloccb(
    el_ctx *ctx,
    Elf_Addr phys,
    Elf_Addr virt,
    Elf_Addr size)
{
	(void)ctx;
	(void)phys;
	(void)size;
    return (void*) virt;
}

void load_elf(proc_list_t * const list, const void * const data)
{
	el_ctx ctx;
	el_status status;
	ctx.user_param = (void*)data;
	ctx.pread = fpread;

	status = el_init(&ctx);
	if (EL_OK != status)
	{
#if defined(ELF_LOAD_DEBUGGING)
		debug_print("Failed to init ELF context: %d\n", status);
#endif
		return;
	}

	ctx.base_load_vaddr = ctx.base_load_paddr = (uintptr_t) data;
	status = el_load(&ctx, alloccb);
	if (EL_OK != status)
	{
#if defined(ELF_LOAD_DEBUGGING)
		debug_print("Failed to load ELF: %d\n", status);
#endif
		return;
	}

	Elf_Phdr addr;
	unsigned ctr = 0;
	while (EL_OK == status)
	{
		util_memset(&addr, 0, sizeof(Elf_Phdr));
		status = el_findphdr(&ctx, &addr, PT_LOAD, &ctr);
		if (EL_OK == status)
		{
			if (ctr == 0xffffffffu)
			{
				break;
			}
			else
			{
				ctr++;
#if defined(ELF_LOAD_DEBUGGING)
				debug_print("PT_LOAD: %d: type %d, offset 0x%8X, virt 0x%8X, phy 0x%8X sz %d align %d flags %d\n",
						ctr,
						addr.p_type,
						addr.p_offset,
						addr.p_vaddr,
						addr.p_paddr,
						addr.p_memsz,
						addr.p_align,
						addr.p_flags);
				if (addr.p_flags & PF_X)
				{
					debug_print("PT_LOAD: %d: Executable\n", ctr);
				}
				if (addr.p_flags & PF_W)
				{
					debug_print("PT_LOAD: %d: Writable\n", ctr);
				}
				if (addr.p_flags & PF_R)
				{
					debug_print("PT_LOAD: %d: Readable\n", ctr);
				}
#endif
				// TODO segment
				process_t * proc = NULL;
				tinker_meminfo_t memory =
				{
						.text_start = ((uint32_t)data) + addr.p_offset,
						.text_size = addr.p_memsz,
						.stack_size = 1024,
						.heap_size = 1024,
						.data_start = 0,
						.data_size = 0
				};
				proc_create_process(
						list,
						"x",
						"main",
						(thread_entry_point*)(ctx.ehdr.e_entry + (uint32_t)data),
						128,
						&memory,
						0,
						&proc);
			}
		}
		else
		{
#if defined(ELF_LOAD_DEBUGGING)
			debug_print("Failed to find next section: %d\n", status);
#endif
			break;
		}
	}
}
