#include <raylib.h>

int main(void) {
	const int screenWidth = 800;
	const int screenHeight = 450;

	InitWindow(screenWidth, screenHeight, "scripteroids");

	SetTargetFPS(60);

	// Detect window close button or ESC key
	while (!WindowShouldClose()) {
		BeginDrawing();

		ClearBackground(RAYWHITE);

		DrawText("welcome to scripteroids", 190, 200, 20, LIGHTGRAY);

		EndDrawing();
	}

	// Close window and OpenGL context
	CloseWindow();

	return 0;
}

