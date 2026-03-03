#include "raylib.h"

int main()
{
    const int screenWidth = 640;
    const int screenHeight = 480;

    char* mapping = LoadFileText("gamecontrollerdb.txt");

    SetGamepadMappings(mapping);

    UnloadFileText(mapping);
    
    InitWindow(screenWidth, screenHeight, "raylib cmake hello world");

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        BeginDrawing();

        ClearBackground(RAYWHITE);

        DrawText("Hello world! It's great to be here!", 190, 200, 20, LIGHTGRAY);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}