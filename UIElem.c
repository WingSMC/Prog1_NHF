#include <debugmalloc.h>
#include <debugmalloc-impl.h>
#include <SDL_image.h>

#include "RGUI.h"
#include "UIElem.h"


extern Uint32 _Mouse_X;
extern Uint32 _Mouse_Y;
extern Uint32 _Mouse_Btn;

extern RGWindow* RGUI_Current_Window;

/// \brief The reference of the two previous elements the user interacted with.
///
/// Technically a QUEUE
/// [0] = IN
/// [1] = OUT
UIElem *_State[2] = { NULL };

/* Structure */

UIElem* UIElem_Init(Vec2 position, Vec2 size, char *tex_path, Uint32 color, char* name) {
	UIElem* uie = (UIElem*)malloc(sizeof(UIElem));
	if(uie == NULL) exit(MALLOC_FAILED);
	if (tex_path != NULL) strcpy(uie->tex_path, tex_path);
	else tex_path = "";

	strcpy(uie->name, name);

	uie->rel_position = position;
	uie->abs_position = position;
	uie->size = size;
	uie->color = color;
	uie->tex = NULL;

	// #region Dynamically allocated things:
	uie->parent = NULL;
	uie->sibling = NULL;
	uie->child = NULL;
	uie->data = NULL;

	for (size_t i = 0; i < N_CALLBACKS; i++) uie->callbacks[i] = NULL;
	// #endregion

	return uie;
}

void UIElem_AddChild(UIElem* parent, UIElem* child) {
	child->sibling = parent->child;
	child->parent = parent;
	parent->child = child;
	UIElem_Update(child);
}
void UIElem_RemoveFromParent(UIElem* child_to_remove) {
	// If child_to_remove is the root just return
	if(child_to_remove->parent == NULL) return;
	// First child of the parent
	UIElem* temp = child_to_remove->parent->child;

	if (temp != child_to_remove) {
		// If the first child isn't the element we want to delete...
		// We find the previous child
		while (temp->sibling != child_to_remove) temp = temp->sibling;
		// and link that to the next sibling.
		temp->sibling = child_to_remove->sibling;
	} else {
		// Else, just link the next sibling to the parent.
		child_to_remove->parent->child = child_to_remove->sibling;
	}

	child_to_remove->sibling = NULL;
	child_to_remove->parent = NULL;
}

/// \brief Recursive delete.
static void Delete_Helper(UIElem *uie) {
	if(uie == NULL) return;
	Delete_Helper(uie->sibling);
	Delete_Helper(uie->child);

	UIElem_RemoveCallbacks(uie);
	if(uie->data != NULL) free(uie->data);
	if(uie->tex != NULL) SDL_DestroyTexture(uie->tex);
	free(uie);
}
void UIElem_Delete(UIElem *uie) {
	UIElem_RemoveFromParent(uie);
	Delete_Helper(uie);
}

/* Utility */

inline Uint32 UIElem_Left(UIElem* uie)		{ return uie->abs_position.X; }
inline Uint32 UIElem_Right(UIElem* uie)		{ return uie->abs_position.X + uie->size.X; }
inline Uint32 UIElem_Top(UIElem* uie)		{ return uie->abs_position.Y; }
inline Uint32 UIElem_Bottom(UIElem* uie)	{ return uie->abs_position.Y + uie->size.Y; }

bool UIElem_IsParent(UIElem* parent, UIElem* child) {
	while (child != NULL) {
		if ((child = child->parent) == parent) {
			return true;
		}
	}
	return false;
}
UIElem* UIElem_FindElem(char* name, UIElem* current) {
	if (current == NULL) return NULL;
	if (strcmp(name, current->name) == 0) return current;

	UIElem* temp = UIElem_FindElem(name, current->child);
	if (temp != NULL) return temp;

	return UIElem_FindElem(name, current->sibling);
}

void UIElem_LoadTextures(UIElem* uie) {
	if (uie == NULL) return;

	if(strlen(uie->tex_path) > 0) uie->tex = IMG_LoadTexture(RGUI_Current_Window->renderer, uie->tex_path);
	UIElem_LoadTextures(uie->sibling);
	UIElem_LoadTextures(uie->child);
}
/// \brief Recursive abs_position update for children.
static void Update_Helper(UIElem* uie) {
	if (uie == NULL) return;
	Vec2 new_position = Vec2_Add(uie->parent->abs_position, uie->rel_position);

	if (Vec2_Compare(uie->abs_position, new_position)) return;

	uie->abs_position = new_position;

	Update_Helper(uie->sibling);
	Update_Helper(uie->child);
}
void UIElem_Update(UIElem* uie) {
	if (uie == NULL) return;
	if (uie->parent != NULL) {
		uie->abs_position = Vec2_Add(uie->parent->abs_position, uie->rel_position);
	} else {
		uie->abs_position = uie->rel_position;
	}

	Update_Helper(uie->child);
}

/// \brief Renders the element and calls the Tick event.
void UIElem_Draw(UIElem* uie) {
	if (uie == NULL) return;
	
	UIElem_TriggerEvent(uie, Tick);

	SDL_Rect rect = (SDL_Rect){
		uie->abs_position.X, uie->abs_position.Y,
		uie->size.X, uie->size.Y
	};
	if ((0x000000FF & uie->color) != 0x00000000) {
		SDL_FillRect(
			RGUI_Current_Window->surface,
			&rect,
			SDL_MapRGBA(
				RGUI_Current_Window->surface->format,
				uie->color >> 24,
				uie->color >> 16,
				uie->color >> 8,
				uie->color
			)
		);
	}
	if (uie->tex != NULL) {
		SDL_RenderCopy(RGUI_Current_Window->renderer, uie->tex, NULL, &rect);
	}

	UIElem_Draw(uie->sibling);
	UIElem_Draw(uie->child);
}

void UIElem_AddCallback(UIElem *root, char *name, EventType evt, UIElem_EventCallback callback) {
	UIElem *uie = UIElem_FindElem(name, root);

	if (uie == NULL) return;

	EvLinkedListNode* new_cb = malloc(sizeof(EvLinkedListNode));
	if (new_cb == NULL) exit(MALLOC_FAILED);

	new_cb->callback = callback;
	new_cb->next = uie->callbacks[evt];
	uie->callbacks[evt] = new_cb;
}
void UIElem_RemoveCallback(UIElem* root, char* name, EventType evt, UIElem_EventCallback callback) {
	UIElem *uie = UIElem_FindElem(name, root);
	if (uie == NULL) return;

	EvLinkedListNode **ind = &(uie->callbacks[evt]);

	while ((*ind)->callback != callback) {
		if ((*ind)->next == NULL) return;
		ind = &((*ind)->next);
	}
	
	EvLinkedListNode *elem = (*ind)->next;
	*ind = elem->next;
	free(elem);
}
void UIElem_RemoveCallbacks(UIElem* uie) {
	for (size_t i = 0; i < N_CALLBACKS; ++i) {
		EvLinkedListNode *current = uie->callbacks[i], *to_del;
		uie->callbacks[i] = NULL;
		while(current != NULL) {
			to_del = current;
			current = current->next;
			free(to_del);
		}
	}
}
void UIElem_TriggerEvent(UIElem* uie, EventType evt) {
	if (uie == NULL) return;
	EvLinkedListNode *elln = uie->callbacks[evt];
	while (elln != NULL) {
		elln->callback(uie);
		elln = elln->next;
	}
}

static void EventHelper(UIElem* uie, EventType evt) {
	if (uie == NULL) return;
	UIElem_TriggerEvent(uie, evt);
	EventHelper(uie->parent, evt);
}
static void Event_MouseEnter(void) {
	UIElem_TriggerEvent(_State[0], MouseEnter);
}
static void Event_MouseLeave(void) {
	UIElem_TriggerEvent(_State[1], MouseLeave);
}
void Event_LMBUp(void) {
	EventHelper(_State[0], LMBUp);
}

bool UIElem_MouseInside(UIElem* uie) {
	if (!(UIElem_Top(uie) <= _Mouse_Y && UIElem_Bottom(uie) >= _Mouse_Y &&
		UIElem_Left(uie) <= _Mouse_X && UIElem_Right(uie) >= _Mouse_X )) {
		return false;
	}

	UIElem_TriggerEvent(uie, MouseHover);

	// If mouse is inside a child
	bool flag = true;
	UIElem* child = uie->child;
	while (child != NULL) {
		if (!UIElem_MouseInside(child)) {
			child = child->sibling;
		} else {
			flag = false;
			break;
		}
	}

	if (flag && _State[0] != uie) { // THIS is _State[0]
		_State[1] = _State[0];
		_State[0] = uie;

		// If prev element was a child
		if (UIElem_IsParent(uie, _State[1])) {
			Event_MouseLeave();
		
		// If prev element was a parent
		} else if (UIElem_IsParent(_State[1], uie)) {
			Event_MouseEnter();

		// If prev element has nothing to do with the current
		} else {
			////////////////////////MouseLeave til common parent
			Event_MouseEnter();
			Event_MouseLeave();
			// MAYBE THIS CAN BE REMOVED
		}
	}

	return true;
}
