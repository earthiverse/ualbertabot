#pragma once

#include <Common.h>
#include "MicroManager.h"

namespace UAlbertaBot
{
class ZerglingManager : public MicroManager
{
public:

	ZerglingManager();
	void executeMicro(const BWAPI::Unitset & targets);

};



}