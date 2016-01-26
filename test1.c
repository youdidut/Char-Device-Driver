#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <unistd.h>  
#include <sys/types.h>  
#include <sys/stat.h>  
#include <fcntl.h>  
//Di_You_hw4.h needs to be included to use the ASP_CHGACCDIR function.
#include "Di_You_hw4.h"
  
int main()  
{      
    int fp, ret, ori;  
    unsigned char ch[]="hello666";  
    unsigned char *buf1 = (unsigned char *)malloc(sizeof(ch)+1);  
    unsigned char *buf2 = (unsigned char *)malloc(sizeof(ch)+1);  
    //init buff  
    memset(buf1, 0, sizeof(ch)+1);  
    memset(buf2, 0, sizeof(ch)+1);  
    strcpy(buf1, ch);//ch to buf1  
    //open device
    fp = open("/dev/mycdrv0", O_RDWR);  
    printf("fp is %d\n", fp);  
      
    ret = write(fp, buf1, sizeof(ch)-1);   //write buf1 to fp  
    printf("write return : %d\n", ret); 

    ori=ioctl(fp, ASP_CHGACCDIR, 1);   //change the direction from 0 to 1
    printf("Original direction is %d. Now it is 1. \n ", ori); 

    ret = read(fp, buf2, sizeof(ch)-1);//read fp to buf2  
    printf("read return : %d\n", ret);  
    printf("read data:%s\n", buf2);

    ret = read(fp,buf2, sizeof(ch)-1); 

    lseek(fp,8,0);   

    ret = write(fp, buf1, sizeof(ch)-1);//write buf1 to fp 
    printf("write return : %d\n", ret);
    ori=ioctl(fp, ASP_CHGACCDIR, 0); //change the direction from1 to 0
    printf("Original direction is %d. Now it is 0. \n ", ori); 
    ret = read(fp, buf2, sizeof(ch)-1);//read fp to buf2  
    printf("read return : %d\n", ret);  
    printf("read data:%s\n", buf2);

    close(fp);  
    return 0;  
}  
