
/*
 *  Ball class definitions
 */

#include "ball.hpp"

Ball::Ball(int x, int y) {
    launched = false;

    this->x = x;
    this->y = y;

    dx = 0;
    dy = 0;

    bounce = false;
    speed = 8;
    angle = 0.0f;
    hit_count = 0;      
}

