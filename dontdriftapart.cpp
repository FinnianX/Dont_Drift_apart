#include <iostream>
#include <string>
#include "raylib.h"
#include <format>
#include <cmath>
#include <ctime>
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include <vector>
#include <random>
#include <algorithm>

// Initalize random number generator
std::random_device rd;
std::mt19937 generator(rd());
std::uniform_int_distribution<int> distribution(1, 70);
// Initalize constants
Color base = { 255, 0, 0, 255 };
using namespace std;
const int screenWidth = 1920;
const int screenHeight = 1080;
bool showIntro = true;
// Define ball struct and initiate default instances
struct Ball {
	float x;
	float y;
	float velocity;
	Color color;
	int radius;
};
Ball ball = { screenWidth / 2.0f,screenHeight / 2.0f,2.0f,YELLOW,35 };
Ball bullet = { ball.x + screenWidth / 2 , ball.y, 0, RED, (int)ball.radius / 2 };
// Utililty functions
bool overlap(float x1, float y1, int r1,
	float x2, float y2, int r2) {
	float dx = x2 - x1;
	float dy = y2 - y1;
	float distSquared = dx * dx + dy * dy;
	float radii = r1 + r2;
	return distSquared <= radii * radii;
}
vector<int> returnLevelChallenge(int difficulty, int level) {
	int time = 60;
	int points = 20 + 6 * (level - 1) * (level - 1);
	return vector<int>{ time / difficulty, points * difficulty };
};
void createDuplicate(vector<Ball>& dupe, float x, float y, float velocity, Color color, int radius) {
	dupe.push_back({ x,y,velocity,color,radius });
};
void renderAllDuplicates(vector<Ball>& dupe) {
	for (int i = 0; i < dupe.size(); i++) {
		DrawCircleV(Vector2{ dupe[i].x, dupe[i].y }, dupe[i].radius, dupe[i].color);
	};
};
void keepDuplicatesInBounds(vector<Ball>& dupe) {
	for (int i = 0; i < dupe.size(); i++) {
		if (dupe[i].x + dupe[i].radius > screenWidth)
		{
			dupe[i].x = screenWidth - dupe[i].radius;
			dupe[i].velocity = dupe[i].velocity * -1;
		}
		if (dupe[i].x - dupe[i].radius < 0)
		{
			dupe[i].x = dupe[i].radius;
			dupe[i].velocity = dupe[i].velocity * -1;
		}
	};
}
void renderCenterText(string text, int fontsize, Color color) {
	int textWidth = MeasureText(text.c_str(), fontsize);
	DrawText(text.c_str(), (screenWidth - textWidth) / 2, (screenHeight - fontsize) / 2, fontsize, color);
};
void fadeOutText(string text, int fontsize, Color color, float progress) {
	Color fadedColor = Fade(color, 1.0f - progress);
	renderCenterText(text, fontsize, fadedColor);
};
void updateDuplicateState(vector<Ball>& dupe) {
	uniform_real_distribution<float> chance(0.0f, 1.0f);
	uniform_real_distribution<float> change(-2.0f, 2.0f);
	for (Ball& b : dupe) {
		if (chance(generator) < 0.15f) {
			b.velocity += change(generator);
		}
		b.velocity = std::clamp(b.velocity, -10.0f, 10.0f);
		b.x += b.velocity;
		b.radius = bullet.radius;
	}
}
void modifyDuplicateVelocity(vector<Ball>& dupe, float amount) {
	for (int i = 0; i < dupe.size(); i++) {
		dupe[i].velocity += amount;
	};
}
void endGame(int level) {
	renderCenterText(format("GAME OVER. You made it to level {} . Press R to restart.", level), 40, GRAY);
}
int main()
{
	// Create window instance
	InitWindow(screenWidth, screenHeight, "Don't Drift Apart");
	SetTargetFPS(60);
	// magic numbers and bools "CoNfIg"
	float elapsedTime = 0.0f; // Time since level start
	float points = 0.0;
	bool paused = false;
	bool gameover = false;
	bool stopped = false; // Y-Axis ball at bottom of screen
	bool btnUpgradeRadius = false; // Upgrade button clicked
	bool btnUpgradePointRate = false; // Upgrade button clicked
	float upgradeCost = 5; // Upgrade cost; increases after each upgrade
	float pointRate = 0.1; // Points per game tick of overlap with non duplicate balls
	bool btnCreateDuplicate; // Upgrade button clicked
	vector<Ball> duplicates; // Vector of all duplicates
	float fadeProgress = 0.0f; 
	float fadeDuration = 3.0f;
	int level = 1; 
	bool difficultySet = false; // Passed through difficulty selector (True | False)
	const char* difficulties = "Easy;Medium;Hard"; // All difficulty options
	int difficulty=0; // Difficulty
	int difficultyIndex = 0; // Filler value  
	while (!WindowShouldClose())
	{
		// Gameover screen
		if (gameover) {
			BeginDrawing();
			ClearBackground(RAYWHITE);
			endGame(level);
			if (IsKeyPressed(KEY_ESCAPE)) {
				break;
			};
			EndDrawing();
			// Reset game state to beginning on restart
			if (IsKeyPressed(KEY_R)) {
				points = 0.0f;
				duplicates.clear();
				level = 1;
				fadeProgress = 0.0f;
				pointRate = 0.1f;
				upgradeCost = 5.0f;
				elapsedTime = 0.0f;
				ball = { screenWidth / 2.0f, screenHeight / 2.0f,
						 2.0f, YELLOW, 35 };
				bullet = { ball.x, ball.y, 0.0f, RED, ball.radius / 2 };
				stopped = false;
				gameover = false;
			}
			continue;
		};
		// Difficulty screen
		if (!difficultySet) {
			BeginDrawing();
			ClearBackground(RAYWHITE);
			renderCenterText("Choose your difficulty:", 30, GRAY);
			Rectangle combo{ screenWidth / 2 - 150, screenHeight / 2 + 60, 300, 40 };
			GuiComboBox(combo, difficulties, &difficultyIndex);
			Rectangle startBtn{ screenWidth / 2 - 100, screenHeight / 2 + 150, 200, 50 };
			if (GuiButton(startBtn, "START")) {
				difficulty = difficultyIndex + 1;
				difficultySet = true;
			}
			EndDrawing();
			continue;
		};
		// Intro screen
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
		// Pause screen
		if (paused) {
			if (IsKeyPressed(KEY_SPACE)) {
				paused = !paused;
			}
			BeginDrawing();
			ClearBackground(RAYWHITE);
			renderCenterText("Press space to unpause", 60, GRAY);
			EndDrawing();
		};
		// Main game loop
		if (!paused) {
			BeginDrawing();
			vector<int> currentChallenge = returnLevelChallenge(difficulty, level);
			elapsedTime += GetFrameTime();
			if (elapsedTime >= currentChallenge[0] && points < currentChallenge[1]) {
				gameover = true;
				EndDrawing();
				continue;
			};
			if (elapsedTime >= currentChallenge[0] && points >= currentChallenge[1]) {
				level += 1;
				fadeProgress = 0;
				elapsedTime = 0.0f;
			};
			fadeProgress += GetFrameTime() / fadeDuration;
			if (fadeProgress >= 1.0f) { fadeProgress = 1.0f; };
			fadeOutText(format("LEVEL {}: Get {} points in {:.1f} minutes!", level, currentChallenge[1], (float)currentChallenge[0]/60), 40, GRAY, fadeProgress);
			// Define upgrade buttons
			btnUpgradeRadius = GuiButton(Rectangle{ screenWidth / 100, screenHeight / 25, 158, 40 }, std::format("UPGRADE: CIRCLE RADIUS\nPoints: {}", upgradeCost).c_str());
			btnUpgradePointRate = GuiButton(Rectangle{ screenWidth / 100, screenHeight / 10, 158, 40 }, std::format("UPGRADE: POINT RATE\nPoints: {}", upgradeCost).c_str());
			btnCreateDuplicate = GuiButton(Rectangle{ screenWidth / 100, screenHeight / 6, 158, 40 }, std::format("CREATE DUPLICATE BALL", upgradeCost).c_str());
			// Menu and upgrades
			if (btnUpgradeRadius && points >= upgradeCost) {
				bullet.radius += 2;
				points = points - upgradeCost;
				upgradeCost = upgradeCost + 5;
			};
			if (btnCreateDuplicate) {
				int r = distribution(generator);
				int g = distribution(generator);
				int b = distribution(generator);
				Color base = { 255 - r, 0 +g, 0+b, 255 };
				createDuplicate(duplicates, bullet.x, bullet.y, 2.0f,base, bullet.radius);
				btnCreateDuplicate = false;
				if (upgradeCost >= 2){
					upgradeCost -= 2;
				}
				pointRate += 0.2;
			}
			if (btnUpgradePointRate && points >= upgradeCost) {
				pointRate += 0.1;
				points = points - upgradeCost;
				upgradeCost = upgradeCost + 5;
			};
			// Check for pause
			if (IsKeyPressed(KEY_SPACE)) {
				paused = !paused;
			}
			// Accelerate x-axis ball to edge of screen
			if (bullet.x <= screenWidth / 2) {
				bullet.velocity -= 0.05;
			};
			if (bullet.x > screenWidth / 2) {
				bullet.velocity += 0.05;
			};
			// Input handling for x-axis ball
			if (IsKeyDown(KEY_RIGHT) && stopped == false) {
				bullet.velocity += 1.5f;
			};
			if (IsKeyDown(KEY_LEFT) && stopped == false) {
				bullet.velocity -= 1.5f;
			};
			bullet.x += bullet.velocity;
			// Keep x-axis ball in bounds
			if (bullet.x + bullet.radius > screenWidth){bullet.x = screenWidth - bullet.radius; bullet.velocity = bullet.velocity * -0.8;}
			if (bullet.x - bullet.radius < 0){bullet.x = bullet.radius;bullet.velocity = bullet.velocity * -0.8;}
			// Gravity-like downwards force for y-axis ball
			ball.velocity += 0.7f;
			// Input handling for y-axis ball
			if (IsKeyDown(KEY_UP)) {
				ball.velocity -= 2;
				stopped = false;
			};
			ball.y += ball.velocity;
			// Keep y-axis ball in bounds
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
			// Main scoring
			if (overlap(ball.x, ball.y, ball.radius, bullet.x, bullet.y, bullet.radius)) { points += pointRate; };
			// Clear screen and render points
			ClearBackground(RAYWHITE);
			DrawText(TextFormat("Points: %.1f", points), 10, 10, 20, GRAY);
			DrawText(TextFormat("Time left: %.1f", currentChallenge[0] - elapsedTime),20 + MeasureText(TextFormat("Points: %.1f", points), 20),10, 20, GRAY);			
			// Render non duplicate balls
			DrawCircleV(Vector2{ ball.x, ball.y }, ball.radius, ball.color);
			DrawCircleV(Vector2{ bullet.x, bullet.y }, bullet.radius, bullet.color);
			// Duplicate handling
			renderAllDuplicates(duplicates);
			keepDuplicatesInBounds(duplicates);
			updateDuplicateState(duplicates);
			EndDrawing();
		};
	};
	CloseWindow();
	return 0;
}
