#ifndef LIGHT_H
#define LIGHT_H

#include "Vertex.h"
#include "Vector3.h"

struct Light
{
	enum LIGHT_TYPE
	{
		LIGHT_POINT = 0,
		LIGHT_DIRECTIONAL,
		LIGHT_SPOT,
	};
	LIGHT_TYPE type;
	Position position;
	Color color;
	float power;
	float kC;
	float kL;
	float kQ;
	Vector3 spotDirection;
	float cosCutoff;
	float cosInner;
	float exponent;

	Light()
	{
		type = LIGHT_POINT;
		color.Set(1, 1, 1);
		power = 1.f;
		kC = 1.f;
		kL = 0.f;
		kQ = 0.f;
		cosCutoff = cosInner = 0.8f;
		exponent = 1.f;
	}
};

#endif