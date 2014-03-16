
/*
 *  Paddle class declaration
 */

#ifndef PADDLE_HPP
#define PADDLE_HPP

class Ball;

class Paddle {
private:
    // Paddle position
    int x;
    int y;
    
public:
    Paddle(int x, int y);

public:
    // Paddle dimensions
    static const int HEIGHT;
    static const int WIDTH;

    // Functions
    int get_x();
    int get_y();
    void set_y(int new_y);
    void add_to_y(int new_y);
    int predict(Ball *ball);
    void AI(Ball *ball);
};

#endif
