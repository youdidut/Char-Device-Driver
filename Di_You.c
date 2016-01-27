//assignment 4+
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h> 
#include <linux/kernel.h>       /* printk() */
#include <linux/slab.h>         /* kmalloc() */
#include <linux/fs.h>           /* everything... */
#include <linux/errno.h>        /* error codes */
#include <linux/types.h>        /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>        /* O_ACCMODE */
#include <linux/seq_file.h>
#include <linux/cdev.h>
#include <linux/semaphore.h> 
#include <asm/uaccess.h>        /* copy_*_user */
#include <linux/list.h>


#define ramdisk_size (size_t) (16*PAGE_SIZE)
#define SCULL_IOC_MAGIC  'a'
#define SCULL_IOC_MAXNR 1
#define ASP_CHGACCDIR _IO(SCULL_IOC_MAGIC,   1)


/*#ifndef DEV_NR_DEVS*/
static void driver_exit(void);
static struct class *foo_class;
int dev_major=  0;
int dev_minor =  0;
int dev_nr_devs = 3;
int DIRECTION=0;
struct semaphore sem_d;
module_param(dev_nr_devs, int, S_IRUSR);
/*module_param(dev_nr_devs, int, S_IRUGO);*/
/*MODULE_LICENSE("Dual BSD/GPL");*/

struct asp_mycdrv 
{
   struct list_head list;
   struct cdev dev;
   char *ramdisk;
   struct semaphore sem;
   int devNo;
} ;
/*struct list_head head;*/
struct asp_mycdrv *dev_devices;
int ret;
dev_t dev_num;
#define DEVICE_NAME "newTestDevice"

char __user * reverse(char __user * buf, int n)
  {
    int i;
    char c;
    for (i=0;i<n/2;i++)
    {
      //pr_info("reversing buf[%d] to buf[%d]: %c, %c \n\n", i, n-1-i, buf[i],buf[n-1-i]);
      c=buf[i];
      buf[i]=buf[n-1-i];
      buf[n-1-i]=c;
      //pr_info("reversed buf[%d] to buf[%d]: %c, %c \n\n", i, n-1-i, buf[i],buf[n-1-i]);
      c=buf[i];
    }
    return buf;   
  }

int device_open(struct inode *inode, struct file *filp)
{
   struct asp_mycdrv *dev; /* device information */ 
   dev = container_of(inode->i_cdev, struct asp_mycdrv, dev);   
   filp->private_data = dev; /* for other methods */
   pr_info(" OPENING device: mycdrv%d:\n\n", dev->devNo);
   return 0;          /* success */
}

static ssize_t
device_read(struct file *file, char __user * buf, size_t lbuf, loff_t * ppos)
{
   struct asp_mycdrv *dev = file->private_data;
   int nbytes=0;
   if (down_interruptible(&dev->sem))
      return -ERESTARTSYS;
   if (down_interruptible(&sem_d))
      return -ERESTARTSYS;
   pr_info("\n READING function, direction is: %d\n", DIRECTION);
   
   if(DIRECTION==0)
   {
     if ((lbuf + *ppos) > ramdisk_size) {
        pr_info("trying to read past end of device,"
           "aborting because this is just a stub!\n");
        up(&sem_d);
        up(&dev->sem);
        return 0;
     }
     nbytes = lbuf - copy_to_user(buf, dev->ramdisk + *ppos, lbuf);
     *ppos += nbytes;
    }

   else if(DIRECTION==1)
   {
     if ((*ppos -lbuf) < 0) {
        pr_info("trying to read past beginning of device,"
           "aborting because this is just a stub!\n");
        up(&sem_d);
        up(&dev->sem);
        return 0;
     }
     nbytes = lbuf - copy_to_user(buf, dev->ramdisk + *ppos-lbuf, lbuf);
     *ppos -=nbytes;
     buf=reverse(buf,lbuf);
    }

    else pr_info("DIRECTION not reconized!\n");   
   
   pr_info("\n READING function, nbytes=%d, pos=%d\n", nbytes, (int)*ppos);
   up(&sem_d);
   up(&dev->sem);
   return nbytes; 
}

static ssize_t
device_write(struct file *file, const char __user * buf, size_t lbuf,
        loff_t * ppos)
{
   int nbytes=0,i;
   char * buffer;
   struct asp_mycdrv *dev = file->private_data;
   mm_segment_t old_fs;

   if (down_interruptible(&dev->sem))
      return -ERESTARTSYS;
   buffer=kmalloc((lbuf)*sizeof(char), GFP_KERNEL );   
   if (down_interruptible(&sem_d))
      return -ERESTARTSYS;
    pr_info("\n WRITING function, direction is: %d\n", DIRECTION);
   if(DIRECTION==0)
   {
     if ((lbuf + *ppos) > ramdisk_size) {
        pr_info("trying to write past end of device,"
           "aborting because this is just a stub!\n");
        up(&sem_d);
        up(&dev->sem);
        return 0;
     }
   nbytes = lbuf - copy_from_user(dev->ramdisk + *ppos, buf, lbuf);
   *ppos += nbytes;
    }

   else if(DIRECTION==1)
   {
     if ((*ppos -lbuf) < 0) {
        pr_info("trying to write past beginning of device,"
           "aborting because this is just a stub!\n");
        up(&sem_d);
        up(&dev->sem);
        return 0;
     }
     
     for(i=0;i<lbuf;i++)
      buffer[i]=buf[lbuf-1-i];       
     buf=buffer;
     printk("\n WRITING function output, %s \n", buf);      
     old_fs = get_fs();
     set_fs(KERNEL_DS);
     nbytes = lbuf - copy_from_user(dev->ramdisk + *ppos-lbuf, buf, lbuf);
     set_fs(old_fs);
     *ppos -=nbytes;
    }

    else pr_info("DIRECTION not reconized!\n");

   pr_info("\n WRITING function, nbytes=%d, pos=%d\n", nbytes, (int)*ppos);
   kfree(buffer);
   up(&sem_d);
   up(&dev->sem);
   return nbytes;
}

static int device_release(struct inode *inode, struct file *file)
{
   struct asp_mycdrv *dev = file->private_data;
   pr_info(" CLOSING device: mycdrv%d:\n\n", dev->devNo);
   return 0;
}

static loff_t device_lseek(struct file *file, loff_t offset, int orig) // In lab3_seek.c
{
	struct asp_mycdrv *dev = file->private_data;
  loff_t testpos;
  if (down_interruptible(&dev->sem))
      return -ERESTARTSYS;
	switch (orig) {
	case SEEK_SET:
		testpos = offset;
		break;
	case SEEK_CUR:
		testpos = file->f_pos + offset;
		break;
	case SEEK_END:
		testpos = ramdisk_size + offset;
		break;
	default:
		return -EINVAL;
	}
	testpos = testpos < ramdisk_size ? testpos : ramdisk_size;
	testpos = testpos >= 0 ? testpos : 0;
	file->f_pos = testpos;
	pr_info("Seeking to pos=%ld\n", (long)testpos);
  up(&dev->sem);
	return testpos;
}

long scull_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
   
   int err = 0;
   int retval = 0;
   
   /*
    * extract the type and number bitfields, and don't decode
    * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
    */
   if (_IOC_TYPE(cmd) != SCULL_IOC_MAGIC) return -ENOTTY;
   if (_IOC_NR(cmd) > SCULL_IOC_MAXNR) return -ENOTTY;
   
   /*
    * the direction is a bitmask, and VERIFY_WRITE catches R/W
    * transfers. `Type' is user-oriented, while
    * access_ok is kernel-oriented, so the concept of "read" and
    * "write" is reversed
    */
   if (_IOC_DIR(cmd) & _IOC_READ)
      err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
   else if (_IOC_DIR(cmd) & _IOC_WRITE)
      err =  !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
   if (err) return -EFAULT;
   
   switch(cmd) {

   case ASP_CHGACCDIR:
      if (! capable (CAP_SYS_ADMIN))
        return -EPERM;
      if (down_interruptible(&sem_d))
        return -ERESTARTSYS; 
      retval=DIRECTION;
      DIRECTION=arg;
      up(&sem_d);
      return retval;
         
   default:  /* redundant, as cmd was checked against MAXNR */
      return -ENOTTY;
   }
 }


struct file_operations fops = {
   .owner = THIS_MODULE,
   .open = device_open,
   .release = device_release,
   .write = device_write,
   .read = device_read,
   .llseek=device_lseek,
   .unlocked_ioctl = scull_ioctl
};

static void scull_setup_cdev(struct asp_mycdrv *dev, int index) {
   int err, devno = MKDEV(dev_major, dev_minor + index);   
   cdev_init(&dev->dev, &fops);
   dev->dev.owner = THIS_MODULE;
   dev->dev.ops = &fops;
   err = cdev_add (&dev->dev, devno, 1);
   /* Fail gracefully if need be */
   if (err)
      printk(KERN_NOTICE "Error %d adding device%d", err, index);
}

static int driver_entry(void)
{
     int result, i;
     dev_t dev = 0;
     LIST_HEAD(head);
   printk("device initiating:\n");
   
   if (dev_major) {
      dev = MKDEV(dev_major, dev_minor);
      result = register_chrdev_region(dev, dev_nr_devs, DEVICE_NAME);
   } else {
      result = alloc_chrdev_region(&dev, dev_minor, dev_nr_devs,
               DEVICE_NAME);
      dev_major = MAJOR(dev);
   }
   if (result < 0) {
      printk(KERN_WARNING "Device: can't get major %d\n", dev_major);
      return result;
   }
   printk("Device Module Registered! Major number %d\n", dev_major);
   dev_devices = kmalloc(dev_nr_devs*sizeof(struct asp_mycdrv), GFP_KERNEL);
   if (!dev_devices) {
      result = -ENOMEM;
      printk("fail to malloc space for devices\n");  /* Make this more graceful */
   }
   memset(dev_devices, 0, dev_nr_devs * sizeof(struct asp_mycdrv));
   
   foo_class = class_create(THIS_MODULE, "my_class");
   sema_init(&sem_d, 1);
   for (i = 0; i <  dev_nr_devs; i++) {
      dev_devices[i].ramdisk = kmalloc(ramdisk_size, GFP_KERNEL);
      dev_devices[i].devNo=i;
      sema_init(&dev_devices[i].sem, 1);
      scull_setup_cdev(&dev_devices[i], i);
      device_create(foo_class, NULL, MKDEV(MAJOR(dev), MINOR(dev) + i) , NULL, "mycdrv%d", i);
      list_add (&dev_devices[i].list ,&head) ;
      printk("set up the %dth device.\n",i);
   }
   return 0;
 }

static void driver_exit(void)
{
   int i;
   dev_t devno = MKDEV(dev_major, dev_minor);
   
   /* Get rid of our char dev entries */
   if (dev_devices) {
      for (i = 0; i < dev_nr_devs; i++) {
    kfree(dev_devices[i].ramdisk);
    cdev_del(&dev_devices[i].dev);
    device_destroy(foo_class, MKDEV(MAJOR(devno), MINOR(devno) + i));
      }
      kfree(dev_devices);
      class_destroy(foo_class);
   }
  
   /* cleanup_module is never called if registering failed */
   unregister_chrdev_region(devno, dev_nr_devs);
   printk("Device module unregistered!\n");
}

module_init(driver_entry);
module_exit(driver_exit);
MODULE_AUTHOR("Jerry Cooperstein");
MODULE_LICENSE("GPL v2"); 