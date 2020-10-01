#include <stdbool.h>
#include <string.h>
#include <SDL.h>

#include "Error.h"
#include "structs.h"

#ifndef UI_ELEM_H
#define UI_ELEM_H

struct UIElem;

/// \brief Utility for indexing event arrays
typedef enum EventType {
	MouseEnter = 0,
	MouseLeave = 1,
	MouseHover = 2,
	LMBDown = 3,
	LMBUp = 4,
	Scroll = 5,
	Drag = 6,
	Drop = 7,
	/// \brief Called with every Draw
	Tick = 8
} EventType;
/// \brief The number of EventTypes
#define N_CALLBACKS 9

/// \brief Defines the type UIElem_EventCallback which can be any function with one UIElem* parameter
typedef void (*UIElem_EventCallback)(struct UIElem*);
/// \brief The basis for linking the event callbacks together
typedef struct EvLinkedListNode {
	UIElem_EventCallback callback;
	struct EvLinkedListNode* next;
} EvLinkedListNode;


/****************************************************************************************************/
/// \brief Function prototype should apply default callbacks and process the UIElem.data field
typedef void (*UIElem_Builder)(struct UIElem*);
/****************************************************************************************************/


/// \brief The struct at the backbone of the UI.
///
/// These structs are linked together into a hierarchical tree structure
/// to handle events.
///
/// The locations and sizes of siblings should not overlap,
/// because I knew that in the project no elements will overlap,
/// when the mouse_inside() finds the first element your cursor is on,
/// no more effort will be made to find other elements.
///
/// Events just like in the HTML DOM with JS apply for parents
/// (except mouse_enter and mouse_leave).
typedef struct UIElem {
	/// \brief The name we can use to add callbacks.
	char name[20+1];
	/// \brief The position of the element relative to the parent's upper left corner.
	Vec2 rel_position;
	/// \brief The absolute position of the element, calculated on update().
	Vec2 abs_position;
	/// \brief The size of the element in px.
	///
	/// position.X + size.X <= parent->size.X
	Vec2 size;
	/// \brief The path to the texture.
	char tex_path[51];

	/// \brief The default backgound color.
	Uint32 color;
	/// \brief The texture of the element (background image).
	SDL_Texture *tex;

	/// \brief The parent in the hierarchy.
	struct UIElem *parent;
	/// \brief The next sibling.
	struct UIElem *sibling;
	/// \brief The first child of the parent.
	struct UIElem *child;
	/// \brief The functions of the UIElem.
	///
	/// eg. translate the UIElem vertically when scrolled:<br>
	/// UIElem_AddCallback(window, "that_red_x", Click, exit);
	EvLinkedListNode *callbacks[N_CALLBACKS];
	/// \brief For storing arbitrary data, will be freed on delete.
	void *data;
} UIElem;


/// \brief Initializes the UIElem.
///
/// \param position	Relative position to parent.
/// \param size		Width and height.
/// \param *tex		Texture to render, can be NULL.
/// \param color	The default background color, if the alpha is 0x00 it will be ignored.
/// \param name		The reference name for adding callbacks, MAX 20 character.
UIElem* UIElem_Init(Vec2 position, Vec2 size, char* tex_path, Uint32 color, char* name);
/// \brief Adds a child to the children linked list.
void UIElem_AddChild(UIElem* parent, UIElem* child);
/// \brief Removes the child from the children linked list.
void UIElem_RemoveFromParent(UIElem *child);
/// \brief Frees up the UIElem and its children
void UIElem_Delete(UIElem *uie);

/* Utility */

/// \brief the (X) coordinate of the element's right side.
extern inline Uint32 UIElem_Left(UIElem* uie);
/// \brief the (X) coordinate of the element's left side.
extern inline Uint32 UIElem_Right(UIElem* uie);
/// \brief the (Y) coordinate of the element's top side.
extern inline Uint32 UIElem_Top(UIElem* uie);
/// \brief the (Y) coordinate of the element's bottom side.
extern inline Uint32 UIElem_Bottom(UIElem* uie);

/// \brief Tells whether the "parent" is above the "child" in the hierarchy.
bool UIElem_IsParent(UIElem* parent, UIElem* child);
/// \brief Finds an element with the given name in a tree.
UIElem* UIElem_FindElem(char* name, UIElem* root);

/* Draw & Update */
/// \brief Load the textures from the files.
void UIElem_LoadTextures(UIElem* root);
/// \brief Updates computed properties of the element and the children such as abs_position.
void UIElem_Update(UIElem* uie);
/// \brief Draws the UI_Elem on the screen.
void UIElem_Draw(UIElem* uie);



/* Callbacks */

/// \brief Adds a callback to the list.
void UIElem_AddCallback(UIElem *root, char* name, EventType evt, UIElem_EventCallback callback);
/// \brief Removes one callback
void UIElem_RemoveCallback(UIElem* root, char* name, EventType evt, UIElem_EventCallback callback);
/// \brief Frees the callback linked lists.
void UIElem_RemoveCallbacks(UIElem* uie);
/// \brief Fires the event.
void UIElem_TriggerEvent(UIElem* uie, EventType evt);

/* Event triggers */

/// \brief Tells whether the mouse is "inside" the UI_Elem.
bool UIElem_MouseInside(UIElem* uie);
/// \brief Should be called on an SDL
void Event_LMBUp(void);

//RGUI: init
//Builders
//event - til - common parent
//UIElem_GetRoot(uie)


//DRAW TEX
//FINISH: EVENTS: click, scroll, drag

#endif
