
/*
 *  Ball class declaration
 */

#ifndef BALL_HPP
#define BALL_HPP

class Paddle;

class Ball {
    public:
        Ball(int x, int y);
        
        // Ball status
        enum Status {READY, LAUNCH, LAUNCHED};
        Status status;

        // Ball dimensions
        static const int LENGTH;

        // Ball position
        int x;
        int y;

        // Ball movement
        int dx;             // movement in pixels over the x-axis for the next frame (speed on the x-axis)
        int dy;             // movement in pixels over the y-axis for the next frame (speed on the y-axis)

        bool bounce;        // true when next frame renders ball after collision impact (ball has bounced)
        int speed;          // ball speed = √(dx²+dy²)
        float angle;        // angle after collision with paddle
        int hits;           // counts the number of hits of the ball with the right paddle, increase speed after 3 hits
        int predicted_y;    // predicted ball position on y-axis after right paddle collision (used for paddle AI)

        void launch_ball(Paddle *ai_paddle);
        void update_speed();
        bool wall_collision();
        bool collides_with(Paddle *paddle);
        void bounces_off(Paddle *paddle);
        void reset();
};

#endif
