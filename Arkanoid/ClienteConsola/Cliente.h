#pragma once
#include <tchar.h>
#include "../DLL/DLL.h"

#define DEFAULT_KEY_MOVE_RIGHT 100
#define DEFAULT_KEY_MOVE_LEFT 97
#define DEFAULT_KEY_QUIT 27

typedef struct {
	TCHAR username[MAX_NAME];
	int move_right;
	int move_left;
} Configurations;