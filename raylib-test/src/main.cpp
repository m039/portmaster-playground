#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

#include <stdio.h>

Model model;

char message[128];

const char * ModelFilename = "assets/planet.glb";

const char * GamepadKeyNames[] = {
    "Unkown",
    "Up",
    "Right",
    "Down",
    "Left",
    "Y",
    "B",
    "A",
    "X",
    "L1",
    "L2",
    "R1",
    "R2",
    "Select",
    "Guide",
    "Start",
    "Left Thumb",
    "Right Thumb"
};

int main()
{
    const int screenWidth = 640;
    const int screenHeight = 480;

    if (!FileExists(ModelFilename)) {
        TraceLog(LOG_ERROR, "Can't find '%s'\n", ModelFilename);
        return 1;
    }
    
    InitWindow(screenWidth, screenHeight, "raylib-test");

    model = LoadModel(ModelFilename);

    SetTargetFPS(60);

    sprintf(message, "Press a Key");

    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 20.0f, 0.0f };   // Позиция камеры
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Точка, на которую смотрит камера
    camera.up = (Vector3){ 1.0f, 0.0f, 0.0f };          // Вектор «вверх»
    camera.fovy = 20.0f;                                // Orthographic width/height
    camera.projection = CAMERA_ORTHOGRAPHIC;            // Set to orthographic

    float angleX, angleZ;

    int gamepad = 0;

    while (!WindowShouldClose())
    {
        int key = GetKeyPressed();
        if (key > 0) {
            sprintf(message, "Key Pressed: %s", GetKeyName(key));
        }
        key = GetGamepadButtonPressed();
        if (key > 0) {
            sprintf(message, "Gamepad's Key Pressed: %s", GamepadKeyNames[key]);
        }

        angleX = - 180 * GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_X);
        angleZ = - 180 * GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_Y);

        if (IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_MIDDLE_LEFT) && IsGamepadButtonDown(0, GAMEPAD_BUTTON_MIDDLE_RIGHT)) {
            break;
        }

        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                rlPushMatrix();
                    rlRotatef(angleX, 1.0, 0.0, 0.0);
                    rlRotatef(angleZ, 0.0, 0.0, 1.0);
                    DrawModel(model, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, BLACK);
                rlPopMatrix();

            EndMode3D();

            DrawText(message, 40, 40, 20, BLUE);

        EndDrawing();
    }

    UnloadModel(model);
    CloseWindow();

    return 0;
}