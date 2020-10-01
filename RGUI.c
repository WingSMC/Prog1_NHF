#include <math.h>
#include <debugmalloc.h>
#include <debugmalloc-impl.h>

#include "RGUI.h"

typedef struct RGWindowNode {
	RGWindow* rg_window;
	struct RGWindowNode* next;
} RGWindowNode;

static RGWindowNode* RGWindowList = NULL;

typedef enum TagState {
	Tab,
	InTag,
	InProperty,
	TagEnd
} TagState;

static UIElem* Make_Node(FILE* rgml, UIElem *prev, int depth) {
	// type, name, dim, color, tex, data
	char props[6][255 + 1] = { '\0' };

	int c, prop_index = 0, char_index = 0, new_depth = 0;
	TagState state = Tab;
	bool continue_loop = true;
	while (continue_loop && (c =  getc(rgml)) != '\n') {
		if (c == EOF) exit(INVALID_RGML);
		switch (state) {
			case Tab:
				if (c == '\t') {
					++new_depth;
				} else if (c == '>') {
					continue_loop = false;
				} else if (c == '<') {
					state = InTag;
				} else {
					exit(INVALID_RGML);
				}
				if (new_depth - depth > 1) exit(INVALID_RGML);
				break;
			case InTag:
				if (c == '"') {
					state = InProperty;
				}
				break;
			case InProperty:
				if (c == '"') {
					char_index = 0;
					++prop_index;
					state = InTag;
				} else {
					if (char_index > 255) exit(INVALID_RGML);
					props[prop_index][char_index++] = c;
				}
				break;
		}
	}
	if ((
	props[0][0] == '\0' ||
	props[1][0] == '\0' ||
	props[2][0] == '\0' ||
	props[3][0] == '\0') &&
	continue_loop) {
		exit(INVALID_RGML);
	}
	if (!continue_loop) {
		while ((prev = prev->parent)->parent != NULL);
		return prev;
	}
	// the depth will determine the position in the hierarchy
	int depth_dir = new_depth - depth;
	Vec2 pos;
	Vec2 size;
	pos.X = SDL_atoi(strtok(props[2], " "));
	pos.Y = SDL_atoi(strtok(NULL, " "));
	size.X = SDL_atoi(strtok(NULL, " "));
	size.Y = SDL_atoi(strtok(NULL, " "));
	Uint8 r, g, b, a;
	r = SDL_atoi(strtok(props[3], " "));
	g = SDL_atoi(strtok(NULL, " "));
	b = SDL_atoi(strtok(NULL, " "));
	a = SDL_atoi(strtok(NULL, " "));
	Uint32 color = (r << 24 | g << 16 | b << 8 | a);

	UIElem* new_elem = UIElem_Init(pos, size, props[4], color, props[1]);
	// HIERARCHY LEGO
	if (depth_dir == 1) {
		// if 1 it is a child
		if (prev != NULL) UIElem_AddChild(prev, new_elem);
		else new_elem->parent = NULL;
	} else {
		// if 0 the prev has a new sibling
		// if -n it is the sibling of the n-th parent of the prev
		depth_dir = -depth_dir;
		for (int i = 0; i < depth_dir; ++i) {
			prev = prev->parent;
		}
		UIElem_AddChild(prev->parent, new_elem);
	}
	return Make_Node(rgml, new_elem, new_depth);
}

RGWindow* RGUI_InitWindow(char* file_name) {
	RGWindow* rg_window = malloc(sizeof(RGWindow));
	RGWindowNode* window_node = malloc(sizeof(RGWindowNode));
	if (rg_window == NULL || window_node == NULL) exit(MALLOC_FAILED);

	/*------------------------Init from file-------------------------*/
	FILE* rgml = fopen(file_name, "r");
	if (rgml == NULL) exit(FILE_READ_ERROR);
	UIElem* root_elem = Make_Node(rgml, NULL, -1);
	fclose(rgml);
	UIElem_AddCallback(root_elem, root_elem->name, Tick, UIElem_MouseInside);
	rg_window->ui_root = root_elem;
	/*---------------------------------------------------------------*/

	
	// Create window
	rg_window->window = SDL_CreateWindow(
		rg_window->ui_root->name,
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		rg_window->ui_root->size.X,
		rg_window->ui_root->size.Y,
		SDL_WINDOW_SHOWN
	);
	if (rg_window->window == NULL) {
		printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		exit(INIT_FAILED);
	}

	// Initialize SDL_Renderer
	rg_window->renderer = SDL_CreateRenderer(rg_window->window, -1, SDL_RENDERER_SOFTWARE);
	if (rg_window->renderer == NULL) {
		SDL_Log("Renderer could not initialize! SDL_Error: %s\n", SDL_GetError());
		exit(INIT_FAILED);
	}

	// Get window surface
	rg_window->surface = SDL_GetWindowSurface(rg_window->window);
	if (rg_window->surface == NULL) {
		SDL_Log("Surface could not be created! SDL_Error: %s\n", SDL_GetError());
		exit(INIT_FAILED);
	}

	window_node->rg_window = rg_window;
	window_node->next = RGWindowList;
	RGWindowList = window_node;

	RGUI_Current_Window = rg_window;
	UIElem_LoadTextures(rg_window->ui_root);

	return rg_window;
}

void RGUI_Free(void) {
	RGWindowNode* temp;
	while ((temp = RGWindowList) != NULL) {
		RGWindowList = RGWindowList->next;
		UIElem_Delete(temp->rg_window->ui_root);
		SDL_FreeSurface(temp->rg_window->surface);
		SDL_DestroyRenderer(temp->rg_window->renderer);
		SDL_DestroyWindow(temp->rg_window->window);
		free(temp->rg_window);
		free(temp);
	}
}

void RGUI_Render(RGWindow* window) {
	RGUI_Current_Window = window;
	UIElem_Draw(window->ui_root);
}
