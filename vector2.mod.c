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
	{ 0x4fb006bc, __VMLINUX_SYMBOL_STR(kmem_cache_alloc) },
	{ 0x5249c5f0, __VMLINUX_SYMBOL_STR(unregister_syscall) },
	{ 0x7c05ac52, __VMLINUX_SYMBOL_STR(overriddenMkdir) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0xe1ebe73e, __VMLINUX_SYMBOL_STR(overriddenRmdir) },
	{ 0x75a1d9d, __VMLINUX_SYMBOL_STR(overriddenClose) },
	{ 0x79b6546e, __VMLINUX_SYMBOL_STR(register_syscall) },
	{ 0x6a97bb65, __VMLINUX_SYMBOL_STR(loggedWrite) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=vector1";


MODULE_INFO(srcversion, "88B239B04F40EF1E9A6E6B9");
