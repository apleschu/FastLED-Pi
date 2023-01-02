#s:	s.c fastled.h ../rpi_ws281x/libws2811.a
#	gcc -o s s.c ../rpi_ws281x/libws2811.a -lm
#	sudo chown root s
#	sudo chmod 777 s
#	sudo chmod +s s
##	./s
#xy:	xy.c fastled.h ../rpi_ws281x/libws2811.a
#	gcc -o xy xy.c ../rpi_ws281x/libws2811.a -lm
#	sudo chown root xy
#	sudo chmod 777 xy
#	sudo chmod +s xy
c:	clock.c font5x10.h colors.h fastled.h ../rpi_ws281x/libws2811.a
	gcc -g -o c clock.c ../rpi_ws281x/libws2811.a -lm
	sudo chown root c
	sudo chmod 777 c
	sudo chmod +s c
