
/*
 *  Pong game
 *  Author: Chafic Najjar <chafic.najjar@gmail.com>
 *  Note: Origin of the coordinate system is the upper left corner of the screen
 */


#include "pong.hpp"

int main(int argc, char *argv[]) {

    srand(time(nullptr));

    Pong pong(argc, argv);
    pong.execute();

    return 0;

}
