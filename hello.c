#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

int __init init_hello(void)
{
	printk("[KERNEL MESSAGE] HELLO WORLD \n");
	return 0;
}

void __exit exit_hello(void)
{
	printk(KERN_ALERT "GOODBYE WORLD\n");
}

module_init(init_hello);
module_exit(exit_hello);
MODULE_LICENSE("GPL");
