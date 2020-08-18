
picture_lib: Main.o Utils.o Picture.o PicProcess.o
	gcc sod_118/sod.c Main.o Utils.o Picture.o PicProcess.o -I sod_118 -lm -std=c99 -o picture_lib
	
Utils.o: Utils.h Utils.c

Picture.o: Utils.h Picture.h Picture.c

PicProcess.o: Utils.h PicProcess.h PicProcess.c

Main.o: Main.c Utils.h Picture.h PicProcess.h

%.o: %.c
	gcc -c -I sod_118 -std=c99 $<

clean:
	rm -rf picture_lib *.o

.PHONY: clean
