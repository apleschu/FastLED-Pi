s:	s.c fastled.h ../rpi_ws281x/libws2811.a
	gcc -o s s.c ../rpi_ws281x/libws2811.a
	sudo chown root s
	sudo chmod 777 s
	sudo chmod +s s
#	./s
