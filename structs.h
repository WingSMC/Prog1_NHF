#include <stdbool.h>

#ifndef STRUCTS_H
#define STRUCTS_H

/// A two dimensional vector
typedef struct Vec2 {
	/// \brief The horizontal coordinate
	int X;
	/// \brief The vertical coordinate
	int Y;
} Vec2;

inline Vec2 Vec2_Add(Vec2 v1, Vec2 v2) {
	v1.X += v2.X;
	v1.Y += v2.Y;
	return v1;
}

inline bool Vec2_Compare(Vec2 v1, Vec2 v2) {
	return (v1.X == v2.X && v1.Y == v2.Y);
}

#endif