#include "wl_window.h"

#include <stdio.h>
#include <time.h>

uint64_t time_ms(void) {
	struct timespec currentTime;
	clock_gettime(CLOCK_MONOTONIC_RAW, &currentTime);
	return (currentTime.tv_sec * 1000 + currentTime.tv_nsec / 1000000);
}

int main(void) {
	wl_context context = {0};
	wl_window window = {0};

	if (wl_connect(&context) != 0) {
		printf("Error: cannot connect to display server\n");
		return 1;
	}
	
	if (wl_window_create(&context, &window, 0, 0, 800, 600) != 0) {
		printf("Error: cannot create window\n");
		return 2;
	}

	// Frame counter
	uint64_t currentTime = time_ms();
	uint64_t lastFrameTime = currentTime;
	uint32_t frameCount = 0;

	while (context.should_close == false) {
		uint32_t* pixels = wl_window_get_active_buffer(&window);
		uint8_t value = (currentTime / 10) % 0xFF;
		for (uint32_t y = 0; y < window.height; ++y) {
			for (uint32_t x = 0; x < window.width; ++x) {
				pixels[y * window.width + x] = 0xFF000000 | value << 16 | value << 8 | value;
			}
		}
			
		wl_window_swap_buffers(&window, true);

		++frameCount;
		currentTime = time_ms();
		const uint64_t timeDiff = currentTime - lastFrameTime;
		if (timeDiff >= 1000UL) {
			printf("FPS: %.1f\n", (double)(frameCount) / (double)timeDiff * 1000.0);
			frameCount = 0;
			lastFrameTime += timeDiff;
		}
	}

	wl_window_destroy(&window);
	wl_disconnect(&context);
	return 0;
}
