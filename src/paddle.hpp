
/*
 *  Paddle class declaration
 */

#ifndef PADDLE_HPP
#define PADDLE_HPP

class Paddle {
public:
    Paddle(int x, int y);
private:
    // Paddle position
    int x;
    int y;
    
public:
    // Paddle dimensions
    static const int HEIGHT = 60;
    static const int WIDTH = 10;

    // Functions
    int get_x();
    int get_y();
    void set_y(int new_y);
    void add_to_y(int new_y);
};

#endif
