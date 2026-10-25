#include <iostream>
#include <string>
#include "raylib.h"
#include <format>
#include <cmath>
#include <ctime>
using namespace std;
const int screenWidth = 1920;
const int screenHeight = 1080;
bool showIntro = true;
bool overlap(float x1, float y1, int r1,
	float x2, float y2, int r2) {
	float dx = x2 - x1;
	float dy = y2 - y1;
	float distSquared = dx * dx + dy * dy;
	float radii = r1 + r2;
	return distSquared <= radii * radii;
}
int main()
{
	InitWindow(screenWidth, screenHeight, "Don't Drift Apart");
	SetTargetFPS(60);
	float elapsedTime = 0.0f;
	float points = 0.0;
	struct Ball {
		float x;
		float y;
		float velocity;
		Color color;
		int radius;
	};
	Ball ball = {
		screenWidth / 2.0f,
		screenHeight / 2.0f,
		2.0f,
		YELLOW,
		35
	};
	Ball bullet = { ball.x + screenWidth / 2 , ball.y, 0, RED, (int)ball.radius / 2};
	bool paused = false;
	bool gameover = false;
	bool stopped = false;
	while (!WindowShouldClose())
	{
		if (showIntro) {
			BeginDrawing();
			ClearBackground(RAYWHITE);

			const char* text =
				"Goal: keep both balls together as long as possible.\n"
				"Use the arrow keys to control the balls.\n"
				"Press SPACE to start.";

			DrawText(text, 100, 400, 30, GRAY);
			EndDrawing();

			if (IsKeyPressed(KEY_SPACE)) {
				showIntro = false;
			}

			continue;
		}
		if (paused) {
			if (IsKeyPressed(KEY_SPACE)) {
				paused = !paused;
			}
			BeginDrawing();
			ClearBackground(RAYWHITE);
			const char* text = "Press space to unpause";
			int fontSize = 60;
			int textWidth = MeasureText(text, fontSize);
			DrawText(text, (screenWidth - textWidth) / 2, (screenHeight - fontSize) / 2, fontSize, GRAY);
			EndDrawing();
		};
		if (!paused && !gameover) {
			BeginDrawing();
			elapsedTime += GetFrameTime();
			if (bullet.x <= screenWidth/2) {
				bullet.velocity -= 0.05;
			};
			if (bullet.x > screenWidth / 2) {
				bullet.velocity += 0.05;
			};
			if (elapsedTime >= 60.0f) {
				ClearBackground(RAYWHITE);
				gameover = true;
			};
			if (IsKeyPressed(KEY_SPACE)) {
				paused = !paused;
			}
			if (IsKeyDown(KEY_RIGHT) && stopped==false) {
				bullet.velocity += 1.5f;
			};
			bullet.x += bullet.velocity;
			if (IsKeyDown(KEY_LEFT) && stopped==false) {
				bullet.velocity -= 1.5f;
			};
			if (bullet.x + bullet.radius > screenWidth)
			{
				bullet.x = screenWidth - bullet.radius;
				//bullet.velocity = 0.0f; No bounce variant
				bullet.velocity = bullet.velocity * -0.8;
			}
			if (bullet.x - bullet.radius < 0)
			{
				bullet.x = bullet.radius;
				//bullet.velocity = 0.0f; No bounce variant
				bullet.velocity = bullet.velocity * -0.8;
			}
			ball.velocity += 0.7f;
			if (IsKeyDown(KEY_UP)) {
				ball.velocity -= 2;
				stopped = false;
			};
			ball.y += ball.velocity;
			if (ball.y + ball.radius > screenHeight)
			{
				ball.y = screenHeight - ball.radius;
				ball.velocity = 0.0f;
				stopped = true;
			}
			if (ball.y - ball.radius < 0)
			{
				ball.y = ball.radius;
				ball.velocity = 0.0f;
			}
			if (overlap(ball.x, ball.y, ball.radius, bullet.x, bullet.y, bullet.radius)) { points += 0.1; };
			ClearBackground(RAYWHITE);
			DrawText(TextFormat("Points: %.1f", points), 10, 10, 20, GRAY);
			DrawCircleV(Vector2{ ball.x, ball.y }, ball.radius, ball.color);
			DrawCircleV(Vector2{ bullet.x, bullet.y }, bullet.radius, bullet.color);
			EndDrawing();
		};
		if (gameover) {
			BeginDrawing();
			ClearBackground(RAYWHITE);
			const char* title = "Game Over";
			int titleSize = 70;
			int fontSize = 40;
			DrawText(title, 100, 380, titleSize, GRAY);
			DrawText(TextFormat("Points: %.1f", points),100, 480, fontSize, GRAY);
			DrawText("Press R to restart",100, 540, fontSize, GRAY);
			EndDrawing();
		}
		if (gameover && IsKeyPressed(KEY_R)) {
			elapsedTime = 0.0;
			points = 0.0;
			gameover = false;
			ball.x = screenWidth / 2.0f;
			ball.y = screenHeight / 2.0f;
			ball.velocity = 2.0f;

			bullet.x = ball.x + screenWidth / 2;
			bullet.y = ball.y;
			bullet.velocity = 0.0f;
		};
	};
	CloseWindow();
	return 0;
}
