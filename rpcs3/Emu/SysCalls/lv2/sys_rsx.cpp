#include "stdafx.h"
#include "Utilities/Log.h"
#include "Emu/Memory/Memory.h"
#include "Emu/System.h"
#include "Emu/SysCalls/SysCalls.h"
#include "sys_rsx.h"

SysCallBase sys_rsx("sys_rsx");

s32 sys_rsx_device_open()
{
	sys_rsx.Todo("sys_rsx_device_open()");
	return CELL_OK;
}

s32 sys_rsx_device_close()
{
	sys_rsx.Todo("sys_rsx_device_close()");
	return CELL_OK;
}

/*
 * lv2 SysCall 668 (0x29C): sys_rsx_memory_allocate
 * @param mem_handle (OUT): Context / ID, which is used by sys_rsx_memory_free to free allocated memory.
 * @param mem_addr (OUT): Returns the local memory base address, usually 0xC0000000.
 * @param size (IN): Local memory size. E.g. 0x0F900000 (249 MB).
 * @param flags (IN): E.g. Immediate value passed in cellGcmSys is 8.
 * @param a5 (IN): E.g. Immediate value passed in cellGcmSys is 0x00300000 (3 MB?).
 * @param a6 (IN): E.g. Immediate value passed in cellGcmSys is 16. 
 * @param a7 (IN): E.g. Immediate value passed in cellGcmSys is 8.
 */
s32 sys_rsx_memory_allocate(mem32_t mem_handle, mem32_t mem_addr, u32 size, u64 flags, u64 a5, u64 a6, u64 a7)
{
	sys_rsx.Todo("sys_rsx_memory_allocate(mem_handle_addr=0x%x, local_mem_addr=0x%x, size=0x%x, flags=0x%x, a5=%d, a6=%d, a7=%d)",
		mem_handle.GetAddr(), mem_addr.GetAddr(), size, flags, a5, a6, a7);
	return CELL_OK;
}

/*
 * lv2 SysCall 669 (0x29D): sys_rsx_memory_free
 * @param mem_handle (OUT): Context / ID, for allocated local memory generated by sys_rsx_memory_allocate
 */
s32 sys_rsx_memory_free(u32 mem_handle)
{
	sys_rsx.Todo("sys_rsx_memory_free(mem_handle=%d)", mem_handle);
	return CELL_OK;
}

/*
 * lv2 SysCall 670 (0x29E): sys_rsx_context_allocate
 * @param context_id (OUT): RSX_context, E.g. 0x55555555 (in vsh.self)
 * @param lpar_dma_control (OUT): Control register area. E.g. 0x60100000 (in vsh.self)
 * @param lpar_driver_info (OUT): RSX data like frequencies, sizes, version... E.g. 0x60200000 (in vsh.self)
 * @param lpar_reports (OUT): Report data area. E.g. 0x60300000 (in vsh.self)
 * @param mem_ctx (IN): mem_ctx given by sys_rsx_memory_allocate
 * @param system_mode (IN):
 */
s32 sys_rsx_context_allocate(mem32_t context_id, mem32_t lpar_dma_control, mem32_t lpar_driver_info, mem32_t lpar_reports, u64 mem_ctx, u64 system_mode)
{
	sys_rsx.Todo("sys_rsx_context_allocate(context_id_addr=0x%x, lpar_dma_control_addr=0x%x, lpar_driver_info_addr=0x%x, lpar_reports_addr=0x%x, mem_ctx=0x%x, system_mode=0x%x)",
		context_id.GetAddr(), lpar_dma_control.GetAddr(), lpar_driver_info.GetAddr(), lpar_reports.GetAddr(), mem_ctx, system_mode);

	return CELL_OK;
}

/*
 * lv2 SysCall 671 (0x29F): sys_rsx_context_free
 * @param a1 (IN): RSX_context generated by sys_rsx_context_allocate to free the context.
 */
s32 sys_rsx_context_free(u32 context_id)
{
	sys_rsx.Todo("sys_rsx_context_free(context_id=%d)", context_id);
	return CELL_OK;
}

s32 sys_rsx_context_iomap()
{
	sys_rsx.Todo("sys_rsx_context_iomap()");
	return CELL_OK;
}

s32 sys_rsx_context_iounmap()
{
	sys_rsx.Todo("sys_rsx_context_iounmap()");
	return CELL_OK;
}

/*
 * lv2 SysCall 674 (0x2A2): sys_rsx_context_attribute
 * @param context_id (IN): RSX context, e.g. 0x55555555
 * @param package_id (IN): 
 * @param a3 (IN): 
 * @param a4 (IN): 
 * @param a5 (IN): 
 * @param a6 (IN): 
 */
s32 sys_rsx_context_attribute(s32 context_id, u32 package_id, u64 a3, u64 a4, u64 a5, u64 a6)
{
	sys_rsx.Todo("sys_rsx_context_attribute(context_id=0x%x, package_id=0x%x, a3=%llu, a4=%llu, a5=%llu, a6=%llu)",
		context_id, package_id, a3, a4, a5, a6);

	switch(package_id)
	{
	case 0x001: // ?
		break;

	case 0x101: // ?
		break;

	case 0x104: // Display buffer
		break;

	case 0x10a: // ?
		break;

	case 0x300: // Tiles
		break;

	case 0x301: // Depth-buffer (Z-cull)
		break;

	default:
		break;
	}
	return CELL_OK;
}

/*
 * lv2 SysCall 675 (0x2A3): sys_rsx_device_map
 * @param a1 (OUT): For example: In vsh.self it is 0x60000000, global semaphore. For a game it is 0x40000000.
 * @param a2 (OUT): Unused?
 * @param dev_id (IN): An immediate value and always 8.
 */
s32 sys_rsx_device_map(mem32_t a1, mem32_t a2, u32 dev_id)
{
	sys_rsx.Todo("sys_rsx_device_map(a1_addr=0x%x, a2_addr=0x%x, a3=%d)", a1.GetAddr(), a2.GetAddr(), dev_id);

	if (dev_id > 15) {
		// TODO: Throw RSX error
		return CELL_EINVAL;
	}

	if (dev_id == 0 || dev_id > 8) {
		// TODO: lv1 related so we may ignore it.
		// if (something) { return CELL_EPERM; }
	}

	return CELL_OK;
}

/*
 * lv2 SysCall 676 (0x2A4): sys_rsx_device_unmap
 * @param dev_id (IN): An immediate value and always 8.
 */
s32 sys_rsx_device_unmap(u32 dev_id)
{
	sys_rsx.Todo("sys_rsx_device_unmap(a1=%d)", dev_id);
	return CELL_OK;
}

s32 sys_rsx_attribute()
{
	sys_rsx.Todo("sys_rsx_attribute()");
	return CELL_OK;
}
