#pragma once
#include "Angel\Angel.h"

class HSVColorPicker
{
public:
	static vec3 HSVToRGB(float h, float s, float v);
	vec3 PickColor();

	HSVColorPicker(void);
	~HSVColorPicker(void);

private:
	vec3 PickRandomColor();

};

