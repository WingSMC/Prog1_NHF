#include <stdio.h>
#include <stdbool.h>

#include <SDL.h>
#include <debugmalloc.h>
#include <debugmalloc-impl.h>

#include "Error.h"
#include "UIElem.h"
#include "RGUI.h"

Uint32 _Mouse_X, _Mouse_Y;
Uint32 _Mouse_Btn;

bool in_progress = true;

Uint32 timer_func(Uint32 dt, void* param) {
	SDL_Event ev;
	ev.type = SDL_USEREVENT;
	SDL_PushEvent(&ev);
	return dt;
}

void Init_UI(UIElem*);

int main(int argc, char* args[]) {
	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Log("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		exit(INIT_FAILED);
	}

	//Create window
	RGWindow* window = RGUI_InitWindow("nhf.rgml");
	Init_UI(window->ui_root);

	SDL_TimerID tmr = SDL_AddTimer(15, timer_func, NULL);
	SDL_Event event;

	while (in_progress) {
		_Mouse_Btn = SDL_GetMouseState(&_Mouse_X, &_Mouse_Y);
		SDL_WaitEvent(&event);

		switch (event.type) {
		case SDL_QUIT:
			in_progress = false;
			break;
		case SDL_MOUSEBUTTONUP:
			Event_LMBUp();
			break;
		}

		SDL_RenderClear(window->renderer);
		RGUI_Render(window);
		SDL_RenderPresent(window->renderer);
	}

	// Free resources and close SDL
	SDL_RemoveTimer(tmr);
	RGUI_Free();
	SDL_Quit();
	
	// Ouput debug info
	remove("debugmalloc.log");
	debugmalloc_log_file("debugmalloc.log");
	debugmalloc_dump();

	return 0;
}

void InvertColor(UIElem* uie) {
	uie->color = uie->color ^ 0xffffff00;
}

#include <math.h>
void FloatElem(UIElem* uie) {
	uie->rel_position.Y = 270 + 50*sin(SDL_GetTicks() / 600.0);
	UIElem_Update(uie);
}

void Exit(UIElem* uie) {
	in_progress = false;
}

void Init_UI(UIElem *window) {

	UIElem_AddCallback(window, "button1", MouseEnter, InvertColor);
	UIElem_AddCallback(window, "button1", MouseLeave, InvertColor);
	UIElem_AddCallback(window, "button1", LMBUp, Exit);
	UIElem_AddCallback(window, "button1", Tick, FloatElem);

	UIElem_AddCallback(window, "button2", MouseEnter, InvertColor);
	UIElem_AddCallback(window, "button2", MouseLeave, InvertColor);
	UIElem_AddCallback(window, "button3", MouseEnter, InvertColor);
	UIElem_AddCallback(window, "button3", MouseLeave, InvertColor);
	UIElem_AddCallback(window, "button4", MouseEnter, InvertColor);
	UIElem_AddCallback(window, "button4", MouseLeave, InvertColor);

}

