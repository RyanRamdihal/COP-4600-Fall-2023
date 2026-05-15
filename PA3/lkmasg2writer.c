/**
 * File:	lkmasg1.c
 * Adapted for Linux 5.15 by: John Aedo
 * Class:	COP4600-SP23
 */

#include <linux/module.h>	  // Core header for modules.
#include <linux/device.h>	  // Supports driver model.
#include <linux/kernel.h>	  // Kernel header for convenient functions.
#include <linux/fs.h>		  // File-system support.
#include <linux/uaccess.h>	  // User access copy function support.
#include <linux/mutex.h>	  // Muex support.
#define DEVICE_NAME "lkmasg2writer" // Device name.
#define CLASS_NAME "char_write"	  ///< The device class -- this is a character device driver

#define BUFFER_SIZE 1024

MODULE_LICENSE("GPL");						 ///< The license type -- this affects available functionality
MODULE_AUTHOR("John Aedo");					 ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION("lkmasg2 Writer Module"); ///< The description -- see modinfo
MODULE_VERSION("0.1");						 ///< A version number to inform users

/**
 * Important variables that store data and keep track of relevant information.
 */
static int major_number;

static struct class *lkmasg1Class = NULL;	///< The device-driver class struct pointer
static struct device *lkmasg1Device = NULL; ///< The device-driver device struct pointer

/**
 * File operation state
 */
struct mutex lkmasg2_mutex;
EXPORT_SYMBOL(lkmasg2_mutex);
volatile size_t availableBuffer = 0;
EXPORT_SYMBOL(availableBuffer);
volatile size_t start = 0;
EXPORT_SYMBOL(start);
volatile char deviceBuffer[BUFFER_SIZE];
EXPORT_SYMBOL(deviceBuffer);

/**
 * Prototype functions for file operations.
 */
static int open(struct inode *, struct file *);
static int close(struct inode *, struct file *);
static ssize_t read(struct file *, char *, size_t, loff_t *);
static ssize_t write(struct file *, const char *, size_t, loff_t *);

/**
 * File operations structure and the functions it points to.
 */
static struct file_operations fops =
	{
		.owner = THIS_MODULE,
		.open = open,
		.release = close,
		.read = read,
		.write = write,
};

/**
 * Initializes module at installation
 */
int init_module(void)
{
	printk(KERN_INFO "lkmasg2 Writer: installing module.\n");

	// Allocate a major number for the device.
	major_number = register_chrdev(0, DEVICE_NAME, &fops);
	if (major_number < 0)
	{
		printk(KERN_ALERT "lkmasg2 Reader/Writer - Error encountered: could not register number.\n");
		return major_number;
	}
	printk(KERN_INFO "lkmasg2 Writer: registered correctly with major number %d\n", major_number);

	// Register the device class
	lkmasg1Class = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(lkmasg1Class))
	{ // Check for error and clean up if there is
		unregister_chrdev(major_number, DEVICE_NAME);
		printk(KERN_ALERT "lkmasg2 Reader/Writer - Error encountered: Failed to register device class\n");
		return PTR_ERR(lkmasg1Class); // Correct way to return an error on a pointer
	}
	printk(KERN_INFO "lkmasg2 Writer: device class registered correctly\n");

	// Register the device driver
	lkmasg1Device = device_create(lkmasg1Class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
	if (IS_ERR(lkmasg1Device))
	{								 // Clean up if there is an error
		class_destroy(lkmasg1Class); // Repeated code but the alternative is goto statements
		unregister_chrdev(major_number, DEVICE_NAME);
		printk(KERN_ALERT "lkmasg2 Reader/Writer - Error encountered: Failed to create the device\n");
		return PTR_ERR(lkmasg1Device);
	}
	printk(KERN_INFO "lkmasg2 Writer: device class created correctly\n"); // Made it! device was initialized

	// Initialize mutex
	mutex_init(&lkmasg2_mutex);

	printk(KERN_INFO "lkmasg2 Writer module successfully installed");

	return 0;
}

/*
 * Removes module, sends appropriate message to kernel
 */
void cleanup_module(void)
{
	printk(KERN_INFO "lkmasg2 Writer: removing module.\n");
	device_destroy(lkmasg1Class, MKDEV(major_number, 0)); // remove the device
	class_unregister(lkmasg1Class);						  // unregister the device class
	class_destroy(lkmasg1Class);						  // remove the device class
	unregister_chrdev(major_number, DEVICE_NAME);		  // unregister the major number
	printk(KERN_INFO "lkmasg2 Writer: Goodbye from the LKM!\n");
	unregister_chrdev(major_number, DEVICE_NAME);
	return;
}

/*
 * Opens device module, sends appropriate message to kernel
 */
static int open(struct inode *inodep, struct file *filep)
{
	printk(KERN_INFO "lkmasg2 Writer: device opened.\n");
	return 0;
}

/*
 * Closes device module, sends appropriate message to kernel
 */
static int close(struct inode *inodep, struct file *filep)
{
	printk(KERN_INFO "lkmasg2 Writer: device closed.\n");
	return 0;
}

/*
 * Reads from device, displays in userspace, and deletes the read data
 */
static ssize_t read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
	/*
	char *returnBuffer = kmalloc(BUFFER_SIZE + 1, GFP_KERNEL); // Using kmalloc to keep stack frame size down
	size_t readBytes = len - 1; // Leave space for null terminator
	
	// Figure out how much data we should read from the buffer
	if (readBytes > BUFFER_SIZE) {
		readBytes = BUFFER_SIZE;
	}
	if (readBytes > availableBuffer) {
		readBytes = availableBuffer;
	}

	
	// Read from our buffer into the returnBuffer and send it back to userspace
	for (int i = 0; i < readBytes; i++) {
		returnBuffer[i] = deviceBuffer[(start + i) % BUFFER_SIZE];
	}
	returnBuffer[readBytes] = '\0'; // Set the null terminator at the end of the read
	copy_to_user(buffer, returnBuffer, readBytes + 1);

	// Mark the read parts of the buffer as read
	start = (start + readBytes) % BUFFER_SIZE;
	availableBuffer -= readBytes;

	printk(KERN_INFO "lkmasg1: Read %d bytes", readBytes);
	kfree(returnBuffer);
	return readBytes + 1;
	*/
	return ENOTSUPP;
}

/*
 * Writes to the device
 */
static ssize_t write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{	printk(KERN_INFO "lkmasg2 Writer - Entered write().");

	mutex_lock(&lkmasg2_mutex); // 
	printk(KERN_INFO "lkmasg2 Writer - Acquired the lock.");

	// Fill up the buffer
	size_t availableSpace = BUFFER_SIZE - availableBuffer;
	char *kernelBuffer = kmalloc(len, GFP_KERNEL);
	size_t bytesToWrite;

	if (len < availableSpace) {
		bytesToWrite = len;
	} else if (availableSpace == 0) {
		printk(KERN_INFO "lkmasg2 Writer - Buffer is full, unable to write");
		bytesToWrite = 0;
	} else {
		printk(KERN_INFO "lkmasg2 Writer - Buffer has %d bytes remaining, attempting to write %d,truncating input.", availableSpace, len);
		bytesToWrite = availableSpace;
	}


	copy_from_user(kernelBuffer, buffer, len);
    //copy_from_user(deviceBuffer + start + availableBuffer, buffer, bytesToWrite)

	
	//writing to buffer
    for (int i = 0; i < bytesToWrite; i++){
		deviceBuffer[(start + availableBuffer + i) % BUFFER_SIZE] = kernelBuffer[i];
	}
	
	availableBuffer += bytesToWrite;
    
	if (bytesToWrite > 0)
		printk(KERN_INFO "lkmasg2 Writer - Wrote %d bytes to the buffer.", bytesToWrite);

	kfree(kernelBuffer);

	mutex_unlock(&lkmasg2_mutex);//
	printk(KERN_INFO "lkmasg2 Writer - Exiting write() function");
	return bytesToWrite;
}

