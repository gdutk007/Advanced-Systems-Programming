Assignment: simulator

simulator:
	gcc driver_simulator.c -o driver_simulator -lpthread
	gcc keyboard.c         -o keyboard         -lpthread
	gcc simulator.c        -o sim              -lpthread

clean:
	rm -rf *.o *.ko *.mod.* *.symvers *.order *~  driver_simulator keyboard sim
