// https://bugaevc.gitbooks.io/writing-wayland-clients/content/black-square/the-complete-code.html
// https://wayland-client-d.dpldocs.info/wayland.client.protocol.wl_registry_set_user_data.html
// https://wayland.freedesktop.org/docs/html/apa.html

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <syscall.h>
#include <unistd.h>
#include <sys/mman.h>

#include <wayland-client.h>

#if defined(XDG)
#include "xdg-shell.h"
#endif

#define BTN_LEFT         0x110
#define BTN_RIGHT        0x111
#define BTN_MIDDLE       0x112

#define LATCH_SHIFT      0x01
#define LATCH_CAPS_LOCK  0x02
#define LATCH_CONTROL    0x04
#define LATCH_ALT        0x08
#define LATCH_NUM_LOCK   0x10

#define KEY_ESCAPE       0x01

typedef struct {
	struct wl_compositor *compositor;
	struct wl_shm *shm;
	struct wl_seat *seat;
	struct wl_pointer *pointer;
	struct wl_keyboard *keyboard;
#if defined(XDG)
	struct zxdg_shell_v6 *xdg_shell;
#else
	struct wl_shell *shell;
#endif
} wl_context;

bool running = true;

// Registry
void registry_global_handler(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version) {
	wl_context* context = (wl_context*)data;
	printf("Register handler (%u)%s %u\n", name, interface, version);

	if (strcmp(interface, "wl_compositor") == 0) {
		context->compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 3);
	} else 
	if (strcmp(interface, "wl_shm") == 0) {
		context->shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
	} else 
	if (strcmp(interface, "wl_seat") == 0) {
        context->seat = wl_registry_bind(registry, name, &wl_seat_interface, 1);
	} else 
#if defined(XDG)
	if (strcmp(interface, "zxdg_shell_v6") == 0) {
		context->xdg_shell = wl_registry_bind(registry, name, &zxdg_shell_v6_interface, 1);
    } else
#else
    if (strcmp(interface, "wl_shell") == 0) {
		context->shell = wl_registry_bind(registry, name, &wl_shell_interface, 1);
	} else
#endif
	{
		// unknown
	}
}

void registry_global_remove_handler(void *data, struct wl_registry *registry, uint32_t name) {
	printf("Unregister handler %u\n", name);
}

const struct wl_registry_listener registry_listener = {
	.global = registry_global_handler,
	.global_remove = registry_global_remove_handler
};

// Pointer
void pointer_enter_handler(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface, wl_fixed_t x, wl_fixed_t y) {
	printf("Pointer enter %f, %f\n", wl_fixed_to_double(x), wl_fixed_to_double(y));
}

void pointer_leave_handler(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface) {
	printf("Pointer leave\n");
}

void pointer_motion_handler(void *data, struct wl_pointer *pointer, uint32_t time, wl_fixed_t x, wl_fixed_t y) {
	static uint32_t count = 0;
	printf("Cursor motion %d: %f, %f\n", count++, wl_fixed_to_double(x), wl_fixed_to_double(y));
}

void pointer_button_handler(void *data, struct wl_pointer *pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state) {
	printf("Button: %d, %d\n", button, state);
}

void pointer_axis_handler(void *data, struct wl_pointer *pointer, uint32_t time, uint32_t axis, wl_fixed_t value) {
	printf("Axis: %d, %f\n", axis, wl_fixed_to_double(value));
}

const struct wl_pointer_listener pointer_listener = {
    .enter = pointer_enter_handler,
    .leave = pointer_leave_handler,
    .motion = pointer_motion_handler,
    .button = pointer_button_handler,
    .axis = pointer_axis_handler
};

// Keyboard
void keymap_handler(void* data, struct wl_keyboard* keyboard, uint32_t format, int fd, uint32_t size) {
	printf("Key map:\n\tformat: %u\n\tfd: %d\n\tsize: %u\n\n", format, fd, size);
}

void key_enter_handler(void* data, struct wl_keyboard* keyboard, uint32_t serial, struct wl_surface* surface, struct wl_array* keys) {
	printf("Key enter\n");
}

void key_leave_handler(void* data, struct wl_keyboard* keyboard, uint32_t serial, struct wl_surface* surface) {
	printf("Key leave\n");
}

void key_handler(void* data, struct wl_keyboard* keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
	printf("Key %X is %s\n", key, (state ? "pressed" : "released"));
	switch (key) {
	case KEY_ESCAPE : 
		running = false;
		break;
	}
}

void key_modifiers_handler(void* data, struct wl_keyboard* keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group) {
	printf("Key modifier:\n\tserial: %u\n\tdepressed: %u\n\tlatched: %u\n\tlocked: %u\n\tgroup: %u\n\n", serial, mods_depressed, mods_latched, mods_locked, group);
}

void key_repeat_info_handler(void* data, struct wl_keyboard* keyboard, int rate, int delay) {
	printf("Key repeat:\n\trate: %d\n\tdelay: %d\n\n", rate, delay);
}

const struct wl_keyboard_listener keyboard_listener = {
	.keymap = keymap_handler,
	.enter = key_enter_handler,
	.leave = key_leave_handler,
	.key = key_handler,
	.modifiers = key_modifiers_handler,
	.repeat_info = key_repeat_info_handler
};

#if defined(XDG)
void xdg_toplevel_configure_handler(void *data, struct zxdg_toplevel_v6 *xdg_toplevel, int32_t width, int32_t height, struct wl_array *states) {
	printf("XDG configure: %dx%d\n", width, height);
}

void xdg_toplevel_close_handler(void *data, struct zxdg_toplevel_v6 *xdg_toplevel) {
	printf("XDG close\n");
}

const struct zxdg_toplevel_v6_listener xdg_toplevel_listener = {
	.configure = xdg_toplevel_configure_handler,
	.close = xdg_toplevel_close_handler
};

void xdg_surface_configure_handler(void *data, struct zxdg_surface_v6 *xdg_surface, uint32_t serial) {
	zxdg_surface_v6_ack_configure(xdg_surface, serial);
}

const struct zxdg_surface_v6_listener xdg_surface_listener = {
    .configure = xdg_surface_configure_handler
};

void xdg_shell_ping_handler(void *data, struct zxdg_shell_v6 *xdg_shell, uint32_t serial) {
	zxdg_shell_v6_pong(xdg_shell, serial);
	printf("XDG ping-pong\n");
}

const struct zxdg_shell_v6_listener xdg_shell_listener = {
    .ping = xdg_shell_ping_handler
};
#endif

int main(void) {
	wl_context context = {0};

	struct wl_display *display = wl_display_connect(NULL);
	struct wl_registry *registry = wl_display_get_registry(display);
	wl_registry_add_listener(registry, &registry_listener, NULL);
	wl_registry_set_user_data(registry, &context);

	// wait for the "initial" set of globals to appear
	wl_display_roundtrip(display);
	
	// this is only going to work if your computer has a pointing device
	context.pointer = wl_seat_get_pointer(context.seat);
	wl_pointer_add_listener(context.pointer, &pointer_listener, NULL);
	
	context.keyboard = wl_seat_get_keyboard(context.seat);
	wl_keyboard_add_listener(context.keyboard, &keyboard_listener, NULL);

#if defined(XDG)
	struct wl_surface *surface = wl_compositor_create_surface(context.compositor);
	struct zxdg_surface_v6 *xdg_surface = zxdg_shell_v6_get_xdg_surface(context.xdg_shell, surface);
	zxdg_surface_v6_add_listener(xdg_surface, &xdg_surface_listener, NULL);
	
	struct zxdg_toplevel_v6 *xdg_toplevel = zxdg_surface_v6_get_toplevel(xdg_surface);
	zxdg_toplevel_v6_add_listener(xdg_toplevel, &xdg_toplevel_listener, NULL);

	wl_display_roundtrip(display);
	zxdg_shell_v6_add_listener(context.xdg_shell, &xdg_shell_listener, NULL);
#else
	struct wl_surface *surface = wl_compositor_create_surface(context.compositor);
	struct wl_shell_surface *shell_surface = wl_shell_get_shell_surface(context.shell, surface);
	wl_shell_surface_set_toplevel(shell_surface);
#endif
	uint32_t width = 800;
	uint32_t height = 600;
	uint32_t stride = width * 4;
	uint32_t count = 1;
	uint32_t size = stride * height * count;  // bytes

	// open an anonymous file and write some zero bytes to it
	int fd = syscall(SYS_memfd_create, "buffer", 0);
	ftruncate(fd, size);

	// map it to the memory
	uint8_t *data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	// turn it into a shared memory pool
	struct wl_shm_pool *pool = wl_shm_create_pool(context.shm, fd, size);

	// allocate the buffer in that pool
	struct wl_buffer *buffer = wl_shm_pool_create_buffer(pool, 0, width, height, stride, WL_SHM_FORMAT_XRGB8888);

	wl_surface_attach(surface, buffer, 0, 0);
	wl_surface_commit(surface);
	
	uint32_t value = 0xFF000000;

	while (running) {
		for (int x = 0; x < width; x++) {
			for (int y = 0; y < height; y++) {
				*((uint32_t*)(data + y * stride + x * 4)) = value;
			}
		}
		++value;
#if 1
		wl_display_dispatch(display);
#else
		wl_display_flush(display);
		wl_display_dispatch_pending(display);
#endif
	}

	wl_buffer_destroy(buffer);
	wl_shm_pool_destroy(pool);
	munmap(data, size);
#if defined(XDG)
	zxdg_surface_v6_destroy(xdg_surface);
#else
	wl_shell_surface_destroy(shell_surface);
#endif
	wl_surface_destroy(surface);
	wl_keyboard_destroy(context.keyboard);
	wl_pointer_destroy(context.pointer);
	wl_seat_destroy(context.seat);
	wl_shell_destroy(context.shell);
	wl_shm_destroy(context.shm);
	wl_compositor_destroy(context.compositor);
	wl_registry_destroy(registry);
	wl_display_disconnect(display);
	close(fd);
	
	return 0;
}
