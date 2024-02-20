#ifndef OBJECT_BASE_H
#define OBJECT_BASE_H

#include <string>
#include "Message.h"

class ObjectBase
{
public:
	virtual ~ObjectBase() {}
	virtual bool Handle(Message *message) = 0;
};

#endif