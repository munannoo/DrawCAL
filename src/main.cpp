#include "raylib.h"
int main(){
    InitWindow(800, 600, "Hello World");
    while (!WindowShouldClose()){
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Hello World", 190, 200, 20, LIGHTGRAY);
        EndDrawing();
    }
    CloseWindow();
}