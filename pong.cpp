
/*
 *  Pong game
 *  Author: Chafic Najjar <chafic.najjar@gmail.com>
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <iostream>
using namespace std;

bool done = false;

SDL_Window*     window;
SDL_Renderer*   renderer;
SDL_Texture*    font_image_score1;
SDL_Texture*    font_image_score2;
SDL_Color font_color = {255, 255, 255};

int SCREEN_WIDTH = 640;
int SCREEN_HEIGHT = 480;

int paddle1_offset = 0;
int paddle2_offset = 0;

int paddle1_position = SCREEN_HEIGHT / 2 - 30;
int paddle2_position = SCREEN_HEIGHT / 2 - 30;

int x, y;
bool mouse = true;
bool keyboard = false;

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

void update() {

    // Playing with mouse
    if (mouse) {
        paddle2_position = y;
        if (y + 60 > SCREEN_HEIGHT) {
            score2++;
            render_score2 = true;
            paddle2_position = SCREEN_HEIGHT - 60;
        }

    }

    // Playing with keyboard
    else if (keyboard) {
        paddle1_position += paddle1_offset;
        if (paddle1_position < 0)
            paddle1_position -= paddle1_offset;
        else if (paddle1_position > SCREEN_HEIGHT - 60)
            paddle1_position -= paddle1_offset;
        paddle1_offset = 0;

        paddle2_position += paddle2_offset;
        if (paddle2_position < 0)
            paddle2_position -= paddle2_offset;
        else if (paddle2_position > SCREEN_HEIGHT - 60)
            paddle2_position -= paddle2_offset;
        paddle2_offset = 0;
    }
}

void render() {

    // Clear screen
    SDL_SetRenderDrawColor( renderer, 0, 0, 0, 255 );
    SDL_RenderClear(renderer);

    // Render white filled quad
    SDL_Rect paddle1 = { 40, paddle1_position, 20, 60 };
    SDL_SetRenderDrawColor( renderer, 255, 255, 255, 255 );
    SDL_RenderFillRect( renderer, &paddle1 );

    // Render white filled quad
    SDL_Rect paddle2 = { SCREEN_WIDTH-60, paddle2_position, 20, 60 };
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

    renderTexture(font_image_score1, renderer, SCREEN_WIDTH / 4, SCREEN_HEIGHT / 8);

    if (render_score2) {
        font_image_score2 = renderText(to_string(score2), "FFFFORWA.TTF", font_color, 24, renderer);
        render_score2 = false;

    }

    renderTexture(font_image_score2, renderer, SCREEN_WIDTH * 3 / 4, SCREEN_HEIGHT / 8);

    // Update screen
    SDL_RenderPresent(renderer);

}


void gameLoop() {

    while(!done) {
        input();
        update();
        render();
    }

    SDL_DestroyTexture(font_image_score1);
    SDL_DestroyTexture(font_image_score2);
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
}

void initialize() {

    SDL_Init(SDL_INIT_EVERYTHING);

    window = SDL_CreateWindow( "Pong",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            SCREEN_WIDTH,
            SCREEN_HEIGHT,
            SDL_WINDOW_SHOWN);
            
    renderer = SDL_CreateRenderer( window,
            -1, 
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

    // Initialize font
    TTF_Init();

    // Don't show cursor
    SDL_ShowCursor(0);

}

int main() {

    initialize();
    gameLoop();

}
