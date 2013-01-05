#include "HSVColorPicker.h"
#include <math.h>
#include <random>

HSVColorPicker::HSVColorPicker(void)
{

}

vec3 HSVColorPicker::HSVToRGB(float h, float s, float v)
{
	//HSV to RGB conversion
    vec3 returnCol = vec3(0);
               
    int i;
    float f, p, q, t;
               
    if( s == 0 ) {
            // achromatic (grey)
            returnCol.x = v;
            returnCol.y = v;
            returnCol.z = v;
            return returnCol;
    }
    h /= 60;                        // sector 0 to 5
	i = (int) floor( (double)h );
    f = h - i;                      // factorial part of h
    p = v * ( 1 - s );
    q = v * ( 1 - s * f );
    t = v * ( 1 - s * ( 1 - f ) );
    switch( i ) {
            case 0:
                    returnCol.x = v;
                    returnCol.y = t;
                    returnCol.z = p;
                    break;
            case 1:
                    returnCol.x = q;
                    returnCol.y = v;
                    returnCol.z = p;
                    break;
            case 2:
                    returnCol.x = p;
                    returnCol.y = v;
                    returnCol.z = t;
                    break;
            case 3:
                    returnCol.x = p;
                    returnCol.y = q;
                    returnCol.z = v;
                    break;
            case 4:
                    returnCol.x = t;
                    returnCol.y = p;
                    returnCol.z = v;
                    break;
            default:                // case 5:
                    returnCol.x = v;
                    returnCol.y = p;
                    returnCol.z = q;
                    break;
    }
 
    return returnCol;
}

vec3 HSVColorPicker::PickColor()
{
	return PickRandomColor();
}

vec3 HSVColorPicker::PickRandomColor()
{
	float hue =  360.f * ( rand() % 1000 / (float) 1000);
	
	return HSVToRGB(hue, 1., 1.);
}


HSVColorPicker::~HSVColorPicker(void)
{
}
