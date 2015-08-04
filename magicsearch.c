#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include "sorting.h"

void vmalloc_test(void);
void file_test(void);

int __init init_magicsearch(void)
{
	printk("[KERNEL MESSAGE] HELLO WORLD \n");
	vmalloc_test();
	file_test();
	return 0;
}

void __exit exit_magicsearch(void)
{
	printk("[KERNEL MESSAGE] goodbye\n");
}

void vmalloc_test(void)
{
	int *ptr_int;
	int ndx;

	ptr_int = (int *)vmalloc(10 * sizeof(int));
	
	for(ndx = 0; ndx < 10; ndx++)
	{
		ptr_int[ndx] = 2007 + ndx;
		printk("[KERNEL MESSAGE] %d\n", ptr_int[ndx]);
	}

	vfree(ptr_int);
}

void file_test(void)
{
	struct file *f;
	int test[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

	f = filp_open("/home/hye/project/modulecustom/test.txt", O_RDONLY, 0);
	if(f == NULL)
		printk("[KERNEL MESSAGE] file open error!\n");
	else
		printk("[KERNEL MESSAGE] file open success!\n");
	filp_close(f, NULL);

	quickSort2(test, 3, 4);
}

module_init(init_magicsearch);
module_exit(exit_magicsearch);
MODULE_LICENSE("GPL");
