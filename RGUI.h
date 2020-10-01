#include <stdio.h>
#include <SDL.h>

#include "Error.h"
#include "UIElem.h"

#ifndef RGUI_H
#define RGUI_H


/// \brief Compact way to store all window related variables.
typedef struct RGWindow {
	UIElem* ui_root;
	SDL_Window* window;
	SDL_Surface* surface;
	SDL_Renderer* renderer;
} RGWindow;
RGWindow* RGUI_Current_Window;

/// \brief Initializes a Window from a file
RGWindow* RGUI_InitWindow(char* file_name);
/// \brief Frees all previously allocated windows
void RGUI_Free(void);
/// \brief Sets surface global and calls UIElem_Draw on root
void RGUI_Render(RGWindow* window);


#endif