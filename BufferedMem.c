/* BufferedMem.c */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>

#include <linux/slab.h>
#include <asm/uaccess.h>

#define BUF_MEM_NAME		"BufferedMem"
#define BUF_MEM_MAJOR		270
#define BUF_LEN_LIMIT    	1024


// 
// Global variables
// 

// Buffer for copy_from_user() and copy_to_user() functions
static char temp[1024];

static int buf_len = 32;
module_param(buf_len, int, 0);
MODULE_PARM_DESC(buf_len, "Changeable buffer length");

// Buffer
static char buffer[BUF_LEN_LIMIT] = "";

static int read_size = 4;
module_param(read_size, int, 0);
MODULE_PARM_DESC(read_size, "Buffer's read size");


//
// Helper functions
//
void remove_from_buffer(int n, int logical_buf_size)
{
	int i;
	for (i = 0; i < logical_buf_size-n; i++)
		buffer[i] = buffer[n+i];
	for (i = logical_buf_size-n; i < buf_len; i++)
		buffer[i] = '\0';
}

void write_to_buffer(char *src)
{
	// Push to buffer; Stop pushing if buffer if full.
	int i;
	int start = strlen(buffer); // Starting position of writting place
	for (i = 0; (i < strlen(src)) && i < buf_len; i++)
		buffer[start+i] = src[i];
}


// 
// File operation functions
// 
int chr_open(struct inode *inode, struct file *filep)
{
	printk("This is Virtual Device BufferedMem, Opened\n");
	return 0;
}

ssize_t chr_write(struct file *filep, const char *buf, size_t count, loff_t *f_pos)
{
  int ret = copy_from_user(temp, buf, (buf_len>count)?count:buf_len);
	if (ret < 0)
		return ret;
	if (strlen(temp)==0)
		return 0;
	else if (strlen(buffer) + strlen(temp) <= buf_len)
	{
		write_to_buffer(temp);
		ret = strlen(buffer) + strlen(temp);
	}
	else if ((strlen(buf) <= buf_len -1) && 
			 (strlen(buffer) + strlen(temp) > buf_len))
	{
		remove_from_buffer(strlen(buffer) + strlen(temp) - (buf_len), buf_len);
		write_to_buffer(temp);
		ret = buf_len;
	}
	else
	{
		remove_from_buffer(buf_len, buf_len);
		write_to_buffer(&(temp[strlen(temp)-buf_len]));
		ret = buf_len;
	}
	return ret;
}

ssize_t chr_read(struct file *filep, char *buf, size_t count, loff_t *f_pos)
{
	int i;
	for (i = 0; i < read_size; i++)
		temp[i] = buffer[i];
	temp[read_size+1] = '\0';
	copy_to_user(buf, temp, read_size+1);
	remove_from_buffer(read_size, buf_len);
	return read_size;
}

int chr_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
	int diff;
	switch(cmd) {
	case 0: 
	  diff = ( (BUF_LEN_LIMIT > arg) ? arg : (BUF_LEN_LIMIT-1) ) - buf_len; 
	  if (diff < 0) 
	    remove_from_buffer(diff, buf_len);
	  buf_len = buf_len + diff;
	  printk("Buffer length changed to %d\n", buf_len); 
	  break;
	case 1: 
	  read_size = (buf_len > arg) ? arg : buf_len; 
	  printk("Reading size changed to %d\n", read_size); 
	  break;
	}
	return 0;
}

int chr_release(struct inode *inode, struct file *filep)
{
	printk("Virtual Device BufferedMem Release\n");
	return 0;
}


// 
// File operation struct
// 
struct file_operations chr_fops = 
{
	.owner				= THIS_MODULE,
	.unlocked_ioctl		= chr_ioctl,
	.write 				= chr_write,
	.read			    = chr_read,
	.open	 		    = chr_open, 
	/* .release             = chr_release, */
};


// 
// Module initializer and cleaner
// 
int my_init_module(void)
{
	int registration;
	printk("Device driver registered\n");
	registration = register_chrdev(BUF_MEM_MAJOR, BUF_MEM_NAME, &chr_fops);
	if (registration < 0)
		return registration;
	return 0;
}

void my_cleanup_module(void)
{
	printk("Device driver unregistered\n");
	unregister_chrdev(BUF_MEM_MAJOR, BUF_MEM_NAME);
}

module_init(my_init_module);
module_exit(my_cleanup_module);