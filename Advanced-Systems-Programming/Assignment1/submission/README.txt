Hi, please make sure that the .txt file you use to test this code was created saved in a linux machine. If not,
There's a chance that the program will not run because of the convetntion DOS of "\n\r" vs "\n" for linux. If 
it happens to be the case that the input file was generated on a windows machine please make sure to run the dos2unix 
utility on linux prior to testing out this software. 

To make: 
  - decompress
  - enter: "make"
  - ./location_updated < input_posted.txt

To test programs individually: 
  - ./email_filter < input_posted.txt | ./calendar_filter


