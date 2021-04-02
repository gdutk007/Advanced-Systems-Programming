#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
int main()
{
     

  printf("Hello World!\n");
/* Pointer to the file */
     

      int fd = open("/dev/mycdrv0", O_RDWR);
      if(fd < 0){
            printf(" Value of errno: %d\n ", errno);
            printf("The error message is : %s\n",strerror(errno));
            printf("We could not open the file. Failed with %i \n",fd);
      }else{
            printf("We could open the file.\n");
      }

     return 0;
}
