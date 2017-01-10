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
	{ 0x6bc3fbc0, __VMLINUX_SYMBOL_STR(__unregister_chrdev) },
	{ 0xf41e854, __VMLINUX_SYMBOL_STR(__mutex_init) },
	{ 0x4f9d57d4, __VMLINUX_SYMBOL_STR(__register_chrdev) },
	{ 0x374db6f6, __VMLINUX_SYMBOL_STR(add_vector_from_bitmap) },
	{ 0xf60e518a, __VMLINUX_SYMBOL_STR(pid_task) },
	{ 0x910c3e3a, __VMLINUX_SYMBOL_STR(find_vpid) },
	{ 0xbea6d7c7, __VMLINUX_SYMBOL_STR(get_vector_address) },
	{ 0xc671e369, __VMLINUX_SYMBOL_STR(_copy_to_user) },
	{ 0xf2491e43, __VMLINUX_SYMBOL_STR(show_all_vectors) },
	{ 0x2fbfedc2, __VMLINUX_SYMBOL_STR(mutex_unlock) },
	{ 0x766c693f, __VMLINUX_SYMBOL_STR(module_put) },
	{ 0xb5419b40, __VMLINUX_SYMBOL_STR(_copy_from_user) },
	{ 0xa35fd3c7, __VMLINUX_SYMBOL_STR(try_module_get) },
	{ 0xd2b09ce5, __VMLINUX_SYMBOL_STR(__kmalloc) },
	{ 0x754d539c, __VMLINUX_SYMBOL_STR(strlen) },
	{ 0x8dc3e070, __VMLINUX_SYMBOL_STR(mutex_lock) },
	{ 0xc2d257bc, __VMLINUX_SYMBOL_STR(reduce_ref_count) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0xa71c1394, __VMLINUX_SYMBOL_STR(show_my_vector) },
	{ 0x4fb006bc, __VMLINUX_SYMBOL_STR(kmem_cache_alloc) },
	{ 0x294cc331, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0xf4f97504, __VMLINUX_SYMBOL_STR(current_task) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=dynamic_vector";


MODULE_INFO(srcversion, "4A36A3F7522C633ED6302B8");
