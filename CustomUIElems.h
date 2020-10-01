#include "UIElem.h"

#ifndef CUSTOM_UIELEMS_H
#define CUSTOM_UIELEMS_H
/// \brief Some predefined elements.
typedef enum UIElem_Types {
	Div,
	Button
} UIElem_Types;

void Make_Button(UIElem* uie);

#endif