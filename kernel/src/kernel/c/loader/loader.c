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

char block[500] = {'\0'}; /* TODO remove */

void load_elf(void * const data)
{
	el_ctx ctx;
	ctx.user_param = data;
	ctx.pread = fpread;

	el_init(&ctx);

	ctx.base_load_vaddr = ctx.base_load_paddr = (uintptr_t) block;
	el_load(&ctx, alloccb);
	el_relocate(&ctx);
}
