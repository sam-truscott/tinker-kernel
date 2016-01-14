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
#include "memory/mem_pool.h"
#if defined(ELF_LOAD_DEBUGGING)
#include "console/print_out.h"
#endif

typedef struct loader_t
{
	mem_pool_info_t * pool;
	proc_list_t * list;
} loader_internal_t;

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

loader_t * loader_create(
		mem_pool_info_t * const pool,
		proc_list_t * const list)
{
	loader_t * load = (loader_t*)mem_alloc(pool, sizeof(loader_t));
	if (load)
	{
		util_memset(load, 0, sizeof(loader_t));
		load->pool = pool;
		load->list = list;
	}
	return load;
}

void load_elf(
		loader_t * const loader,
		const void * const data)
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
	tinker_mempart_t * first_part = NULL;
	tinker_mempart_t * current_part = first_part;
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
				tinker_mempart_t * new_part = (tinker_mempart_t*)mem_alloc(loader->pool, sizeof(tinker_mempart_t));
				if (new_part)
				{
					if (current_part)
					{
						current_part->next = new_part;
					}
					current_part = new_part;
					util_memset(current_part, 0, sizeof(tinker_mempart_t));
					current_part->real = addr.p_paddr;
					current_part->virt = addr.p_vaddr;
					current_part->size = addr.p_memsz;
					current_part->mem_type = MEM_RANDOM_ACCESS_MEMORY;
					current_part->priv = MEM_USER_ACCESS;
					// TODO add exec
					/*
					if (addr.p_flags & PF_X)
					{
						current_part->access = MEM_READ_WRITE;
					}
					*/
					if (addr.p_flags & PF_W)
					{
						current_part->access = MEM_READ_WRITE;
					}
					else if (addr.p_flags & PF_R)
					{
						current_part->access = MEM_READ_ONLY;
					}
					else
					{
						current_part->access = MEM_NO_ACCESS;
					}
					current_part->next = NULL;
					if (first_part == NULL)
					{
						first_part = current_part;
					}
				}
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
	if (first_part != NULL)
	{
		process_t * proc = NULL;
		tinker_meminfo_t memory =
		{
				.stack_size = 4096,
				.heap_size = 4096,
				.first_part = first_part
		};
		proc_create_process(
				loader->list,
				"x",
				"main",
				(thread_entry_point*)(ctx.ehdr.e_entry + (uint32_t)data),
				128,
				&memory,
				0,
				&proc);
		current_part = first_part;
		while (current_part)
		{
			tinker_mempart_t * const next = current_part->next;
			mem_free(loader->pool, current_part);
			current_part = next;
		}
	}
}
