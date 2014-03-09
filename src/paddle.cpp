
/*
 *  Paddle class definitions
 */

#include "paddle.hpp"
#include "common.hpp"

Paddle::Paddle(int new_x, int new_y) {
    x = new_x;
    y = new_y;
}

int Paddle::get_x() {
    return x;
}

int Paddle::get_y() {
    return y;
}

void Paddle::set_y(int new_y) {
    y = new_y;

    // Paddle shouldn't be allowed to go above or below the screeen
    if (y < 0)
        y = 0;
    else if (y + HEIGHT > SCREEN_HEIGHT)
        y = SCREEN_HEIGHT - HEIGHT;
}

void Paddle::add_to_y(int new_y) {
    y += new_y;

    // Paddle shouldn't be allowed to go above or below the screeen
    if (y < 0)
        y = 0;
    else if (y + HEIGHT > SCREEN_HEIGHT)
        y = SCREEN_HEIGHT - HEIGHT; 
}


