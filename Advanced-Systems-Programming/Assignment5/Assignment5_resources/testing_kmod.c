#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "char_driver.h"

int main()
{
      int fd = open("/dev/mycdrv0", O_RDWR);
      if(fd < 0){
            printf("Value of errno: %d\n ", errno);
            printf("The error message is : %s\n",strerror(errno));
            printf("We could not open the file. Failed with %i \n",fd);
      }else{
            const size_t BUFF_COUNT = 256;
            off_t offset = 0;
            char buff[BUFF_COUNT];
            char nullVal = 0x00;
            memcpy(&buff[0],&nullVal,BUFF_COUNT);

            const char * str = "This is a test. Testing... 1 2 3.\n";
	      char testString[256];
            const int str_size = strlen(str);
            
            // offset = lseek(fd,65537,SEEK_SET);
            // if(offset > 0){
            //       printf("lseek was successfull\n");
            // }else{
            //       printf("lseek failed with errno: %d\n ", errno);
            //       printf("The errno message is : %s\n",strerror(errno));
            // }
            
            ssize_t bytesWritten = write(fd, str, str_size);
            if(bytesWritten >= 0){
                  printf("We wrote %li bytes. The length of the string: %d\n"
                                                      ,bytesWritten, str_size);      
            }else{
                  printf("Writing file failed with errno: %d\n ", errno);
                  printf("The errno message is : %s\n",strerror(errno));
            }
            
            offset = lseek(fd,0,SEEK_SET);
            if(offset > 0){
                  printf("lseek was successfull\n");
            }else{
                  printf("lseek failed with errno: %d\n ", errno);
                  printf("The errno message is : %s\n",strerror(errno));
            }

            printf("We could open the file. Attempting to read. \n");
	      ssize_t bytesRead = read(fd,&buff[0],str_size);
            buff[str_size] = '\0';
	      if(bytesRead >= 0){
                  printf("We read %li bytes. String: %s \n",bytesRead, &buff[0]);      
            }else{
                  printf("Reading file failed with errno: %d\n ", errno);
                  printf("The errno message is : %s\n",strerror(errno));
            }

            int io = 0;
            io = ioctl(fd,ASP_CLEAR_BUF,0);            

            offset = lseek(fd,0,SEEK_SET);
            if(offset > 0){
                  printf("lseek was successfull\n");
            }else{
                  printf("lseek failed with errno: %d\n ", errno);
                  printf("The errno message is : %s\n",strerror(errno));
            }

            printf("We could open the file. Attempting to read. \n");
	      bytesRead = read(fd,&buff[0],str_size);
            buff[str_size] = '\0';
	      if(bytesRead >= 0){
                  printf("We read %li bytes. String: %s \n",bytesRead, &buff[0]);      
            }else{
                  printf("Reading file failed with errno: %d\n ", errno);
                  printf("The errno message is : %s\n",strerror(errno));
            }
      }

     return 0;
}
