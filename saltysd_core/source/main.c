#include <switch_min.h>

#include <inttypes.h>
#include <dirent.h>
#include <switch_min/kernel/ipc.h>
#include <switch_min/runtime/threadvars.h>
#include <stdlib.h>

#include "useful.h"
#include "saltysd_ipc.h"
#include "saltysd_core.h"
#include "saltysd_dynamic.h"

#include "bm.h"

u32 __nx_applet_type = AppletType_None;

static char g_heap[0x10000];

extern void __nx_exit_clear(void* ctx, Handle thread, void* addr);
extern void elf_trampoline(void* context, Handle thread, void* func);
void* __stack_tmp;

Handle orig_main_thread;
void* orig_ctx;

Handle sdcard;
size_t elf_area_size = 0;

ThreadVars vars_orig;
ThreadVars vars_mine;

uint64_t tid = 0;

void __libnx_init(void* ctx, Handle main_thread, void* saved_lr)
{
	extern char* fake_heap_start;
	extern char* fake_heap_end;

	fake_heap_start = &g_heap[0];
	fake_heap_end   = &g_heap[sizeof g_heap];

	orig_ctx = ctx;
	orig_main_thread = main_thread;
	
	// Hacky TLS stuff, TODO: just stop using libnx t b h
	vars_mine.magic = 0x21545624;
	vars_mine.handle = main_thread;
	vars_mine.thread_ptr = NULL;
	vars_mine.reent = _impure_ptr;
	vars_mine.tls_tp = (void*)malloc(0x1000);
	vars_orig = *getThreadVars();
	*getThreadVars() = vars_mine;
}

void __attribute__((weak)) __libnx_exit(int rc)
{
	fsdevUnmountAll();
	
	// Restore TLS stuff
	*getThreadVars() = vars_orig;
	
	u64 addr = SaltySDCore_getCodeStart();

	__nx_exit_clear(orig_ctx, orig_main_thread, (void*)addr);
}

u64  g_heapAddr;
size_t g_heapSize;

void SaltySDCore_LoadPatches (bool Aarch64) {
	char* tmp4 = malloc(0x100);
	char* tmp2 = malloc(0x100);
	DIR *d;
	struct dirent *dir;
	char* instr;
	
	SaltySDCore_printf("SaltySD Patcher: Searching patches in dir '/'...\n");
	
	snprintf(tmp4, 0x100, "sdmc:/SaltySD/patches/");

	d = opendir(tmp4);
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			char *dot = strrchr(dir->d_name, '.');
			if (Aarch64 == true) {
				if (dot && !strcmp(dot, ".asm64")) {
					snprintf(tmp2, 0x100, "%s%s", tmp4, dir->d_name);
					SaltySDCore_printf("SaltySD Patcher: Found %s\n", dir->d_name);
					FILE* patch = fopen(tmp2, "rb");
					fseek(patch, 0, SEEK_END);
					uint32_t size = ftell(patch);
					fseek(patch, 0, SEEK_SET);
					//Test if filesize is valid
					float val = (float)size;
					val = val / 4;
					uint32_t trunc = (uint32_t)val;
					if ((float)trunc != val) {
						fclose(patch);
						SaltySDCore_printf("%s doesn't have valid filesize...\n", tmp2);
						break;
					}
					fread(&instr, 4, trunc, patch);
					fclose(patch);
					char* filename = dir->d_name;
					uint8_t namelen = strlen(filename);
					filename[namelen - 6] = 0;
					uint64_t position = SaltySDCore_FindSymbol(filename);
					SaltySDCore_printf("SaltySD Patcher: Symbol Position: %016llx\n", position);
					SaltySD_Memcpy(position, (uint64_t)&instr, 4*trunc);
				}
			}
			else {
				if (dot && !strcmp(dot, ".asm32")) {
					snprintf(tmp2, 0x100, "%s%s", tmp4, dir->d_name);
					SaltySDCore_printf("SaltySD Patcher: Found %s\n", dir->d_name);
					FILE* patch = fopen(tmp2, "rb");
					fseek(patch, 0, SEEK_END);
					uint32_t size = ftell(patch);
					fseek(patch, 0, SEEK_SET);
					//Test if filesize is valid
					float val = (float)size;
					val = val / 4;
					uint32_t trunc = (uint32_t)val;
					if ((float)trunc != val) {
						fclose(patch);
						SaltySDCore_printf("%s doesn't have valid filesize...\n", tmp2);
						break;
					}
					fread(&instr, 4, trunc, patch);
					fclose(patch);
					char* filename = dir->d_name;
					uint8_t namelen = strlen(filename);
					filename[namelen - 6] = 0;
					uint64_t position = SaltySDCore_FindSymbol(filename);
					SaltySDCore_printf("SaltySD Patcher: Symbol Position: %0l6llx$$$\n", position);
					SaltySD_Memcpy(position, (uint64_t)&instr, 4*trunc);
				}
			}
		}
		closedir(d);
	}

	svcGetInfo(&tid, 18, CUR_PROCESS_HANDLE, 0);
	
	SaltySDCore_printf("SaltySD Patcher: Searching patches in dir '/%016llx'...\n", tid);
	
	snprintf(tmp4, 0x100, "sdmc:/SaltySD/patches/%016" PRIx64 "/", tid);

	d = opendir(tmp4);
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			char *dot = strrchr(dir->d_name, '.');
			if (Aarch64 == true) {
				if (dot && !strcmp(dot, ".asm64")) {
					snprintf(tmp2, 0x100, "%s%s", tmp4, dir->d_name);
					SaltySDCore_printf("SaltySD Patcher: Found %s\n", dir->d_name);
					FILE* patch = fopen(tmp2, "rb");
					fseek(patch, 0, SEEK_END);
					uint32_t size = ftell(patch);
					fseek(patch, 0, SEEK_SET);
					//Test if filesize is valid
					float val = (float)size;
					val = val / 4;
					uint32_t trunc = (uint32_t)val;
					if ((float)trunc != val) {
						fclose(patch);
						SaltySDCore_printf("%s doesn't have valid filesize...\n", tmp2);
						break;
					}
					fread(&instr, 4, trunc, patch);
					fclose(patch);
					char* filename = dir->d_name;
					uint8_t namelen = strlen(filename);
					filename[namelen - 6] = 0;
					uint64_t position = SaltySDCore_FindSymbol(filename);
					SaltySDCore_printf("SaltySD Patcher: Symbol Position: %016llx\n", position);
					SaltySD_Memcpy(position, (uint64_t)&instr, 4*trunc);
				}
			}
			else {
				if (dot && !strcmp(dot, ".asm32")) {
					snprintf(tmp2, 0x100, "%s%s", tmp4, dir->d_name);
					SaltySDCore_printf("SaltySD Patcher: Found %s\n", dir->d_name);
					FILE* patch = fopen(tmp2, "rb");
					fseek(patch, 0, SEEK_END);
					uint32_t size = ftell(patch);
					fseek(patch, 0, SEEK_SET);
					//Test if filesize is valid
					float val = (float)size;
					val = val / 4;
					uint32_t trunc = (uint32_t)val;
					if ((float)trunc != val) {
						fclose(patch);
						SaltySDCore_printf("%s doesn't have valid filesize...\n", tmp2);
						break;
					}
					fread(&instr, 4, trunc, patch);
					fclose(patch);
					char* filename = dir->d_name;
					uint8_t namelen = strlen(filename);
					filename[namelen - 6] = 0;
					uint64_t position = SaltySDCore_FindSymbol(filename);
					SaltySDCore_printf("SaltySD Patcher: Symbol Position: %0l6llx$$$\n", position);
					SaltySD_Memcpy(position, (uint64_t)&instr, 4*trunc);
				}
			}
		}
		closedir(d);
	}
	free(tmp4);
	free(tmp2);
	
	return;
}

void setupELFHeap(void)
{
	void* addr = NULL;
	Result rc = 0;

	rc = svcSetHeapSize(&addr, ((elf_area_size+0x200000) & 0xffe00000));

	if (rc || addr == NULL)
	{
		debug_log("SaltySD Bootstrap: svcSetHeapSize failed with err %x\n", rc);
	}

	g_heapAddr = (u64)addr;
	g_heapSize = ((elf_area_size+0x200000) & 0xffe00000);
}

u64 find_next_elf_heap()
{
	u64 addr = g_heapAddr;
	while (1)
	{
		MemoryInfo info;
		u32 pageinfo;
		Result ret = svcQueryMemory(&info, &pageinfo, addr);
		
		if (info.perm == Perm_Rw)
			return info.addr;

		addr = info.addr + info.size;
		
		if (!addr || ret) break;
	}
	
	return 0;
}

extern void _start();

void SaltySDCore_RegisterExistingModules()
{
	u64 addr = 0;
	while (1)
	{
		MemoryInfo info;
		u32 pageinfo;
		Result ret = svcQueryMemory(&info, &pageinfo, addr);
		
		if (info.perm == Perm_Rx)
		{
			SaltySDCore_RegisterModule((void*)info.addr);
			u64 compaddr = (u64)info.addr;
			if ((u64*)compaddr != (u64*)_start)
				SaltySDCore_RegisterBuiltinModule((void*)info.addr);
		}

		addr = info.addr + info.size;
		
		if (!addr || ret) break;
	}
	
	return;
}

Result svcSetHeapSizeIntercept(u64 *out, u64 size)
{
	static bool Initialized = false;
	Result ret = 1;
	if (!Initialized)
		size += ((elf_area_size+0x200000) & 0xffe00000);
	ret = svcSetHeapSize((void*)out, size);
	
	//SaltySDCore_printf("SaltySD Core: svcSetHeapSize intercept %x %llx %llx\n", ret, *out, size+((elf_area_size+0x200000) & 0xffe00000));
	
	if (!ret && !Initialized)
	{
		*out += ((elf_area_size+0x200000) & 0xffe00000);
		Initialized = true;
	}
	
	return ret;
}

Result svcGetInfoIntercept (u64 *out, u64 id0, Handle handle, u64 id1)	
{	
	Result ret = svcGetInfo(out, id0, handle, id1);	

	//SaltySDCore_printf("SaltySD Core: svcGetInfo intercept %p (%llx) %llx %x %llx ret %x\n", out, *out, id0, handle, id1, ret);	

	if (id0 == 6 && id1 == 0 && handle == 0xffff8001)	
	{	
		*out -= elf_area_size;
	}		

	return ret;	
}

void SaltySDCore_PatchSVCs()
{
	Result ret;
	static u8 orig_1[0x8] = {0xE0, 0x0F, 0x1F, 0xF8, 0x21, 0x00, 0x00, 0xD4}; //STR [sp, #-0x10]!; SVC #0x1
	static u8 orig_2[0x8] = {0xE0, 0x0F, 0x1F, 0xF8, 0x21, 0x05, 0x00, 0xD4}; //STR [sp, #-0x10]!; SVC #0x29
	const u8 nop[0x4] = {0x1F, 0x20, 0x03, 0xD5}; // NOP
	static u8 patch[0x10] = {0x44, 0x00, 0x00, 0x58, 0x80, 0x00, 0x1F, 0xD6, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F, 0xF0}; // LDR X4 #8; BR X4; ADRP X15, #0x1FE03000; ADRP X15, #0x1FE03000
	u64 dst_1 = SaltySDCore_findCode(orig_1, 8);
	u64 dst_2 = SaltySDCore_findCode(orig_2, 8);
	
	if (!dst_1 || !dst_2)
	{
		SaltySDCore_printf("SaltySD Core: Failed to find svcSetHeapSize! %llx\n", dst_1);
		return;
	}

	*(u64*)&patch[8] = (u64)svcSetHeapSizeIntercept;
	if (dst_1 & 4)
	{
		ret = SaltySD_Memcpy(dst_1, (u64)nop, 0x4);
		if (ret)
		{
			debug_log("svcSetHeapSize memcpy failed!\n");
		}
		else
		{
			ret = SaltySD_Memcpy(dst_1+4, (u64)patch, 0x10);
		}
	}
	else
	{
		ret = SaltySD_Memcpy(dst_1, (u64)patch, 0x10);
	}
	if (ret) debug_log("svcSetHeapSize memcpy failed!\n");
	
	*(u64*)&patch[8] = (u64)svcGetInfoIntercept;	
	if (dst_2 & 4)	
	{	
		ret = SaltySD_Memcpy(dst_2, (u64)nop, 0x4);	
		if (ret)	
		{	
			debug_log("svcSetHeapSize memcpy failed!\n");	
		}	
		else	
		{	
			ret = SaltySD_Memcpy(dst_2+4, (u64)patch, 0x10);	
		}	
	}	
	else	
	{	
		ret = SaltySD_Memcpy(dst_2, (u64)patch, 0x10);	
	}	
	if (ret) debug_log("svcSetHeapSize memcpy failed!\n");
}

void** SaltySDCore_LoadPluginsInDir(char* path, void** entries, size_t* num_elfs)
{
	char* tmp = malloc(0x100);
	DIR *d;
	struct dirent *dir;

	SaltySDCore_printf("SaltySD Core: Searching plugin dir `%s'...\n", path);
	
	snprintf(tmp, 0x100, "sdmc:/SaltySD/plugins/%s", path);

	d = opendir(tmp);
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			char *dot = strrchr(dir->d_name, '.');
			if (dot && !strcmp(dot, ".elf"))
			{
				u64 elf_addr, elf_size;
				setupELFHeap();
				snprintf(tmp, 0x100, "%s%s", path, dir->d_name);
				SaltySD_LoadELF(find_next_elf_heap(), &elf_addr, &elf_size, tmp);
				*num_elfs = *num_elfs + 1;
				entries = realloc(entries, *num_elfs * sizeof(void*));
				entries[*num_elfs-1] = (void*)elf_addr;

				SaltySDCore_RegisterModule(entries[*num_elfs-1]);
				elf_area_size += elf_size;
			}
		}
		closedir(d);
	}
	free(tmp);
	
	return entries;
}

void SaltySDCore_LoadPlugins()
{
	// Load plugin ELFs
	char* tmp3 = malloc(0x20);

	void** entries = NULL;
	size_t num_elfs = 0;
	
	entries = SaltySDCore_LoadPluginsInDir("", entries, &num_elfs);
	snprintf(tmp3, 0x20, "%016" PRIx64 "/", tid);
	entries = SaltySDCore_LoadPluginsInDir(tmp3, entries, &num_elfs);
	
	for (int i = 0; i < num_elfs; i++)
	{
		SaltySDCore_DynamicLinkModule(entries[i]);
		elf_trampoline(orig_ctx, orig_main_thread, entries[i]);
	}
	free(tmp3);
	if (num_elfs) free(entries);
	else SaltySDCore_printf("SaltySD Core: Plugins not detected...\n");
	
	return;
}

int main(int argc, char *argv[])
{
	Result ret;

	debug_log("SaltySD Core: waddup\n");
	SaltySDCore_RegisterExistingModules();
	
	SaltySD_Init();

	SaltySDCore_printf("SaltySD Core: restoring code...\n");
	ret = SaltySD_Restore();
	if (ret) goto fail;
	
	ret = SaltySD_GetSDCard(&sdcard);
	if (ret) goto fail;

	SaltySDCore_PatchSVCs();
	SaltySDCore_LoadPatches(true);
	
	Result exc = SaltySD_Exception();
	if (exc == 0x0) SaltySDCore_LoadPlugins();
	else SaltySDCore_printf("SaltySD Core: Detected exception title, aborting loading plugins...\n");

	ret = SaltySD_Deinit();
	if (ret) goto fail;

	__libnx_exit(0);

fail:
	debug_log("SaltySD Core: failed with retcode %x\n", ret);
	SaltySDCore_printf("SaltySD Core: failed with retcode %x\n", ret);
	__libnx_exit(0);
}
