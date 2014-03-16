
/*
 *  Pong class definition
 */

#include "pong.hpp"
#include "ball.hpp"
#include "paddle.hpp"
#include "utilities.hpp"

/* Screen resolution */
const int Pong::SCREEN_WIDTH = 640;
const int Pong::SCREEN_HEIGHT = 480;

Pong::Pong(int argc, char *argv[]) {

    /* Initilize SDL */
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_ShowCursor(0);      // don't show cursor

    /* Window and renderer */
    window = SDL_CreateWindow("Pong",
            SDL_WINDOWPOS_UNDEFINED,        // centered window
            SDL_WINDOWPOS_UNDEFINED,        // centered window
            SCREEN_WIDTH,
            SCREEN_HEIGHT,
            SDL_WINDOW_SHOWN);
            
    renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );

    /* Game objects */
    ball = new Ball(SCREEN_WIDTH/2-ball->LENGTH/2, SCREEN_HEIGHT/2);
    left_paddle = new Paddle(40, SCREEN_HEIGHT/2-30);
    right_paddle = new Paddle(SCREEN_WIDTH-(40+Paddle::WIDTH), SCREEN_HEIGHT/2-30);

    /* Sounds */
    Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 1024);  // initialize SDL_mixer

    paddle_sound = Mix_LoadWAV("resources/sounds/paddle_hit.wav");      // load paddle sound
    wall_sound = Mix_LoadWAV("resources/sounds/wall_hit.wav");          // load wall sound
    score_sound = Mix_LoadWAV("resources/sounds/score_update.wav");     // load score sound

    /* Controllers */
    if (argc == 2) { 
        if ( strcmp(argv[1], "keyboard") == 0 )
            controller = keyboard;
        else if ( strcmp(argv[1], "joystick") == 0 )
            controller = joystick;
        else
            controller = mouse;
    } else
        controller = mouse;     // default controller

    if (controller == joystick) {
        printf("%i joysticks were found.\n\n", SDL_NumJoysticks() );
        printf("The names of the joysticks are:\n");

        gamepad = SDL_JoystickOpen(0);          // give control to the first joystick
        for(int i = 0; i < SDL_NumJoysticks(); i++) 
            std::cout << "\t" << SDL_JoystickName(gamepad) << std::endl;
                        
        SDL_JoystickEventState(SDL_ENABLE);     // initialize joystick controller

        gamepad_direction = 0;
    }

    /* Fonts */
    TTF_Init();     // initialize font

    dark_font = {67, 68, 69};       // dark grey
    light_font = {187, 191, 194};   // light grey

    fonts[0] = "resources/fonts/Lato-Reg.TTF";
    fonts[1] = "resources/fonts/FFFFORWA.TTF";

    font_image_launch1 = renderText("Press SPA", fonts[0], light_font, 18, renderer);
    font_image_launch2 = renderText("CE to start", fonts[0], dark_font, 18, renderer);

    /* Scores */
    left_score = 0;
    right_score = 0;
    left_score_changed = true;     // indicates when rendering new score is necessary 
    right_score_changed = true;    // indicates when rendering new score is necessary 

    /* Game states */
    exit = false;

}

void Pong::execute() {

    while(!exit) {
        input();
        update();
        render();
    }

    clean_up();
}

void Pong::input() {

    //=== Handling events ===//

    SDL_Event event;
    while(SDL_PollEvent(&event)) {

        // Track mouse movement
        if (event.type == SDL_MOUSEMOTION)
            SDL_GetMouseState(&mouse_x, &mouse_y);

        // Clicking 'x' or pressing F4
        if (event.type == SDL_QUIT)
            exit = true;

        // Joystick direction controller moved
        if (event.type == SDL_JOYAXISMOTION) {
            // 32767
            // Up or down
            if (event.jaxis.axis == 1)
                gamepad_direction = event.jaxis.value/5461;
        }

        // Joystick action button pressed
        if (event.type == SDL_JOYBUTTONDOWN) 
            if (ball->status == ball->READY)
                ball->status = ball->LAUNCH;

        // Pressing a key
        if (event.type == SDL_KEYDOWN)
            switch(event.key.keysym.sym) {

                // Pressing ESC exits from the game
                case SDLK_ESCAPE:
                    exit = true;
                    break;

                // Pressing space will launch the ball if it isn't already launched
                case SDLK_SPACE:
                    if (ball->status == ball->READY)
                        ball->status = ball->LAUNCH;
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

// Update game values
void Pong::update() {

    //=======================//
    //=== Paddle movement ===//

    // Right paddle follows the player's mouse on the y-axis
    if (controller == mouse)
        right_paddle->set_y(mouse_y);

    // Right paddle follows the player's gamepad
    else if (controller == joystick)
        right_paddle->add_to_y(gamepad_direction);

    // AI paddle movement
    left_paddle->AI(ball);
 

    //===================//
    //=== Launch ball ===//

    if (ball->status == ball->READY)
        return;

    else if (ball->status == ball->LAUNCH) {
        ball->launch_ball(left_paddle);
        ball->predicted_y = left_paddle->predict(ball);
    }


    //=========================//
    //=== Update ball speed ===//

    ball->update_speed();


    //=================//
    //=== Collision ===//

    if (ball->collides_with(left_paddle)) {
        ball->bounces_off(left_paddle);
        Mix_PlayChannel(-1, paddle_sound, 0);           // play collision sound
    }

    else if (ball->collides_with(right_paddle)) {
        ball->bounces_off(right_paddle);
        ball->predicted_y = left_paddle->predict(ball); // predict ball position on the y-axis
        Mix_PlayChannel(-1, paddle_sound, 0);
    }

    // Upper and bottom walls collision
    if (ball->wall_collision()) {
        ball->dy *= -1;                                 // reverse ball direction on y-axis
        Mix_PlayChannel(-1, wall_sound, 0);             // play collision sound
    }


    //===============================//
    //=== Update ball coordinates ===//

    ball->x += ball->dx;
    ball->y += ball->dy;


    //=====================//
    //=== Ball goes out ===//

    if (ball->x > SCREEN_WIDTH || ball->x < 0) {

        // Change score
        if (ball->x > SCREEN_WIDTH) {
            left_score++;
            left_score_changed = true;
        } else {
            right_score++;
            right_score_changed = true;
        }

        Mix_PlayChannel(-1, score_sound, 0); 

        ball->reset();
    }

}

// Render objects on screen
void Pong::render() {

    // Clear screen (background color)
    SDL_SetRenderDrawColor( renderer, 67, 68, 69, 255 );        // dark grey
    SDL_RenderClear(renderer);

    // Color left background with light grey
    SDL_Rect left_background = { SCREEN_WIDTH / 2, 0, SCREEN_WIDTH / 2, SCREEN_HEIGHT };
    SDL_SetRenderDrawColor( renderer, 187, 191, 194, 255 );
    SDL_RenderFillRect( renderer, &left_background );

    // Paddle color
    SDL_SetRenderDrawColor( renderer, 212, 120, 102, 255 );

    // Render filled paddle
    SDL_Rect paddle1 = { left_paddle->get_x(), left_paddle->get_y(), Paddle::WIDTH, Paddle::HEIGHT };
    SDL_RenderFillRect( renderer, &paddle1 );

    // Render filled paddle
    SDL_Rect paddle2 = { right_paddle->get_x(), right_paddle->get_y(), Paddle::WIDTH, Paddle::HEIGHT };
    SDL_RenderFillRect( renderer, &paddle2 );

    // Render ball
    SDL_Rect pong_ball = { ball->x, ball->y, ball->LENGTH, ball->LENGTH };
    SDL_RenderFillRect(renderer, &pong_ball);

    // Render scores
    if (left_score_changed) {
        font_image_left_score = renderText(std::to_string(left_score), "resources/fonts/Lato-Reg.TTF", light_font, 24, renderer);
        left_score_changed = false;
    }
    renderTexture(font_image_left_score, renderer, SCREEN_WIDTH * 4 / 10, SCREEN_HEIGHT / 12);

    int score_font_size = 24;
    if (right_score_changed) {
        font_image_right_score = renderText(std::to_string(right_score), "resources/fonts/Lato-Reg.TTF", dark_font, score_font_size, renderer);
        right_score_changed = false;

    }
    renderTexture(font_image_right_score, renderer, SCREEN_WIDTH * 6 / 10 - score_font_size/2, SCREEN_HEIGHT/ 12);

    // Render text indicating the winner
    if (left_score == 5) {
        font_image_winner = renderText("Player 1 won!", fonts[0], light_font, 24, renderer);
        renderTexture(font_image_winner, renderer, SCREEN_WIDTH * 1 / 10 + 3, SCREEN_HEIGHT / 4);   // align with score
        font_image_restart = renderText("Press SPACE to restart", fonts[0], light_font, 18, renderer);
        renderTexture(font_image_restart, renderer, SCREEN_WIDTH * 1 / 10 + 3, SCREEN_HEIGHT / 3);
        if (ball->status == ball->LAUNCHED) {
            left_score = 0;
            right_score = 0;
            left_score_changed = true;
            right_score_changed = true;
        }
    } else if (right_score == 5) {
        font_image_winner = renderText("Player 2 won!", fonts[0], dark_font, 24, renderer);
        renderTexture(font_image_winner, renderer, SCREEN_WIDTH * 6 / 10 - score_font_size/2, SCREEN_HEIGHT / 4);   // align with score
        font_image_restart = renderText("Press SPACE to restart", fonts[0], dark_font, 18, renderer);
        renderTexture(font_image_restart, renderer, SCREEN_WIDTH * 6 / 10 - score_font_size/2, SCREEN_HEIGHT / 3);
        if (ball->status == ball->LAUNCHED) {
            left_score = 0;
            right_score = 0;
            left_score_changed = true;
            right_score_changed = true;
        }
    }

    // Draw "Press SPACE to start"
    else if (!ball->status == ball->LAUNCHED) {
        renderTexture(font_image_launch1, renderer, SCREEN_WIDTH / 2 - 80, SCREEN_HEIGHT - 25);
        renderTexture(font_image_launch2, renderer, SCREEN_WIDTH / 2 + 1, SCREEN_HEIGHT - 25);
    }

    // Swap buffers
    SDL_RenderPresent(renderer);

}

//=== Release resources ===//
void Pong::clean_up() {

    // Destroy textures
    SDL_DestroyTexture(font_image_left_score);
    SDL_DestroyTexture(font_image_right_score);

    // Free the sound effects
    Mix_FreeChunk(paddle_sound);
    Mix_FreeChunk(wall_sound);
    Mix_FreeChunk(score_sound);

    // Quit SDL_mixer
    Mix_CloseAudio();

    // Close joystick
    if (controller == joystick)
        SDL_JoystickClose(gamepad);

    // Destroy renderer and window
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    // Shuts down SDL
    SDL_Quit();
}

