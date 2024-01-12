// https://gitlab.freedesktop.org/wayland
// https://github.com/4X3L/example-wayland/blob/master/src/main.c#L782
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

#define UNUSED(X)        ((void)(X))

#define BTN_LEFT         0x0110
#define BTN_RIGHT        0x0111
#define BTN_MIDDLE       0x0112

#define LATCH_SHIFT      0x0001
#define LATCH_CAPS_LOCK  0x0002
#define LATCH_CONTROL    0x0004
#define LATCH_ALT        0x0008
#define LATCH_NUM_LOCK   0x0010

#define KEY_ESCAPE       0x0001

typedef struct {
	struct wl_display *display;
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

// ==================== Pointer ====================
void pointer_enter_handler(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface, wl_fixed_t x, wl_fixed_t y) {
	UNUSED(data);
	UNUSED(pointer);
	UNUSED(serial);
	UNUSED(surface);

	printf("Pointer enter %f, %f\n", wl_fixed_to_double(x), wl_fixed_to_double(y));
}

void pointer_leave_handler(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface) {
	UNUSED(data);
	UNUSED(pointer);
	UNUSED(serial);
	UNUSED(surface);

	printf("Pointer leave\n");
}

void pointer_motion_handler(void *data, struct wl_pointer *pointer, uint32_t time, wl_fixed_t x, wl_fixed_t y) {
	UNUSED(data);
	UNUSED(pointer);
	UNUSED(time);

	printf("Pointer motion %d, %d\n", wl_fixed_to_int(x), wl_fixed_to_int(y));
}

void pointer_button_handler(void *data, struct wl_pointer *pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state) {
	UNUSED(data);
	UNUSED(pointer);
	UNUSED(serial);
	UNUSED(time);

	printf("Button: %d, %d\n", button, state);
}

void pointer_axis_handler(void *data, struct wl_pointer *pointer, uint32_t time, uint32_t axis, wl_fixed_t value) {
	UNUSED(data);
	UNUSED(pointer);
	UNUSED(time);

	printf("Axis: %d, %f\n", axis, wl_fixed_to_double(value));
}

void pointer_frame_handler(void* data, struct wl_pointer* pointer) {
	UNUSED(data);
	UNUSED(pointer);
}

void pointer_axis_source_handler(void* data, struct wl_pointer* pointer, uint32_t axis_source) {
	UNUSED(data);
	UNUSED(pointer);
	UNUSED(axis_source);
}

void pointer_axis_stop_handler(void* data, struct wl_pointer* pointer, uint32_t time, uint32_t axis)  {
	UNUSED(data);
	UNUSED(pointer);
	UNUSED(time);
	UNUSED(axis);
}

void pointer_axis_discrete_handler(void* data, struct wl_pointer* pointer, uint32_t axis, int discrete) {
	UNUSED(data);
	UNUSED(pointer);
	UNUSED(axis);
	UNUSED(discrete);
}

const struct wl_pointer_listener pointer_listener = {
    .enter = pointer_enter_handler,
    .leave = pointer_leave_handler,
    .motion = pointer_motion_handler,
    .button = pointer_button_handler,
    .axis = pointer_axis_handler,
	.frame = pointer_frame_handler,
	.axis_source = pointer_axis_source_handler,
	.axis_stop = pointer_axis_stop_handler,
	.axis_discrete = pointer_axis_discrete_handler,
};

// ==================== Keyboard ====================
void keymap_handler(void* data, struct wl_keyboard* keyboard, uint32_t format, int fd, uint32_t size) {
	UNUSED(data);
	UNUSED(keyboard);

	printf("Key map format: %u, fd: %d, size: %u\n", format, fd, size);
}

void key_enter_handler(void* data, struct wl_keyboard* keyboard, uint32_t serial, struct wl_surface* surface, struct wl_array* keys) {
	UNUSED(data);
	UNUSED(keyboard);
	UNUSED(serial);
	UNUSED(surface);
	UNUSED(keys);

	printf("Key enter\n");
}

void key_leave_handler(void* data, struct wl_keyboard* keyboard, uint32_t serial, struct wl_surface* surface) {
	UNUSED(data);
	UNUSED(keyboard);
	UNUSED(serial);
	UNUSED(surface);

	printf("Key leave\n");
}

void key_handler(void* data, struct wl_keyboard* keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
	UNUSED(data);
	UNUSED(keyboard);
	UNUSED(serial);
	UNUSED(time);

	printf("Key 0x%.2X is %s\n", key, (state ? "pressed" : "released"));

	switch (key) {
	case KEY_ESCAPE : 
		running = false;
		break;
	}
}

void key_modifiers_handler(void* data, struct wl_keyboard* keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group) {
	UNUSED(data);
	UNUSED(keyboard);
	UNUSED(serial);

	printf("Key modifier depressed: 0x%.2X, latched: 0x%.2X, locked: 0x%.2X, group: %u\n", mods_depressed, mods_latched, mods_locked, group);
}

void key_repeat_info_handler(void* data, struct wl_keyboard* keyboard, int rate, int delay) {
	UNUSED(data);
	UNUSED(keyboard);

	printf("Key repeat:\n\trate: %d\n\tdelay: %d\n\n", rate, delay);
}

const struct wl_keyboard_listener keyboard_listener = {
	.keymap = keymap_handler,
	.enter = key_enter_handler,
	.leave = key_leave_handler,
	.key = key_handler,
	.modifiers = key_modifiers_handler,
	.repeat_info = key_repeat_info_handler,
};

// ==================== Seat ====================
void seat_capabilities_handler(void *data, struct wl_seat *seat, uint32_t capabilities) {
	wl_context *context = (wl_context *)data;

	if (capabilities & WL_SEAT_CAPABILITY_POINTER) {
		context->pointer = wl_seat_get_pointer(seat);
		wl_pointer_add_listener(context->pointer, &pointer_listener, NULL);
	}
	if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD) {
		context->keyboard = wl_seat_get_keyboard(seat);
		wl_keyboard_add_listener(context->keyboard, &keyboard_listener, NULL);
	}
	if (capabilities & WL_SEAT_CAPABILITY_TOUCH) {
//		context->touch = wl_seat_get_touch(seat);
//		wl_touch_add_listener(context->touch, &touch_listener, NULL);
	}
}

void seat_name_handler(void* data, struct wl_seat* seat, const char* name) {
	UNUSED(data);
	UNUSED(seat);
	UNUSED(name);
}

const struct wl_seat_listener seat_listener = {
	.capabilities = seat_capabilities_handler,
	.name = seat_name_handler,
};

#if defined(XDG)
// ==================== XDG extension ====================
void xdg_toplevel_configure_handler(void *data, struct zxdg_toplevel_v6 *xdg_toplevel, int32_t width, int32_t height, struct wl_array *states) {
	UNUSED(data);
	UNUSED(xdg_toplevel);
	UNUSED(states);

	printf("XDG configure: %dx%d\n", width, height);
}

void xdg_toplevel_close_handler(void *data, struct zxdg_toplevel_v6 *xdg_toplevel) {
	UNUSED(data);
	UNUSED(xdg_toplevel);

	printf("XDG close\n");
}

const struct zxdg_toplevel_v6_listener xdg_toplevel_listener = {
	.configure = xdg_toplevel_configure_handler,
	.close = xdg_toplevel_close_handler
};

void xdg_surface_configure_handler(void *data, struct zxdg_surface_v6 *xdg_surface, uint32_t serial) {
	UNUSED(data);
	UNUSED(xdg_surface);

	zxdg_surface_v6_ack_configure(xdg_surface, serial);
}

const struct zxdg_surface_v6_listener xdg_surface_listener = {
    .configure = xdg_surface_configure_handler
};

void xdg_shell_ping_handler(void *data, struct zxdg_shell_v6 *xdg_shell, uint32_t serial) {
	UNUSED(data);

	zxdg_shell_v6_pong(xdg_shell, serial);
	printf("XDG ping-pong\n");
}

const struct zxdg_shell_v6_listener xdg_shell_listener = {
    .ping = xdg_shell_ping_handler
};
#endif


// ==================== Shell surface ====================
void shell_surface_ping_handler(void* data, struct wl_shell_surface* shell_surface, uint32_t serial) {
	UNUSED(data);
	UNUSED(shell_surface);
	UNUSED(serial);

	printf("Shell surface ping\n");
	wl_shell_surface_pong(shell_surface, serial);
}

void shell_surface_configure_handler(void* data, struct wl_shell_surface* shell_surface, uint32_t edges, int width, int height) {
	UNUSED(data);
	UNUSED(shell_surface);

	printf("Shell surface configure: %u, %d, %d\n", edges, width, height);
}

void shell_surface_popup_done_handler(void* data, struct wl_shell_surface* shell_surface) {
	UNUSED(data);
	UNUSED(shell_surface);

	printf("Shell surface popup done\n");
}

const struct wl_shell_surface_listener shell_surface_listener = {
	.ping = shell_surface_ping_handler,
	.configure = shell_surface_configure_handler,
	.popup_done = shell_surface_popup_done_handler
};

// ==================== Frame ====================
void buffer_release_handler(void *data, struct wl_buffer *buf) {
	UNUSED(buf);

	bool *b = data;
	*b = false;
}

const struct wl_buffer_listener buffer_listener = {
	.release = buffer_release_handler
};

void callback_set_ready(void *data, struct wl_callback *cb, uint32_t cb_data) {
	UNUSED(cb);
	UNUSED(cb_data);

	bool *ready = data;
	*ready = true;
}

const struct wl_callback_listener frame_limiter_listener = {
	.done = callback_set_ready
};

// ==================== Registry ====================
void registry_global_handler(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version) {
	wl_context* context = (wl_context*)data;

	printf("Register %u. %s v%u\n", name, interface, version);

	if (strcmp(interface, "wl_compositor") == 0) {
		context->compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 3);
	} else 
	if (strcmp(interface, "wl_shm") == 0) {
		context->shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
	} else 
	if (strcmp(interface, "wl_seat") == 0) {
        context->seat = wl_registry_bind(registry, name, &wl_seat_interface, 1);
		wl_seat_add_listener(context->seat, &seat_listener, context);
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
	UNUSED(data);
	UNUSED(registry);

	printf("Unregister %u\n", name);
}

const struct wl_registry_listener registry_listener = {
	.global = registry_global_handler,
	.global_remove = registry_global_remove_handler
};

int main(void) {
	wl_context context = {0};

	struct wl_display *display = wl_display_connect(NULL);

	struct wl_registry *registry = wl_display_get_registry(display);
	wl_registry_add_listener(registry, &registry_listener, NULL);
	wl_registry_set_user_data(registry, &context);
	wl_display_roundtrip(display);

	struct wl_surface *surface = wl_compositor_create_surface(context.compositor);
#if defined(XDG)
	struct zxdg_surface_v6 *xdg_surface = zxdg_shell_v6_get_xdg_surface(context.xdg_shell, surface);
	zxdg_surface_v6_add_listener(xdg_surface, &xdg_surface_listener, NULL);
	
	struct zxdg_toplevel_v6 *xdg_toplevel = zxdg_surface_v6_get_toplevel(xdg_surface);
	zxdg_toplevel_v6_add_listener(xdg_toplevel, &xdg_toplevel_listener, NULL);

	wl_display_roundtrip(display);
	zxdg_shell_v6_add_listener(context.xdg_shell, &xdg_shell_listener, NULL);
#else
	struct wl_shell_surface *shell_surface = wl_shell_get_shell_surface(context.shell, surface);
	wl_shell_surface_set_toplevel(shell_surface);
	wl_shell_surface_add_listener(shell_surface, &shell_surface_listener, NULL);
	
	// Fullscreen currently not working in WSL
	// Scaling methods:
	//   WL_SHELL_SURFACE_FULLSCREEN_METHOD_DEFAULT
	//   WL_SHELL_SURFACE_FULLSCREEN_METHOD_SCALE
	//   WL_SHELL_SURFACE_FULLSCREEN_METHOD_DRIVER
	//   WL_SHELL_SURFACE_FULLSCREEN_METHOD_FILL
	// Framerate in mHz
	// wl_shell_surface_set_fullscreen(shell_surface, WL_SHELL_SURFACE_FULLSCREEN_METHOD_DRIVER, 60000, NULL);
	
	// Maximize currently not working in WSL
	// wl_shell_surface_set_maximized(shell_surface, NULL);
#endif
	uint32_t width = 800;
	uint32_t height = 600;
	uint32_t stride = width * 4;
	uint32_t count = 2;
	uint32_t size = stride * height;  // bytes
	uint32_t totalSize = size * count;

	// open an anonymous file and write some zero bytes to it
	int fd = syscall(SYS_memfd_create, "buffer", 0);
	ftruncate(fd, totalSize);

	// map it to the memory
	uint8_t *data = mmap(NULL, totalSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	// turn it into a shared memory pool
	struct wl_shm_pool *pool = wl_shm_create_pool(context.shm, fd, totalSize);

	// allocate the buffer in that pool
	int activeBuffer = 0;
	struct wl_buffer *buffer[2];
	for (uint32_t index = 0; index < count; ++index) {
		buffer[index] = wl_shm_pool_create_buffer(pool, index * size, width, height, stride, WL_SHM_FORMAT_XRGB8888);
	}

	wl_surface_attach(surface, buffer[activeBuffer], 0, 0);
	wl_surface_commit(surface);
	
	uint32_t value = 0xFF000000;
	struct wl_callback *ready_cb = NULL;
	bool ready = true;

	while (running) {
#if 0
		if (ready) {
			ready = false;
			// current->used = true;
			if (ready_cb != NULL) {
				wl_callback_destroy(ready_cb);
			}
			ready_cb = wl_surface_frame (surface);
			wl_callback_add_listener(ready_cb, &frame_limiter_listener, &ready);

			activeBuffer = (activeBuffer + 1) & 1;
			
			// Render
			
			wl_surface_attach(surface, buffer[activeBuffer], 0, 0);
			wl_surface_damage(surface, 0, 0, width, height);
			wl_surface_commit(surface);			
		}
		wl_display_dispatch(display);
#else
		// Block until all pending request are processed by the server.
		wl_display_roundtrip(display);
		
		// Dispatch default queue events without reading from the display fd.
		//wl_display_dispatch(display);
		
		for (uint32_t x = 0; x < width; x++) {
			for (uint32_t y = 0; y < height; y++) {
				*((uint32_t*)(data + size * activeBuffer + y * stride + x * 4)) = value;
			}
		}
		++value;

		wl_surface_attach(surface, buffer[activeBuffer], 0, 0);
		wl_surface_damage(surface, 0, 0, width, height);
		wl_surface_commit(surface);
#endif
	}

	for (uint32_t index = 0; index < count; ++index) {
		wl_buffer_destroy(buffer[index]);
	}
	wl_shm_pool_destroy(pool);
	munmap(data, totalSize);
	close(fd);
#if defined(XDG)
	zxdg_surface_v6_destroy(xdg_surface);
#else
	wl_shell_surface_destroy(shell_surface);
#endif
	wl_surface_destroy(surface);
	wl_keyboard_destroy(context.keyboard);
	wl_pointer_destroy(context.pointer);
	wl_seat_destroy(context.seat);
#if defined(XDG)
	zxdg_shell_v6_destroy(context.xdg_shell);
#else
	wl_shell_destroy(context.shell);
#endif
	wl_shm_destroy(context.shm);
	wl_compositor_destroy(context.compositor);
	wl_registry_destroy(registry);
	wl_display_disconnect(display);
	
	return 0;
}
