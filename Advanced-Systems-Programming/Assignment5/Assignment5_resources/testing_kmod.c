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
            const size_t BUFF_COUNT = 256;
            char buff[BUFF_COUNT];
            char nullVal = 0x00;
            memcpy(buff,&nullVal,BUFF_COUNT);

            const char * str = "This is a test. Testing... 1 2 3.\n\0";
	      char testString[256];
            //memcpy(testString,str,sizeof(str));
            
            ssize_t bytesWritten = write(fd, str, 25);
            if(bytesWritten >= 0){
                  printf("We wrote %li bytes.\n",bytesWritten);      
            }else{
                  printf("Writing file failed with errno: %d\n ", errno);
                  printf("The errno message is : %s\n",strerror(errno));
            }
            printf("We could open the file. Attempting to read. \n");
	      ssize_t bytesRead = read(fd,&buff[0],25);
	      if(bytesRead >= 0){
                  printf("We read %li bytes. String: %s \n",bytesRead, &buff[0]);      
            }else{
                  printf("Reading file failed with errno: %d\n ", errno);
                  printf("The errno message is : %s\n",strerror(errno));
            }
      }

     return 0;
}
