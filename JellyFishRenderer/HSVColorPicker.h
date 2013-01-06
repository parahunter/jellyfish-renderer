#pragma once
#include "Angel\Angel.h"

enum ColorState {COLOR_SINGLE, COLOR_PULSATING, COLOR_RANDOM};

class HSVColorPicker
{
public:
	static vec3 HSVToRGB(float h, float s, float v);
	vec3 PickColor();
	ColorState state;
	float currentHue;
	HSVColorPicker(void);
	HSVColorPicker(float initialHue);
	~HSVColorPicker(void);
	void UpdateColor(float t);
	void HSVColorPicker::UpdateState();

private:
	vec3 PickRandomColor();

};

