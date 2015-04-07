
/* 
 *  Useful functions
 */

#ifndef UTILITIES_H
#define UTILITIES_H

#include <string>
#include <SDL2/SDL.h>

void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, SDL_Rect dst, SDL_Rect *clip = nullptr);
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y, SDL_Rect *clip = nullptr);
SDL_Texture* renderText(const std::string &message, const std::string &fontFile, SDL_Color color, int fontSize, SDL_Renderer *renderer);

#endif  // UTILITIES_H
