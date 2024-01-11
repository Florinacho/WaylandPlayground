OUTPUT=a.out

SOURCE_FILES=main.c
#C_FLAGS+=-DXDG
#SOURCE_FILES+=xdg-shell.c

CC=gcc
C_FLAGS=
L_FLAGS=
LIBS=wayland-client

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
