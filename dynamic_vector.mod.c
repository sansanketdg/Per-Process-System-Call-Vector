#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xeecb04c6, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x285cbfdf, __VMLINUX_SYMBOL_STR(blockedOpen) },
	{ 0x294cc331, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0xd2b09ce5, __VMLINUX_SYMBOL_STR(__kmalloc) },
	{ 0xe11de68e, __VMLINUX_SYMBOL_STR(check_any_current_vector) },
	{ 0xc738e390, __VMLINUX_SYMBOL_STR(overriddenOpen) },
	{ 0x6ba605e9, __VMLINUX_SYMBOL_STR(overriddenLink) },
	{ 0x754d539c, __VMLINUX_SYMBOL_STR(strlen) },
	{ 0x91715312, __VMLINUX_SYMBOL_STR(sprintf) },
	{ 0xf4f97504, __VMLINUX_SYMBOL_STR(current_task) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x7e061af7, __VMLINUX_SYMBOL_STR(register_dynamic_syscall) },
	{ 0x61651be, __VMLINUX_SYMBOL_STR(strcat) },
	{ 0x4fb006bc, __VMLINUX_SYMBOL_STR(kmem_cache_alloc) },
	{ 0x5249c5f0, __VMLINUX_SYMBOL_STR(unregister_syscall) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0x75a1d9d, __VMLINUX_SYMBOL_STR(overriddenClose) },
	{ 0x6a97bb65, __VMLINUX_SYMBOL_STR(loggedWrite) },
	{ 0xc243413f, __VMLINUX_SYMBOL_STR(overriddenRead) },
	{ 0x390b20a8, __VMLINUX_SYMBOL_STR(overriddenWrite) },
	{ 0xa35fd3c7, __VMLINUX_SYMBOL_STR(try_module_get) },
	{ 0x477e8dad, __VMLINUX_SYMBOL_STR(overriddenUnlink) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=vector1";


MODULE_INFO(srcversion, "C3BC74EA64E5361ADF7642E");
