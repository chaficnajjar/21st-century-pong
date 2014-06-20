
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

    // Intialize SDL
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_ShowCursor(0);      // don't show cursor

    // Create window and renderer
    window = SDL_CreateWindow("Pong",
            SDL_WINDOWPOS_UNDEFINED,        // centered window
            SDL_WINDOWPOS_UNDEFINED,        // centered window
            SCREEN_WIDTH,
            SCREEN_HEIGHT,
            SDL_WINDOW_SHOWN);
            
    renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );

    // Instantiate game objects
    ball = new Ball(SCREEN_WIDTH/2-ball->LENGTH/2, SCREEN_HEIGHT/2-ball->LENGTH/2);
    left_paddle = new Paddle(40, SCREEN_HEIGHT/2-Paddle::HEIGHT/2);
    right_paddle = new Paddle(SCREEN_WIDTH-(40+Paddle::WIDTH), SCREEN_HEIGHT/2-Paddle::HEIGHT/2);

    // Sounds
    Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 1024);  // initialize SDL_mixer
    paddle_sound = Mix_LoadWAV("resources/sounds/paddle_hit.wav");      // load paddle sound
    wall_sound = Mix_LoadWAV("resources/sounds/wall_hit.wav");          // load wall sound
    score_sound = Mix_LoadWAV("resources/sounds/score_update.wav");     // load score sound

    // Controllers
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

    // Fonts
    TTF_Init();     // initialize font

    font_color = {255, 255, 255, 255};

    font_name = "resources/fonts/FFFFORWA.TTF";

    font_image_launch = renderText("Press SPACE to start", font_name, font_color, 18, renderer);

    // Scores
    left_score = 0;
    right_score = 0;
    left_score_changed = true;     // indicates when rendering new score is necessary 
    right_score_changed = true;    // indicates when rendering new score is necessary 

    // Game status
    exit = false;

}

void Pong::execute() {

    while(!exit) {
        input();
        update();
        render();
        SDL_Delay(10);
    }

    clean_up();
}

void Pong::input() {
    // Handle events
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

    //=== Paddle movement ===//
    // Right paddle follows the player's mouse on the y-axis
    if (controller == mouse)
        right_paddle->set_y(mouse_y);

    // Right paddle follows the player's gamepad
    else if (controller == joystick)
        right_paddle->add_to_y(gamepad_direction);

    // AI paddle movement
    left_paddle->AI(ball);
 

    //=== Launch ball ===//
    if (ball->status == ball->READY)
        return;

    else if (ball->status == ball->LAUNCH) {
        ball->launch_ball(left_paddle);
        ball->predicted_y = left_paddle->predict(ball);
    }

    //=== Update ball speed ===//
    ball->update_speed();

    //=== Collision ===//
    // Left paddle and ball collision
    if (ball->collides_with(left_paddle)) {
        ball->bounces_off(left_paddle);
        Mix_PlayChannel(-1, paddle_sound, 0);           // play collision sound
    }
    // Right paddle and ball collision
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

    //=== Update ball coordinates ===//
    ball->x += ball->dx;
    ball->y += ball->dy;

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
    SDL_SetRenderDrawColor( renderer, 0, 0, 0, 255 );        // dark grey
    SDL_RenderClear(renderer);

    // Paddle color
    SDL_SetRenderDrawColor( renderer, 255, 255, 255, 255 );

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
        font_image_left_score = renderText(std::to_string(left_score), font_name, font_color, 24, renderer);
        left_score_changed = false;
        if (left_score == 5) {
            font_image_winner = renderText("Player 1 won!", font_name, font_color, 24, renderer);
            font_image_restart = renderText("Press SPACE to restart", font_name, font_color, 14, renderer);
        }
    }
    renderTexture(font_image_left_score, renderer, SCREEN_WIDTH * 4 / 10, SCREEN_HEIGHT / 12);

    int score_font_size = 24;
    if (right_score_changed) {
        font_image_right_score = renderText(std::to_string(right_score), font_name, font_color, score_font_size, renderer);
        right_score_changed = false;
        if (right_score == 5) {
            font_image_winner = renderText("Player 2 won!", font_name, font_color, 24, renderer);
            font_image_restart = renderText("Press SPACE to restart", font_name, font_color, 14, renderer);
        }

    }
    renderTexture(font_image_right_score, renderer, SCREEN_WIDTH * 6 / 10 - score_font_size/2, SCREEN_HEIGHT/ 12);

    // Render text indicating the winner
    if (left_score == 5) {
        renderTexture(font_image_winner, renderer, SCREEN_WIDTH * 1 / 10 + 3, SCREEN_HEIGHT / 4);   // align with score
        renderTexture(font_image_restart, renderer, SCREEN_WIDTH * 1 / 10 + 3, SCREEN_HEIGHT / 3);
        if (ball->status == ball->LAUNCHED) {
            left_score = 0;
            right_score = 0;
            left_score_changed = true;
            right_score_changed = true;
        }
    } else if (right_score == 5) {
        renderTexture(font_image_winner, renderer, SCREEN_WIDTH * 6 / 10 - score_font_size/2, SCREEN_HEIGHT / 4);   // align with score
        renderTexture(font_image_restart, renderer, SCREEN_WIDTH * 6 / 10 - score_font_size/2, SCREEN_HEIGHT / 3);
        if (ball->status == ball->LAUNCHED) {
            left_score = 0;
            right_score = 0;
            left_score_changed = true;
            right_score_changed = true;
        }
    }

    // Draw "Press SPACE to start"
    else if (ball->status == ball->READY)
        renderTexture(font_image_launch, renderer, SCREEN_WIDTH / 2 - 120, SCREEN_HEIGHT - 25);

    // Swap buffers
    SDL_RenderPresent(renderer);
}

// Release resources
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

