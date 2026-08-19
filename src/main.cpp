// greyproject1 - Pong
//
// Structure of this file, top to bottom:
//
//     1. constants        - the tuning knobs (sizes, speeds)
//     2. types            - what a Paddle is, what a Ball is
//     3. state            - the actual paddles and ball, living OUTSIDE main
//     4. helper functions - small jobs, one per function
//     5. UpdateDrawFrame  - the body of ONE frame
//     6. main             - setup, then call UpdateDrawFrame over and over
//
// Why is the state outside main and the frame body in its own function?
// Because browsers won't let a program sit in an infinite loop - a web build
// has to hand ONE frame back to the browser at a time. Writing it this way now
// means a web export later is a config change, not a rewrite. It also happens
// to be tidier.

#include "raylib.h"
#if defined(__EMSCRIPTEN__) 
    #include <emscripten/emscripten.h>
#endif

// ---------------------------------------------------------------------------
// 1. Constants
// ---------------------------------------------------------------------------
// `constexpr` means "known at compile time, never changes". Prefer it to
// scattering magic numbers through the code - when the ball feels too slow,
// you want ONE place to change.

constexpr int   SCREEN_WIDTH   = 960;
constexpr int   SCREEN_HEIGHT  = 540;

constexpr float PADDLE_WIDTH   = 16.0f;
constexpr float PADDLE_HEIGHT  = 100.0f;
constexpr float PADDLE_SPEED   = 400.0f;   // pixels per second
constexpr float PADDLE_MARGIN  = 40.0f;    // gap between paddle and screen edge

constexpr float BALL_RADIUS    = 10.0f;
constexpr float BALL_SPEED     = 350.0f;   // starting speed, pixels per second

constexpr int   WINNING_SCORE  = 11;

// ---------------------------------------------------------------------------
// 2. Types
// ---------------------------------------------------------------------------
// A `struct` bundles related variables under one name. It's a Java class with
// everything public and no methods - which is all we need here.

struct Paddle
{
    Rectangle rect;     // raylib's { x, y, width, height }. Position AND size.
    int       score;
    int       upKey;    // which key moves this paddle up   (e.g. KEY_W)
    int       downKey;  // which key moves this paddle down (e.g. KEY_S)
};

struct Ball
{
    Vector2 position;   // raylib's { x, y } - the CENTRE of the ball
    Vector2 velocity;   // pixels per second along x and y. Direction + speed
                        // in one value: velocity.x < 0 means moving left.
};

enum GameState {
    PLAYING,
    PLAYER_ONE_WINS,
    PLAYER_TWO_WINS
};

// ---------------------------------------------------------------------------
// 3. State
// ---------------------------------------------------------------------------
// These live at "file scope" - created once when the program starts, visible
// to every function below. In a bigger project you'd avoid globals, but for a
// single-screen game they're the honest, simple choice.

Paddle leftPaddle;
Paddle rightPaddle;
Ball   ball;
Color white = { 255, 255, 255, 255 };
GameState state;

// ---------------------------------------------------------------------------
// 4. Helper functions
// ---------------------------------------------------------------------------
// Note the `&` in parameters like `Paddle& paddle`. That's a REFERENCE: the
// function receives the caller's actual paddle, not a copy, so changes stick.
// Without the `&` you'd modify a throwaway copy and nothing would happen -
// a classic C++ first-week bug. (Java gave you this behaviour for free on
// objects; C++ makes you ask for it.)

// Put the ball back in the centre and send it moving again.
// `directionX` should be -1 (serve toward the left player) or +1 (toward the
// right). Called at the start of the game and after every point.
void ResetBall(int directionX) {
    ball.position = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f }; 
    // since Vector2 is just a struct not a Java class there is nothing to call,
    // and so we just assign a different struct. 
    // Vector2{ float x, float y } and ball.position.x = x work too
    
    ball.velocity.x = BALL_SPEED * directionX;
    ball.velocity.y = GetRandomValue(-200, 200);
}

// Set up both paddles and serve the first ball. Called once before the loop,
// and again when the players restart after a win.
void InitGame() {

    state = PLAYING;

    leftPaddle = {
        { // rect
            PADDLE_MARGIN,
            (SCREEN_HEIGHT - PADDLE_HEIGHT) / 2.0f,
            PADDLE_WIDTH,
            PADDLE_HEIGHT
        },
        0, // score
        KEY_W, // UpKey
        KEY_S // downKey
    };

    rightPaddle = {
        { // rect
            SCREEN_WIDTH - PADDLE_MARGIN - PADDLE_WIDTH,
            (SCREEN_HEIGHT - PADDLE_HEIGHT) / 2.0f,
            PADDLE_WIDTH,
            PADDLE_HEIGHT
        },
        0, // score
        KEY_UP, // UpKey
        KEY_DOWN // downKey
    };

    int serveDir = 0;
    do {
        serveDir = GetRandomValue(-1, 1);
    } while (serveDir == 0);
    ResetBall(serveDir);

}

// Move one paddle according to its own keys, then stop it leaving the screen.
void UpdatePaddle(Paddle& paddle, float dt)
{
    if (IsKeyDown(paddle.upKey)) paddle.rect.y -= PADDLE_SPEED * dt;
    if (IsKeyDown(paddle.downKey)) paddle.rect.y += PADDLE_SPEED * dt;

    if (paddle.rect.y > SCREEN_HEIGHT - PADDLE_HEIGHT) paddle.rect.y = SCREEN_HEIGHT - PADDLE_HEIGHT;
    if (paddle.rect.y < 0) paddle.rect.y = 0;
}

// Move the ball and handle everything it can hit.
// Making the bounce angle depend on WHERE the ball
// hit the paddle is the single best upgrade afterwards, but get it working
// first.
void UpdateBall(float dt)
{
    ball.position.x += ball.velocity.x * dt;
    ball.position.y += ball.velocity.y * dt;
    if (ball.position.y - BALL_RADIUS < 0) {
        ball.velocity.y *= -1;
        ball.position.y = 0 + BALL_RADIUS;
    }
    else if (ball.position.y + BALL_RADIUS > SCREEN_HEIGHT) {
        ball.velocity.y *= -1;
        ball.position.y = SCREEN_HEIGHT - BALL_RADIUS;
    }

    // Paddle Collision
    if  (CheckCollisionCircleRec(ball.position, BALL_RADIUS, leftPaddle.rect)) { // Left Paddle   
        ball.velocity.x *= -1.10f;
        ball.position.x = leftPaddle.rect.width + PADDLE_MARGIN + BALL_RADIUS;
    }
    else if (CheckCollisionCircleRec(ball.position, BALL_RADIUS, rightPaddle.rect)) { // Right Paddle 
        ball.velocity.x *= -1.10f;
        ball.position.x = SCREEN_WIDTH - rightPaddle.rect.width - PADDLE_MARGIN - BALL_RADIUS;
    } 

    
    // scoring
    if (ball.position.x + BALL_RADIUS <= 0) {
        rightPaddle.score++;
        if (rightPaddle.score >= WINNING_SCORE && rightPaddle.score >= leftPaddle.score + 2) state = PLAYER_TWO_WINS;
        else ResetBall(-1);
    } else if (ball.position.x - BALL_RADIUS >= SCREEN_WIDTH) {
        leftPaddle.score++;
        if (leftPaddle.score >= WINNING_SCORE && leftPaddle.score >= rightPaddle.score + 2) state = PLAYER_ONE_WINS;
        else ResetBall(1);
    }
}

// Draw the court: centre line, both scores, and the help text.
// Drawing is separate from updating on purpose - update decides what's true,
// draw just reports it. Keep the two from bleeding into each other.
void DrawCourt()
{    
    // Dashed Centreline
    for (float y = 0.0f; y < SCREEN_HEIGHT; y += 50.0f) {
        DrawRectangleRec({ SCREEN_WIDTH / 2 - 5, y, 10, 50 }, white);
        y += 10;
    }

    //Left Player Score
    DrawRectangleLinesEx({ 0, 0, 150, 60 }, 2, white);
    DrawText(TextFormat("%d", leftPaddle.score), 80, 20, 20, white);

    //Right Player Score
    DrawRectangleLinesEx({ SCREEN_WIDTH - 150, 0, 150, 60 }, 2, white);
    DrawText(TextFormat("%d", rightPaddle.score), SCREEN_WIDTH - 80, 20, 20, white);

    // Gameover
    if (state == PLAYER_ONE_WINS) {
        DrawRectangleRec({ 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT }, { 50, 50, 50, 100 });
        DrawRectangleLinesEx({ SCREEN_WIDTH / 2 - 300, SCREEN_HEIGHT / 2 - 75, 600, 150 }, 4, white);
        DrawText("Player One Wins! - Press Space", SCREEN_WIDTH / 2 - 250, SCREEN_HEIGHT / 2 - 15, 30, white);
    } else if (state == PLAYER_TWO_WINS) {
        DrawRectangleRec({ 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT }, { 50, 50, 50, 100 });
        DrawRectangleLinesEx({ SCREEN_WIDTH / 2 - 300, SCREEN_HEIGHT / 2 - 75, 600, 150 }, 4, white);
        DrawText("Player Two Wins!- Press Space", SCREEN_WIDTH / 2 - 250, SCREEN_HEIGHT / 2 - 15, 30, white);
    }

}

// ---------------------------------------------------------------------------
// 5. One frame
// ---------------------------------------------------------------------------
// Input -> update -> draw. Everything that happens in 1/60th of a second.
void UpdateDrawFrame()
{
    float dt = GetFrameTime();
    if (state == PLAYING) {

        UpdatePaddle(leftPaddle, dt);
        UpdatePaddle(rightPaddle, dt);
        UpdateBall(dt);
    }
    else {
        if (IsKeyPressed(KEY_SPACE)) InitGame();
    }

    BeginDrawing();
    ClearBackground({ 50, 50, 50, 255 }); // also draws the BG

    DrawCourt();
    DrawRectangleRec(leftPaddle.rect, white);
    DrawRectangleRec(rightPaddle.rect, white);
    DrawCircleV(ball.position, BALL_RADIUS, white);
    

    EndDrawing(); // always redraw even with no updtaes as this is what 
    // actually does a frame. Without it it cannot poll for input (spacebar)
}

// ---------------------------------------------------------------------------
// 6. Entry point
// ---------------------------------------------------------------------------

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pong");
    InitGame();

#if defined(__EMSCRIPTEN__)
    // TODO: hand UpdateDrawFrame to the browser, then fall through and return.
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
    //
    //   arg 1: the function to run each frame. Pass it by NAME, with no
    //          parentheses - you're passing the function ITSELF, not calling it
    //          and passing its result. (This is a function pointer. Closest
    //          thing you've seen is a Java method reference.)
    //   arg 2: target fps. Pass 0 to let the browser drive, syncing to the
    //          monitor's refresh rate. That's what you want here.
    //   arg 3: pass 1.
    //
    // Deliberately no SetTargetFPS on this path - two things throttling the
    // frame rate fight each other. Let the browser own the timing.
    // Deliberately no CloseWindow either: the page closing IS the shutdown.
#else
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        UpdateDrawFrame();
    }

    CloseWindow();
#endif
    return 0;
}
