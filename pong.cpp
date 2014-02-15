
/*
 *  Pong game
 *  Author: Chafic Najjar <chafic.najjar@gmail.com>
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <ctime>

#include <iostream>
using namespace std;

bool done = false;

SDL_Window*     window;
SDL_Renderer*   renderer;
SDL_Texture*    font_image_score1;
SDL_Texture*    font_image_score2;
SDL_Color font_color = {255, 255, 255};

// Screen resolution
int SCREEN_WIDTH = 640;
int SCREEN_HEIGHT = 480;

bool mouse = true;
bool keyboard = false;

// Mouse coordinates;
int x, y;

// Paddle lengths
const int PADDLE_WIDTH = 10;
const int PADDLE_HEIGHT = 60;

// Paddle position
int left_paddle_x = 40; 
int left_paddle_y = SCREEN_HEIGHT / 2 - 30;

int right_paddle_x = SCREEN_WIDTH - (40+PADDLE_WIDTH);
int right_paddle_y = SCREEN_HEIGHT / 2 - 30;

// Paddle movement
int paddle1_offset = 0;
int paddle2_offset = 0;

// Ball movement indicator
bool launch_ball = false;

// Ball lengths
const int BALL_WIDTH = 10;
const int BALL_HEIGHT = 10;

// Ball position
int x_ball = SCREEN_WIDTH / 2;
int y_ball = SCREEN_HEIGHT / 2;

// Ball movement
float dx = 0;
float dy = 0;

float speed = 8;
int hit_count = 0;
float angle = 0;

bool bounce = false;

// Score 
int score1 = 0;
int score2 = 0;

bool render_score1 = true;
bool render_score2 = true;

void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, SDL_Rect dst, SDL_Rect *clip = nullptr) {
    SDL_RenderCopy(ren, tex, clip, &dst);
}

void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y, SDL_Rect *clip = nullptr) {
    SDL_Rect dst;
    dst.x = x;
    dst.y = y;
    if (clip != nullptr){
        dst.w = clip->w;
        dst.h = clip->h;
    }

    else
        SDL_QueryTexture(tex, NULL, NULL, &dst.w, &dst.h);

    renderTexture(tex, ren, dst, clip);
}

SDL_Texture* renderText(const string &message, const string &fontFile, SDL_Color color, int fontSize, SDL_Renderer *renderer) {
    TTF_Font *font = TTF_OpenFont(fontFile.c_str(), fontSize);

    SDL_Surface *surf = TTF_RenderText_Blended(font, message.c_str(), color);

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surf);

    SDL_FreeSurface(surf);
    TTF_CloseFont(font);
    return texture;
}

void input() {

    // Queuing events
    SDL_Event event;
    while(SDL_PollEvent(&event)) {

        // Mouse moves
        if (event.type == SDL_MOUSEMOTION) {
            SDL_GetMouseState(&x, &y);
        }

        // Pressing 'x' or F4
        if (event.type == SDL_QUIT)
            done = true;

        // Pressing a key
        if (event.type == SDL_KEYDOWN)
            switch(event.key.keysym.sym) {

                case SDLK_ESCAPE:
                    done = true;
                    break;

                case SDLK_UP:
                    paddle1_offset = -5;
                    break;

                case SDLK_DOWN:
                    paddle1_offset = 5;
                    break;

                case SDLK_SPACE:
                    if (!launch_ball) {
                        angle = (float)(rand() % 3600)/10.0f;
                        dx = speed*cos(angle*M_PI/180.0f);
                        dy = speed*sin(angle*M_PI/180.0f);
                        launch_ball = true;
                    }
                    break;

                case SDLK_w:
                    paddle2_offset = -5;
                    break;

                case SDLK_s:
                    paddle2_offset = 5;
                    break;

                case SDLK_F11:
                    int flags = SDL_GetWindowFlags(window);
                    if(flags & SDL_WINDOW_FULLSCREEN)
                        SDL_SetWindowFullscreen(window, 0);
                    else
                        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
                    break;
            }

    }
}

bool checkRightCollision() {
    if (!(x_ball + BALL_WIDTH + dx >= right_paddle_x))
        return false; 
    if (x_ball > right_paddle_x + PADDLE_WIDTH)
        return false;
    if (!(y_ball + BALL_WIDTH > right_paddle_y && y_ball <= right_paddle_y + PADDLE_HEIGHT))
        return false;
    return true;
}

bool checkLeftCollision() {
    if (!(x_ball + dx <= left_paddle_x + PADDLE_WIDTH))
        return false;
    if (x_ball < left_paddle_x)
        return false;
    if (!(y_ball + BALL_WIDTH >= left_paddle_y && y_ball <= left_paddle_y + PADDLE_HEIGHT))
        return false;
    return true;
}

void update() {

    // Playing with mouse
    if (mouse == true) {
        right_paddle_y = y;
    }

    // return if ball hasn't been launched
    if (!launch_ball)
        return;


    if (hit_count == 3) {
        speed++;
        hit_count = 0;
    }

    // AI: left paddle follows the ball
    left_paddle_y = y_ball - PADDLE_HEIGHT / 2;

    if (right_paddle_y + 60 > SCREEN_HEIGHT)
        right_paddle_y = SCREEN_HEIGHT - 60;
    // No need to anticipate the paddle going above the screen, mouse coordinates cannot be negative

    if (left_paddle_y < 0)
        left_paddle_y = 0;

    else if (left_paddle_y + 60 > SCREEN_HEIGHT)
        left_paddle_y = SCREEN_HEIGHT - 60;

    // Smooth left paddle-ball collision
    if (checkLeftCollision()) {
            if (bounce) {
                int left_relative_y = (y_ball - left_paddle_y + BALL_HEIGHT);
                angle = (2.14 * left_relative_y - 75);
                dx = speed*cos(angle*M_PI/180.0f);      // convert to radian first 
                dy = speed*sin(angle*M_PI/180.0f);
                bounce = false;

            }
            x_ball = left_paddle_x + PADDLE_WIDTH;      // move ball to paddle surface
            hit_count++;
            bounce = true;
    }

    // Smooth right paddle-ball collision
    else if (checkRightCollision()) {
            if (bounce) {
                int right_relative_y = (y_ball - right_paddle_y + BALL_HEIGHT);
                angle = (2.14 * right_relative_y - 75);
                dx = -speed*cos(angle*M_PI/180.0f); 
                dy = speed*sin(angle*M_PI/180.0f);
                bounce = false;
            }
            x_ball = right_paddle_x - BALL_WIDTH;       // ball hits paddle on surface
            bounce = true;
    }

    // Upper and bottom walls collision
    else if ( (y_ball + dy < 0) || (y_ball + BALL_HEIGHT + dy >= SCREEN_HEIGHT) ) 
        dy *= -1;

    // No collision occurs
    else {
        x_ball += dx;
        y_ball += dy;
    }

    // If ball goes out...
    if (x_ball < 0) {
        score2++;
        render_score2 = true;
        x_ball = SCREEN_WIDTH / 2;
        y_ball = SCREEN_HEIGHT / 2;
        dx = 0;
        dy = 0;
        launch_ball = false;
        speed = 8;
        hit_count = 0;
    }

    else if (x_ball > SCREEN_WIDTH) {
        score1++;
        render_score1 = true;
        x_ball = SCREEN_WIDTH / 2;
        y_ball = SCREEN_HEIGHT / 2;
        dx = 0;
        dy = 0;
        launch_ball = false;
        speed = 8;
        hit_count = 0;
    }

}

void render() {

    // Clear screen
    SDL_SetRenderDrawColor( renderer, 0, 0, 0, 255 );
    SDL_RenderClear(renderer);

    // Render white filled paddle
    SDL_Rect paddle1 = { left_paddle_x, left_paddle_y, PADDLE_WIDTH, PADDLE_HEIGHT };
    SDL_SetRenderDrawColor( renderer, 255, 255, 255, 255 );
    SDL_RenderFillRect( renderer, &paddle1 );

    // Render white filled paddle
    SDL_Rect paddle2 = { right_paddle_x, right_paddle_y, PADDLE_WIDTH, PADDLE_HEIGHT };
    SDL_SetRenderDrawColor( renderer, 255, 255, 255, 255 );
    SDL_RenderFillRect( renderer, &paddle2 );

    // Draw white vertical line
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    int j = 30;
    int k = 35;
    while (k < SCREEN_HEIGHT - 30) {
        SDL_RenderDrawLine(renderer, SCREEN_WIDTH / 2 , j, SCREEN_WIDTH / 2, k);
        SDL_RenderDrawLine(renderer, SCREEN_WIDTH / 2 + 1, j, SCREEN_WIDTH / 2 + 1, k);
        SDL_RenderDrawLine(renderer, SCREEN_WIDTH / 2 + 2, j, SCREEN_WIDTH / 2 + 2, k);
        SDL_RenderDrawLine(renderer, SCREEN_WIDTH / 2 + 3, j, SCREEN_WIDTH / 2 + 3, k);
        j += 17;
        k += 17;
    }

    // Render scores
    if (render_score1) {
        font_image_score1 = renderText(to_string(score1), "FFFFORWA.TTF", font_color, 24, renderer);
        render_score1 = false;
    }
    renderTexture(font_image_score1, renderer, SCREEN_WIDTH * 4 / 10, SCREEN_HEIGHT / 8);

    int score_font_size = 24;
    if (render_score2) {
        font_image_score2 = renderText(to_string(score2), "FFFFORWA.TTF", font_color, score_font_size, renderer);
        render_score2 = false;

    }
    renderTexture(font_image_score2, renderer, SCREEN_WIDTH * 6 / 10 - score_font_size/2, SCREEN_HEIGHT / 8);

    // Render ball
    SDL_Rect ball = { x_ball, y_ball, BALL_WIDTH, BALL_HEIGHT };
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &ball);

    // Swap buffers
    SDL_RenderPresent(renderer);

}

void cleanUp() {
    SDL_DestroyTexture(font_image_score1);
    SDL_DestroyTexture(font_image_score2);
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
}

void gameLoop() {

    while(!done) {
        input();
        update();
        render();
    }

    cleanUp();
}

void initialize() {

    SDL_Init(SDL_INIT_EVERYTHING);

    window = SDL_CreateWindow( "Pong",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            SCREEN_WIDTH,
            SCREEN_HEIGHT,
            SDL_WINDOW_SHOWN);
            
    renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

    // Initialize font
    TTF_Init();

    // Don't show cursor
    SDL_ShowCursor(0);

}

int main() {

    srand(time(NULL));
    initialize();
    gameLoop();

}
