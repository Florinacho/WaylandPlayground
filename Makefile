OUTPUT=a.out

#xdg-shell.c 
SOURCE_FILES=wl_window.c main.c 

CC=gcc
C_FLAGS=-Wall -Wextra -g -std=gnu11
L_FLAGS= -lrt
LIBS=rt wayland-client

#C_FLAGS+=-DWL_XDG
#SOURCE_FILES+=xdg-shell.c

LIB_FLAGS=$(addprefix -l, $(LIBS))
OBJECT_FILES=$(SOURCE_FILES:.c=.o)

build: $(OUTPUT)

clean: 
	rm -rf $(OUTPUT) *.o xdg-shell.*

rebuild: clean build

xdg-shell-unstable-v6.xml:
	curl -O https://cgit.freedesktop.org/wayland/wayland-protocols/plain/unstable/xdg-shell/xdg-shell-unstable-v6.xml

xdg-shell.c: xdg-shell-unstable-v6.xml
	wayland-scanner client-header xdg-shell-unstable-v6.xml xdg-shell.h
	wayland-scanner public-code xdg-shell-unstable-v6.xml xdg-shell.c

%.o: %.c
	$(CC) $(C_FLAGS) -c $< -o $@

$(OUTPUT): $(OBJECT_FILES)
	$(CC) $(L_FLAGS) $(OBJECT_FILES) $(LIB_FLAGS) -o $(OUTPUT)
