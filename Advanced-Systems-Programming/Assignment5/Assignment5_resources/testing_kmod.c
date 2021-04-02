#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
int main()
{
     

  printf("Hello World!\n");
/* Pointer to the file */
     
      int fd = open("/dev/mycdrv0", O_RDWR);
      if(fd < 0){
            printf("Value of errno: %d\n ", errno);
            printf("The error message is : %s\n",strerror(errno));
            printf("We could not open the file. Failed with %i \n",fd);
      }else{
            const size_t BUFF_COUNT = 10;
            char buff[BUFF_COUNT];
            char nullVal = 0x00;
	      memcpy(buff,&nullVal,10);
            printf("We could open the file. Attempting to read. \n");
	      ssize_t bytesRead = read(fd,buff,BUFF_COUNT);
	      if(bytesRead >= 0){
                  printf("We read %li bytes.\n",bytesRead);      
            }else{
                  printf("Reading file failed with errno: %d\n ", errno);
                  printf("The errno message is : %s\n",strerror(errno));
            }
      }

     return 0;
}
