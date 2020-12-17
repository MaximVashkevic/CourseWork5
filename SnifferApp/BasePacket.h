#pragma once
#include <Windows.h>
#include <memory>
#include <string>
#include "Headers.h"

class BasePacket
{
public:
	virtual std::wstring Protocol() const = 0;
	virtual std::wstring Source() const = 0;
	virtual std::wstring Destination() const = 0;
	virtual SIZE_T Length() const = 0;
	virtual ~BasePacket() = default;
};

