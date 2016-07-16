/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: OBJ2RCM                                           |
|                             File: Helper.h                                             |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

enum COLORS
{
	COLOR_RED,
	COLOR_YELLOW,
	COLOR_GREEN,
	COLOR_BLUE,
	COLOR_WHITE
};

void SetTextColor(COLORS color);
void PrintWelcomeMessage();
void PrintCredits();
char * GetFilenameFromPath(char * path);