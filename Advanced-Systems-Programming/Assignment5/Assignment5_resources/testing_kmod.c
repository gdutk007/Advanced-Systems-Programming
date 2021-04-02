#include <stdio.h>
int main()
{
     /* Pointer to the file */
     FILE *fp1;
     /* Character variable to read the content of file */
     char c;

     /* Opening a file in r mode*/
     fp1= fopen ("/dev/mycdrv0", "r");
     if(fp1 == NULL){
	printf("We Could Not Open the FILE!!!!");
      }else{
	printf("Successfuly opened the FILE!!!!");
      }
     /* Infinite loop –I have used break to come out of the loop*/
    // while(1)
    // {
    //    c = fgetc(fp1);
    //    if(c==EOF)
    //        break;
    //    else
    //        printf("%c", c);
    // }
     fclose(fp1);
     return 0;
}
