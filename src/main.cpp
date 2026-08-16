// greyproject1 - raylib starter
//
// Every game, no matter how complex, is this shape:
//
//     setup
//     while (the window is still open):
//         1. read input
//         2. update the world a tiny bit
//         3. draw the world
//     cleanup
//
// That loop runs ~60 times per second. Nothing "moves" on its own - things
// move because each pass through the loop nudges them slightly and redraws.

#include "raylib.h"

int main()
{
    const int screenWidth  = 960;
    const int screenHeight = 540;

    InitWindow(screenWidth, screenHeight, "greyproject1");
    SetTargetFPS(60);   // don't run the loop faster than 60 times per second

    // A Rectangle in raylib is just { x, y, width, height }.
    // Note that y increases DOWNWARD - (0,0) is the top-left of the window.
    Rectangle player = { screenWidth / 2.0f - 25, screenHeight / 2.0f - 25, 50, 50 };

    const float playerSpeed = 300.0f;   // pixels per SECOND, not per frame

    while (!WindowShouldClose())        // false once you hit ESC or close the window
    {
        // -- 1 & 2: input and update ---------------------------------------
        //
        // GetFrameTime() is how long the previous frame took, in seconds
        // (~0.016 at 60fps). Multiplying speed by it is called "delta time",
        // and it's why the square moves at the same real-world speed whether
        // the game runs at 30fps or 144fps. Forget it, and your game runs at
        // different speeds on different computers.
        float dt = GetFrameTime();

        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) player.x += playerSpeed * dt;
        if (IsKeyDown(KEY_LEFT)  || IsKeyDown(KEY_A)) player.x -= playerSpeed * dt;
        if (IsKeyDown(KEY_DOWN)  || IsKeyDown(KEY_S)) player.y += playerSpeed * dt;
        if (IsKeyDown(KEY_UP)    || IsKeyDown(KEY_W)) player.y -= playerSpeed * dt;

        // Keep the player inside the window.
        if (player.x < 0) player.x = 0;
        if (player.y < 0) player.y = 0;
        if (player.x + player.width  > screenWidth)  player.x = screenWidth  - player.width;
        if (player.y + player.height > screenHeight) player.y = screenHeight - player.height;

        // -- 3: draw --------------------------------------------------------
        //
        // Everything between BeginDrawing and EndDrawing is one frame. You
        // repaint the ENTIRE screen every frame, starting from a blank
        // background - you never "erase" the old square, you just stop
        // drawing it in the old place.
        BeginDrawing();

            ClearBackground(RAYWHITE);
            DrawRectangleRec(player, MAROON);
            DrawText("WASD / arrow keys to move  -  ESC to quit", 16, 16, 20, DARKGRAY);
            DrawFPS(screenWidth - 90, 16);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
