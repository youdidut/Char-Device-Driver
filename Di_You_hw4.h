#ifndef _SCULL_IOCTL_H_
#define _SCULL_IOCTL_H_

#include <linux/ioctl.h> /* needed for the _IOW etc stuff used later */

/*
 * Ioctl definitions
 */

/* Use 'k' as magic number */
#define SCULL_IOC_MAGIC  'a'
/* Please use a different 8-bit number in your code */

#define ASP_CHGACCDIR _IO(SCULL_IOC_MAGIC,   1)



#define SCULL_IOC_MAXNR 1

#endif 
