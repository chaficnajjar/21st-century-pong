
/*
 *  Pong game
 *  Author: Chafic Najjar <chafic.najjar@gmail.com>
 *  Note: Origin of the coordinate system is the upper left corner of the screen
 */

#include <SDL2/SDL.h>           // SDL library
#include <SDL2/SDL_ttf.h>       // SDL font library
#include <cmath>                // abs()

#include <ctime>                // rand()

#include <iostream>
using namespace std;

bool done = false;                      // true when player exits game

SDL_Window*     window;                 // holds window properties
SDL_Renderer*   renderer;               // holds rendering surface properties

SDL_Texture*    font_image_score1;      // holds text indicating player 1 score (left)
SDL_Texture*    font_image_score2;      // holds text indicating right score (right)
SDL_Texture*    font_image_winner;      // holds text indicating winner
SDL_Texture*    font_image_restart;     // holds text suggesting to restart the game
SDL_Texture*    font_image_launch1;     // holds first part of text suggesting to launch the ball
SDL_Texture*    font_image_launch2;     // holds second part of text suggesting to launch the ball

SDL_Color dark_font = {67, 68, 69};     // dark_grey
SDL_Color light_font = {187, 191, 194}; // light_grey

// Screen resolution
int SCREEN_WIDTH = 640;
int SCREEN_HEIGHT = 480;

// Controllers
bool mouse = true;
bool keyboard = false;

// Mouse coordinates;
int mouse_x, mouse_y;

// Paddle lengths
const int PADDLE_WIDTH = 10;
const int PADDLE_HEIGHT = 60;

// Paddle position
int left_paddle_x = 40; 
int left_paddle_y = SCREEN_HEIGHT / 2 - 30;

int right_paddle_x = SCREEN_WIDTH - (40+PADDLE_WIDTH);
int right_paddle_y = SCREEN_HEIGHT / 2 - 30;

// Launch ball
bool launch_ball = false;

// Ball dimensions
const int BALL_WIDTH = 10;
const int BALL_HEIGHT = 10;

// Ball position
int x_ball = SCREEN_WIDTH / 2;
int y_ball = SCREEN_HEIGHT / 2;

// Ball movement
int dx = 0;             // movement in pixels over the x-axis for the next frame (speed on the x-axis)
int dy = 0;             // movement in pixels over the y-axis for the next frame (speed on the y-axis)

int speed = 8;          // ball speed = √(x²+dy²)
int hit_count = 0;      // counts the number of hits of the right paddle
                        // after three hits, speed gets incremented

float angle = 0.0f;     // angle on collision with paddle

bool bounce = false;    // true when next frame renders ball after collision impact (ball has bounced)

// Match score
int score1 = 0;
int score2 = 0;

bool left_score_changed = true;     // self-explanatory
bool right_score_changed = true;    // self-explanatory

// Prediction
int final_predicted_y;              // predicted ball position on y-axis after right paddle collision

// Font names
string fonts[] = {"Lato-Reg.TTF", "FFFFORWA.TTF"};

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

// Imprecise prediction of ball position on the y-axis after right paddle collision 
int predict() {

    // Find slope
    float slope = (float)(y_ball - y_ball+dy)/(x_ball - x_ball+dx);

    // Distance between paddles
    int paddle_distance = right_paddle_x - (left_paddle_x+PADDLE_WIDTH); 

    // Prediction without taking into consideration upper and bottom collisions
    int predicted_y = abs(slope * -(paddle_distance) + y_ball);

    // Calculate number of reflexions
    int number_of_reflexions = predicted_y / SCREEN_HEIGHT;

    // Even number of reflexions
    if (number_of_reflexions % 2 == 0)
        predicted_y = predicted_y % SCREEN_HEIGHT;
    // Odd number of reflexions
    else
        predicted_y = SCREEN_HEIGHT - (predicted_y % SCREEN_HEIGHT);

    return predicted_y;

}

// Get user input
void input() {

    SDL_Event event;    // stores next event to be processes

    // Queuing events
    while(SDL_PollEvent(&event)) {

        // Track mouse movement
        if (event.type == SDL_MOUSEMOTION)
            SDL_GetMouseState(&mouse_x, &mouse_y);

        // Clicking 'x' or pressing F4
        if (event.type == SDL_QUIT)
            done = true;

        // Pressing a key
        if (event.type == SDL_KEYDOWN)
            switch(event.key.keysym.sym) {

                // Pressing ESC exits from the game
                case SDLK_ESCAPE:
                    done = true;
                    break;

                // Pressing space will launch the ball if it isn't already launched
                case SDLK_SPACE:
                    if (!launch_ball) {
                        int direction = 1+(-2)*(rand()%2);      // either 1 or -1
                        angle = rand()%120-60;                  // between -60 and 59
                        dx = direction*speed*cos(angle*M_PI/180.0f);
                        dy = speed*sin(angle*M_PI/180.0f);

                        // Find slope
                        float slope = (float)(y_ball - y_ball+dy)/(x_ball - x_ball+dx);

                        // Distance between left paddle and center
                        int paddle_distance = SCREEN_WIDTH/2 - (left_paddle_x+PADDLE_WIDTH); 

                        // Predicting where the left paddle should go 
                        final_predicted_y = abs(slope * -(paddle_distance) + y_ball);

                        launch_ball = true;
                    }
                    break;

                // Pressing F11 to toggle fullscreen
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

// Checks if collision with left paddle occurs in next frame
bool checkLeftCollision() {
    if (!(x_ball + dx <= left_paddle_x + PADDLE_WIDTH))
        return false;
    if (x_ball < left_paddle_x)
        return false;
    if (!(y_ball + BALL_WIDTH >= left_paddle_y && y_ball <= left_paddle_y + PADDLE_HEIGHT))
        return false;
    return true;
}

// Checks if collision with right paddle occurs in next frame
bool checkRightCollision() {
    if (!(x_ball + BALL_WIDTH + dx >= right_paddle_x))
        return false; 
    if (x_ball > right_paddle_x + PADDLE_WIDTH)
        return false;
    if (!(y_ball + BALL_WIDTH > right_paddle_y && y_ball <= right_paddle_y + PADDLE_HEIGHT))
        return false;
    return true;
}

// Update game values
void update() {

    // Right paddle follows the player's mouse movement 
    if (mouse == true)
        right_paddle_y = mouse_y;

    /* Basic AI */
    // Follow the ball 
    // Ball on the left 3/5th side of the screen and going left
    if (x_ball < SCREEN_WIDTH*3/5 && dx < 0) { 
        // Follow the ball
        if (left_paddle_y + (PADDLE_HEIGHT - BALL_HEIGHT)/2 < final_predicted_y-2)
            left_paddle_y += speed/8 * 4;
        else if (left_paddle_y + (PADDLE_HEIGHT - BALL_HEIGHT)/2 > final_predicted_y+2)
            left_paddle_y -= speed/8 * 4;
    }

    // Ball is anywhere on the screen but going right
    else if (dx >= 0) {

        // Left paddle slowly moves to the center
        if (left_paddle_y + PADDLE_HEIGHT / 2 < SCREEN_HEIGHT/2)
            left_paddle_y += 2;
        else if (left_paddle_y + PADDLE_HEIGHT / 2 > SCREEN_HEIGHT/2) 
            left_paddle_y -= 2;
    }


    /* Paddle-wall collision */

    // No need to anticipate the right paddle going above the screen, mouse coordinates cannot be negative

    // Right paddle shouldn't be allowed to go below the screen
    if (right_paddle_y + PADDLE_HEIGHT > SCREEN_HEIGHT)
        right_paddle_y = SCREEN_HEIGHT - PADDLE_HEIGHT;


    // Left paddle shouldn't be allowed to go above the screen
    if (left_paddle_y < 0)
        left_paddle_y = 0;

    // Left paddle shouldn't be allowed to below the screen
    else if (left_paddle_y + PADDLE_HEIGHT > SCREEN_HEIGHT)
        left_paddle_y = SCREEN_HEIGHT - PADDLE_HEIGHT;

    // We're done updating values if the ball hasn't been launched yet
    if (!launch_ball)
        return;

    // Three hits => increment ball speed and reset hit counter
    if (hit_count == 3) {
        speed++;
        hit_count = 0;
    }

    // Smooth collision between ball and left paddle
    if (checkLeftCollision()) {
            if (bounce) {
                // y coordinate of the ball in relation to the left paddle from 0 to 60
                int left_relative_y = (y_ball - left_paddle_y + BALL_HEIGHT);
                
                // Angle formed between ball direction and left paddle after collision
                angle = (2.14f * left_relative_y - 75.0f);

                dx = speed*cos(angle*M_PI/180.0f);      // convert angle to radian, find its cos() and multiply by the speed
                dy = speed*sin(angle*M_PI/180.0f);      // convert angle to radina, find its sin() and multiply by the speed
                bounce = false;                         // finished bouncing

            }
            x_ball = left_paddle_x + PADDLE_WIDTH;      // deposit ball on left paddle surface (smooth collision)
            bounce = true;                              // bounce ball on next frame
    }

    // Smooth collision between ball and right paddle
    else if (checkRightCollision()) {
            if (bounce) {
                // y coordinate of the ball in relation to the right paddle from 0 to 60
                int right_relative_y = (y_ball - right_paddle_y + BALL_HEIGHT);

                // Angle formed between ball direction and right paddle after collision
                angle = (2.14 * right_relative_y - 75);

                dx = -speed*cos(angle*M_PI/180.0f);     // convert angle to radian, find its cos() and multiply by the negative of speed
                dy = speed*sin(angle*M_PI/180.0f);      // convert angle to radian, find its sin() and multiply by the speed
                bounce = false;                         // finished bouncing
            }
            x_ball = right_paddle_x - BALL_WIDTH;       // deposit ball on surface right paddle surface (smooth collision)
            hit_count++;                                // increment hit counter
            bounce = true;                              // bounce ball on next frame

            final_predicted_y = predict();              // predict ball position for AI to intercept

    }

    // Upper and bottom walls collision
    else if ( (y_ball + dy < 0) || (y_ball + BALL_HEIGHT + dy >= SCREEN_HEIGHT) )
        dy *= -1;

    // No collision occurs, update ball coordinates
    else {
        x_ball += dx;
        y_ball += dy;
    }

    // If ball goes out...
    if (x_ball > SCREEN_WIDTH || x_ball < 0) {

        // Change score
        if (x_ball > SCREEN_WIDTH) {
            score1++;
            left_score_changed = true;
        } else {
            score2++;
            right_score_changed = true;
        }

        // Reset ball position to before launch
        x_ball = SCREEN_WIDTH / 2;
        y_ball = SCREEN_HEIGHT / 2;
        
        // Ball is fixed
        dx = 0;
        dy = 0;
        launch_ball = false;

        // Speed and hit counter are reset to their initial positions
        speed = 8;
        hit_count = 0; 
    }

}

// Render objects on screen
void render() {

    // Clear screen (background color)
    SDL_SetRenderDrawColor( renderer, 67, 68, 69, 255 );        // dark grey
    SDL_RenderClear(renderer);


    // Color left background with light grey
    SDL_Rect left_background = { SCREEN_WIDTH / 2, 0, SCREEN_WIDTH / 2, SCREEN_HEIGHT };
    SDL_SetRenderDrawColor( renderer, 187, 191, 194, 255 );
    SDL_RenderFillRect( renderer, &left_background );

    // Render filled paddle
    SDL_Rect paddle1 = { left_paddle_x, left_paddle_y, PADDLE_WIDTH, PADDLE_HEIGHT };
    SDL_SetRenderDrawColor( renderer, 212, 120, 102, 255 );
    SDL_RenderFillRect( renderer, &paddle1 );

    // Render filled paddle
    SDL_Rect paddle2 = { right_paddle_x, right_paddle_y, PADDLE_WIDTH, PADDLE_HEIGHT };
    SDL_RenderFillRect( renderer, &paddle2 );

    // Render scores
    if (left_score_changed) {
        font_image_score1 = renderText(to_string(score1), "Lato-Reg.TTF", light_font, 24, renderer);
        left_score_changed = false;
    }
    renderTexture(font_image_score1, renderer, SCREEN_WIDTH * 4 / 10, SCREEN_HEIGHT / 12);

    int score_font_size = 24;
    if (right_score_changed) {
        font_image_score2 = renderText(to_string(score2), "Lato-Reg.TTF", dark_font, score_font_size, renderer);
        right_score_changed = false;

    }
    renderTexture(font_image_score2, renderer, SCREEN_WIDTH * 6 / 10 - score_font_size/2, SCREEN_HEIGHT/ 12);

    // Render ball
    SDL_Rect ball = { x_ball - BALL_WIDTH / 2, y_ball, BALL_WIDTH, BALL_HEIGHT };
    SDL_SetRenderDrawColor(renderer, 212, 120, 102, 255);
    SDL_RenderFillRect(renderer, &ball);

    // Render text indicating the winner
    if (score1 == 5) {
        font_image_winner = renderText("Player 1 won!", fonts[0], light_font, 24, renderer);
        renderTexture(font_image_winner, renderer, SCREEN_WIDTH * 1 / 10 + 3, SCREEN_HEIGHT / 4);   // align with score
        font_image_restart = renderText("Press SPACE to restart", fonts[0], light_font, 18, renderer);
        renderTexture(font_image_restart, renderer, SCREEN_WIDTH * 1 / 10 + 3, SCREEN_HEIGHT / 3);
        if (launch_ball) {
            score1 = 0;
            score2 = 0;
            left_score_changed = true;
            right_score_changed = true;
        }
    } else if (score2 == 5) {
        font_image_winner = renderText("Player 2 won!", fonts[0], dark_font, 24, renderer);
        renderTexture(font_image_winner, renderer, SCREEN_WIDTH * 6 / 10 - score_font_size/2, SCREEN_HEIGHT / 4);   // align with score
        font_image_restart = renderText("Press SPACE to restart", fonts[0], dark_font, 18, renderer);
        renderTexture(font_image_restart, renderer, SCREEN_WIDTH * 6 / 10 - score_font_size/2, SCREEN_HEIGHT / 3);
        if (launch_ball) {
            score1 = 0;
            score2 = 0;
            left_score_changed = true;
            right_score_changed = true;
        }
    }

    // Draw "Press SPACE to start"
    else if (!launch_ball) {
        renderTexture(font_image_launch1, renderer, SCREEN_WIDTH / 2 - 80, SCREEN_HEIGHT - 25);
        renderTexture(font_image_launch2, renderer, SCREEN_WIDTH / 2 + 1, SCREEN_HEIGHT - 25);
    }

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

    // Holds text "Press SPACE to start"
    font_image_launch1 = renderText("Press SPA", fonts[0], light_font, 18, renderer);
    font_image_launch2 = renderText("CE to start", fonts[0], dark_font, 18, renderer);

    // Don't show cursor
    SDL_ShowCursor(0);

}

int main() {

    srand(time(NULL));
    initialize();
    gameLoop();

}
