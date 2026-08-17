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

// ---------------------------------------------------------------------------
// 1. Constants
// ---------------------------------------------------------------------------
// `constexpr` means "known at compile time, never changes". Prefer it to
// scattering magic numbers through the code - when the ball feels too slow,
// you want ONE place to change.

constexpr int   SCREEN_WIDTH   = 960;
constexpr int   SCREEN_HEIGHT  = 540;

constexpr int   CLAMP_MARGIN   = 3;

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
// bool   gameOver = false;

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
    ball.velocity.y = GetRandomValue(-100, 100);
}

// Set up both paddles and serve the first ball. Called once before the loop,
// and again when the players restart after a win.
void InitGame() {

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
// Works for BOTH paddles because the keys are stored in the struct - this is
// why Paddle has upKey/downKey fields instead of hard-coded input.
void UpdatePaddle(Paddle& paddle, float dt)
{
    if (IsKeyDown(paddle.upKey)) paddle.rect.y -= PADDLE_SPEED * dt;
    if (IsKeyDown(paddle.downKey)) paddle.rect.y += PADDLE_SPEED * dt;

    if (paddle.rect.y > SCREEN_HEIGHT) paddle.rect.y = SCREEN_HEIGHT;
    if (paddle.rect.y < 0) paddle.rect.y = 0;
}

// Move the ball and handle everything it can hit.
//
// TODO, roughly in this order:
//   - move position by velocity * dt
//   - bounce off the top and bottom walls: if the ball has gone past an edge,
//     flip velocity.y. (Nudge it back inside the wall too, or a ball that
//     lands exactly on the edge can flip every frame and jitter.)
//   - bounce off paddles. CheckCollisionCircleRec(centre, radius, rect)
//     returns true when a circle overlaps a rectangle. On a hit, flip
//     velocity.x.
//     Guard against re-hitting: only flip if the ball is actually moving
//     TOWARD that paddle, otherwise it can get stuck vibrating inside one.
//   - if the ball has left the screen past a paddle, that's a point for the
//     other player: increment their score, then ResetBall toward the loser.
//   - if someone reached WINNING_SCORE, set gameOver
//
// Do the simple version first - a flat bounce that just flips velocity.x is a
// complete, playable game. Making the bounce angle depend on WHERE the ball
// hit the paddle is the single best upgrade afterwards, but get it working
// first.
void UpdateBall(float dt)
{
    ball.position.x += ball.velocity.x * dt;
    ball.position.y += ball.velocity.y * dt;
    if (ball.position.y <= 0) {
        ball.velocity.y *= -1;
        ball.position.y += CLAMP_MARGIN;
    }
    else if (ball.position.y >= SCREEN_HEIGHT) {
        ball.velocity.y *= -1;
        ball.position.y -= CLAMP_MARGIN;
    }

    // Paddle Collision (AABB)
    if  (
            ball.position.x - BALL_RADIUS >= PADDLE_MARGIN
            && ball.position.x - BALL_RADIUS <= PADDLE_MARGIN + PADDLE_WIDTH
            && ball.position.y >= leftPaddle.rect.y
            && ball.position.y <= leftPaddle.rect.y + PADDLE_HEIGHT
        ) 
    {
        ball.velocity.x *= -1;
        ball.position.x += CLAMP_MARGIN;
    }
    else if (
            ball.position.x + BALL_RADIUS >= SCREEN_WIDTH - PADDLE_MARGIN
            && ball.position.x + BALL_RADIUS <= SCREEN_WIDTH - PADDLE_MARGIN - PADDLE_WIDTH
            && ball.position.y >= rightPaddle.rect.y
            && ball.position.y <= rightPaddle.rect.y + PADDLE_HEIGHT
        )
    {
        ball.velocity.x *= -1;
        ball.position.x -= CLAMP_MARGIN;
    }

    // scoring
    if (ball.position.x + BALL_RADIUS <= 0) {
        rightPaddle.score++;
        ResetBall(1);
    } else if (ball.position.x - BALL_RADIUS <= 0) {
        leftPaddle.score++;
        ResetBall(-1);
    }

}

// Draw the court: centre line, both scores, and the help text.
// Drawing is separate from updating on purpose - update decides what's true,
// draw just reports it. Keep the two from bleeding into each other.
void DrawCourt()
{
    Color darkGrey = { 50, 50, 50, 255 };

    // Background
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, darkGrey);
    
    // Dashed Centreline
    for (int y = 0; y < SCREEN_HEIGHT; y += 50) {
        DrawRectangle(SCREEN_WIDTH / 2 - 5, y, 10, 50, white);
        y += 10;
    }

    //Left Player Score
    DrawRectangleLinesEx({ 0, 0, 100, 40 }, 2, white);
    DrawText(TextFormat("%d", leftPaddle.score), 20, 20, 20, white);

    //Right Player Score
    DrawRectangleLinesEx({ SCREEN_WIDTH - 100, 0, 100, 40 }, 2, white);
    DrawText(TextFormat("%d", rightPaddle.score), SCREEN_WIDTH - 80, 20, 20, white);

    // Gameover
    if (leftPaddle.score >= 11 && leftPaddle.score >= rightPaddle.score + 2) {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, { 50, 50, 50, 100 });
        DrawRectangleLinesEx({ SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 50, 200, 100 }, 4, white);
        DrawText(TextFormat("Player One Wins! - Press Space"), SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT / 2 - 15, 30, white);
    } else if (rightPaddle.score >= 11 && rightPaddle.score >= leftPaddle.score + 2) {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, { 50, 50, 50, 100 });
        DrawRectangleLinesEx({ SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 50, 200, 100 }, 4, white);
        DrawText(TextFormat("Player Two Wins! - Press Space"), SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT / 2 - 15, 30, white);
    }

}

// ---------------------------------------------------------------------------
// 5. One frame
// ---------------------------------------------------------------------------
// Input -> update -> draw. Everything that happens in 1/60th of a second.
//
// TODO:
//   - float dt = GetFrameTime();
//   - if gameOver: wait for IsKeyPressed(KEY_SPACE) to InitGame(), and skip
//     the movement updates so the final position stays frozen on screen
//   - otherwise: UpdatePaddle for each paddle, then UpdateBall
//   - then BeginDrawing / ClearBackground / DrawCourt / paddles / ball /
//     EndDrawing
//
// Useful raylib draw calls:
//   DrawRectangleRec(rect, colour)
//   DrawCircleV(position, radius, colour)
void UpdateDrawFrame()
{
    float dt = GetFrameTime();
    while (leftPaddle.score >= 11 && leftPaddle.score >= rightPaddle.score + 2 ||
        rightPaddle.score >= 11 && rightPaddle.score >= leftPaddle.score + 2) {
        if (IsKeyPressed(KEY_SPACE)) {
            InitGame();
        }
    }

    UpdatePaddle(leftPaddle, dt);
    UpdatePaddle(rightPaddle, dt);
    UpdateBall(dt);

    DrawCourt();
    DrawRectangle(leftPaddle.rect.x, leftPaddle.rect.y, leftPaddle.rect.width, leftPaddle.rect.height, white);
    DrawRectangle(rightPaddle.rect.x, rightPaddle.rect.y, rightPaddle.rect.width, rightPaddle.rect.height, white);
    DrawCircle(ball.position.x, ball.position.y, BALL_RADIUS, white);
}

// ---------------------------------------------------------------------------
// 6. Entry point
// ---------------------------------------------------------------------------

int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pong");
    SetTargetFPS(60);

    InitGame();

    while (!WindowShouldClose())
    {
        UpdateDrawFrame();
    }

    CloseWindow();
    return 0;
}
