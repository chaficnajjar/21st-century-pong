
/*
 *  Pong class declaration
 */

#ifndef PONG_HPP
#define PONG_HPP

#include <SDL2/SDL.h>                       // SDL library
#include <SDL2/SDL_ttf.h>                   // SDL font library
#include <SDL2/SDL_mixer.h>                 // SDL sound library

#include <iostream>

class Ball;
class Paddle;

class Pong {
public:
    // Screen resolution
    static const int SCREEN_WIDTH;
    static const int SCREEN_HEIGHT;

    Pong(int argc, char *argv[]);
    void execute();
    void input();
    void update();
    void render();
    void clean_up();

    // Window and renderer
    SDL_Window* window;                     // holds window properties
    SDL_Renderer* renderer;                 // holds rendering surface properties
    
    // Game objects
    Ball *ball;
    Paddle *left_paddle;
    Paddle *right_paddle;

    // Sounds
    Mix_Chunk *paddle_sound;                // holds sound produced after ball collides with paddle
    Mix_Chunk *wall_sound;                  // holds sound produced after ball collides with wall
    Mix_Chunk *score_sound;                 // holds sound produced when updating score

    // Controllers
    enum Controllers { mouse, keyboard, joystick };
    Controllers controller;
    SDL_Joystick *gamepad;                  // holds joystick information
    int gamepad_direction;                  // gamepad direction
    int mouse_x, mouse_y;                   // mouse coordinates

    // Fonts
    std::string font_name;                  // font name
    SDL_Color font_color;                   // font color
    SDL_Texture*    font_image_left_score;  // holds text indicating player 1 score (left)
    SDL_Texture*    font_image_right_score; // holds text indicating palyer 2 score (right)
    SDL_Texture*    font_image_winner;      // holds text indicating winner
    SDL_Texture*    font_image_restart;     // holds text suggesting to restart the game
    SDL_Texture*    font_image_launch;     // holds first part of text suggesting to launch the ball
    
    // Scores
    int left_score;     
    int right_score;
    bool left_score_changed;                // indicates when rendering new score is necessary 
    bool right_score_changed;               // indicates when rendering new score is necessary 

    // Game status
    bool exit;                              // true when player wants to exit game
};

#endif
