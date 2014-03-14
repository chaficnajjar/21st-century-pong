
/*
 *  Pong class definition
 */

#include "pong.hpp"
#include "utilities.hpp"
#include "ball.hpp"
#include "paddle.hpp"

std::random_device rd;
std::mt19937 gen(rd());

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
    ball = new Ball(SCREEN_WIDTH/2, SCREEN_HEIGHT/2);
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

    // Embodies events waiting to be processed
    SDL_Event event;

    // Queuing events
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
        if (event.type == SDL_JOYBUTTONDOWN)  {
            if (!ball->launched)
                launchBall();
            // As if space bar was pressed
            //event.type = SDL_KEYDOWN;
            //event.key.keysym.sym = SDLK_SPACE;
        }

        // Pressing a key
        if (event.type == SDL_KEYDOWN)
            switch(event.key.keysym.sym) {

                // Pressing ESC exits from the game
                case SDLK_ESCAPE:
                    exit = true;
                    break;

                // Pressing space will launch the ball if it isn't already launched
                case SDLK_SPACE:
                    if (!ball->launched)
                        launchBall();
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

    // Right paddle follows the player's mouse movement on the y-axis
    if (controller == mouse)
        right_paddle->set_y(mouse_y);

    else if (controller == joystick)
        right_paddle->add_to_y(gamepad_direction);

    /* Basic AI */
    // Ball on the left 3/5th side of the screen and going left
    if (ball->x < SCREEN_WIDTH*3/5 && ball->dx < 0) { 
        // Follow the ball
        if (left_paddle->get_y() + (Paddle::HEIGHT - ball->HEIGHT)/2 < ball->predicted_y-2)
            left_paddle->add_to_y(ball->speed/8 * 5);
        else if (left_paddle->get_y() + (Paddle::HEIGHT - ball->HEIGHT)/2 > ball->predicted_y+2)
            left_paddle->add_to_y( -(ball->speed/8 * 5) );
    }

    // Ball is anywhere on the screen but going right
    else if (ball->dx >= 0) {

        // Left paddle slowly moves to the center
        if (left_paddle->get_y() + Paddle::HEIGHT / 2 < SCREEN_HEIGHT/2)
            left_paddle->add_to_y(2);
        else if (left_paddle->get_y() + Paddle::HEIGHT / 2 > SCREEN_HEIGHT/2) 
            left_paddle->add_to_y(-2);
    }

    // We're exit updating values if the ball hasn't been launched yet
    if (!ball->launched)
        return;

    // Three hits => increment ball speed and reset hit counter
    if (ball->hit_count == 3) {
        ball->speed++;
        ball->hit_count = 0;
    }

    // Smooth collision between ball and left paddle
    if (checkLeftCollision()) {
            if (ball->bounce) {
                // y coordinate of the ball in relation to the left paddle (from 0 to 70)
                int left_relative_y = (ball->y - left_paddle->get_y() + ball->HEIGHT);
                
                // Angle formed between ball direction and left paddle after collision
                ball->angle = (2.14f * left_relative_y - 75.0f);

                ball->dx = ball->speed*cos(ball->angle*M_PI/180.0f);    // convert angle to radian, find its cos() and multiply by the speed
                ball->dy = ball->speed*sin(ball->angle*M_PI/180.0f);    // convert angle to radina, find its sin() and multiply by the speed
                ball->bounce = false;                                   // finished bouncing

            }
            ball->x = left_paddle->get_x() + Paddle::WIDTH;                   // deposit ball on left paddle surface (smooth collision)
            ball->bounce = true;                                        // bounce ball on next frame
            Mix_PlayChannel(-1, paddle_sound, 0);                       // play collision sound
    }

    // Smooth collision between ball and right paddle
    else if (checkRightCollision()) {
            if (ball->bounce) {
                // y coordinate of the ball in relation to the right paddle (from 0 to 70)
                int right_relative_y = (ball->y - right_paddle->get_y() + ball->HEIGHT);

                // Angle formed between ball direction and right paddle after collision
                ball->angle = (2.14 * right_relative_y - 75.0f);

                ball->dx = -ball->speed*cos(ball->angle*M_PI/180.0f);   // convert angle to radian, find its cos() and multiply by the negative of speed
                ball->dy = ball->speed*sin(ball->angle*M_PI/180.0f);    // convert angle to radian, find its sin() and multiply by the speed
                ball->bounce = false;                                   // finished bouncing
            }
            ball->x = right_paddle->get_x() - ball->WIDTH;                    // deposit ball on surface right paddle surface (smooth collision)
            ball->hit_count++;                                          // increment hit counter
            ball->bounce = true;                                        // bounce ball on next frame
            Mix_PlayChannel(-1, paddle_sound, 0);                       // play collision sound

            ball->predicted_y = predict();                              // predict ball position for AI to intercept

    }

    // Upper and bottom walls collision
    else if ( (ball->y + ball->dy < 0) || (ball->y + ball->HEIGHT + ball->dy >= SCREEN_HEIGHT) ) {
        ball->dy *= -1;                                 // reverse ball direction on y-axis
        Mix_PlayChannel(-1, wall_sound, 0);             // play collision sound
    }

    // No collision occurs, update ball coordinates
    else {
        ball->x += ball->dx;
        ball->y += ball->dy;
    }

    // If ball goes out...
    if (ball->x > SCREEN_WIDTH || ball->x < 0) {

        // Change score
        if (ball->x > SCREEN_WIDTH) {
            left_score++;
            left_score_changed = true;
        } else {
            right_score++;
            right_score_changed = true;
        }

        // Play score sound
        Mix_PlayChannel(-1, score_sound, 0); 

        // Reset ball position as before launch
        ball->x = SCREEN_WIDTH / 2;
        ball->y = SCREEN_HEIGHT / 2;
        
        // Ball is fixed
        ball->dx = 0;
        ball->dy = 0;
        ball->launched = false;

        // Speed and hit counter are reset to their initial positions
        ball->speed = 8;
        ball->hit_count = 0; 
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
    SDL_Rect pong_ball = { ball->x - ball->WIDTH / 2, ball->y, ball->WIDTH, ball->HEIGHT };
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
        if (ball->launched) {
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
        if (ball->launched) {
            left_score = 0;
            right_score = 0;
            left_score_changed = true;
            right_score_changed = true;
        }
    }

    // Draw "Press SPACE to start"
    else if (!ball->launched) {
        renderTexture(font_image_launch1, renderer, SCREEN_WIDTH / 2 - 80, SCREEN_HEIGHT - 25);
        renderTexture(font_image_launch2, renderer, SCREEN_WIDTH / 2 + 1, SCREEN_HEIGHT - 25);
    }

    // Swap buffers
    SDL_RenderPresent(renderer);

}

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

// Imprecise prediction of ball position on the y-axis after right paddle collision 
int Pong::predict() {

    // Find slope
    float slope = (float)(ball->y - ball->y+ball->dy)/(ball->x - ball->x+ball->dx);

    // Distance between paddles
    int paddle_distance = right_paddle->get_x() - (left_paddle->get_x()+Paddle::WIDTH); 

    // Prediction without taking into consideration upper and bottom wall collisions
    int predicted_y = abs(slope * -(paddle_distance) + ball->y);

    // Calculate number of reflexions
    int number_of_reflexions = predicted_y / SCREEN_HEIGHT;

    // Predictions taking into consideration upper and bottom wall collisions
    if (number_of_reflexions % 2 == 0)                      // Even number of reflexions
        predicted_y = predicted_y % SCREEN_HEIGHT;
    else                                                    // Odd number of reflexsion
        predicted_y = SCREEN_HEIGHT - (predicted_y % SCREEN_HEIGHT);

    return predicted_y;

}

void Pong::launchBall() {

        std::uniform_int_distribution<int> dir(0, 1);
        int direction = 1+(-2)*(dir(gen)%2);                            // either 1 or -1

        std::uniform_int_distribution<int> ang(-60, 60);
        ball->angle = ang(gen);                                         // between -60 and 60

        ball->dx = direction*ball->speed*cos(ball->angle*M_PI/180.0f);  // speed on the x-axis
        ball->dy = ball->speed*sin(ball->angle*M_PI/180.0f);            // speed on the y-axis

        // Find slope
        float slope = (float)(ball->y - ball->y+ball->dy)/(ball->x - ball->x+ball->dx);

        // Distance between left paddle and center
        int paddle_distance = SCREEN_WIDTH/2 - (left_paddle->get_x()+Paddle::WIDTH); 

        // Predicting where the left paddle should go in case ball is launched left
        if (direction == -1) {
            ball->predicted_y = abs(slope * -(paddle_distance) + ball->y);      // case where y is negative (upper wall collision) is taken care of

            // Bottom wall collision
            if (ball->predicted_y > SCREEN_HEIGHT)
                ball->predicted_y = SCREEN_HEIGHT - (ball->predicted_y % SCREEN_HEIGHT);
        }

        ball->launched = true;
}

// Check if collision with left paddle occurs in next frame
bool Pong::checkLeftCollision() {
    if (!(ball->x + ball->dx <= left_paddle->get_x() + Paddle::WIDTH))
        return false;
    if (ball->x < left_paddle->get_x())
        return false;
    if (!(ball->y + ball->WIDTH >= left_paddle->get_y() && ball->y <= left_paddle->get_y() + Paddle::HEIGHT))
        return false;
    return true;
}

// Check if collision with right paddle occurs in next frame
bool Pong::checkRightCollision() {
    if (!(ball->x + ball->WIDTH + ball->dx >= right_paddle->get_x()))
        return false; 
    if (ball->x > right_paddle->get_x() + Paddle::WIDTH)
        return false;
    if (!(ball->y + ball->WIDTH > right_paddle->get_y() && ball->y <= right_paddle->get_y() + Paddle::HEIGHT))
        return false;
    return true;
}
