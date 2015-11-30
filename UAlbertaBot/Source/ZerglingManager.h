#pragma once

#include <Common.h>
#include "MicroManager.h"

namespace UAlbertaBot
{
class ZerglingManager : public MicroManager
{
public:

	ZerglingManager();
	~ZerglingManager() {} 
	void executeMicro(const BWAPI::Unitset & targets);

	bool zerglingUnitShouldRetreat(BWAPI::Unit attacker, const BWAPI::Unitset & targets);
	BWAPI::Unit getTarget(BWAPI::Unit attacker, const BWAPI::Unitset & targets);
	int getAttackPriority(BWAPI::Unit attacker, BWAPI::Unit unit);
};



}