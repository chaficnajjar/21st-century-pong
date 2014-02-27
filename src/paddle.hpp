
/*
 *  Paddle class declaration
 */

#ifndef PADDLE_HPP
#define PADDLE_HPP

class Paddle {

public:
    Paddle(int x, int y);
    
    // Paddle dimensions
    static const int HEIGHT = 60;
    static const int WIDTH = 10;

    // Paddle position
    int x;
    int y;
};

#endif
