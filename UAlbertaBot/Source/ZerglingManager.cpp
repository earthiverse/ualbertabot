#include "ZerglingManager.h"
#include "UnitUtil.h"

using namespace UAlbertaBot;

ZerglingManager::ZerglingManager()
{
}

void ZerglingManager::executeMicro(const BWAPI::Unitset & targets)
{
	const BWAPI::Unitset& zerglings = getUnits();

	// TODO: Be more selective with the targets?
	BWAPI::Unitset zerglingTargets = targets;

	for (auto & zerg : zerglings) {
		//BWAPI::Broodwar->drawCircleMap(zerg->getPosition(), 25, BWAPI::Colors::Red);

		//TODO: Check order type? Attack or Defend?

		if (zerglingUnitShouldRetreat(zerg, targets)) {
			BWAPI::Broodwar->printf("Should retreat");
			// TODO : Flee from closest enemy
			BWAPI::Position fleeTo(BWAPI::Broodwar->self()->getStartLocation());
			BWAPI::Broodwar->drawLineMap(zerg->getPosition(), fleeTo, BWAPI::Colors::Cyan);
			Micro::SmartMove(zerg, fleeTo);
		} 
		else if (!zerglingTargets.empty()) {
			BWAPI::Unit target = getTarget(zerg, targets);
			Micro::SmartAttackUnit(zerg, target);
		}

	}
}

bool ZerglingManager::zerglingUnitShouldRetreat(BWAPI::Unit attacker, const BWAPI::Unitset & targets) {
	// NOTE: getting the initial hit points doesn't work....
	int hp = attacker->getHitPoints();
	int max_hp = attacker->getType().maxHitPoints();
	if ((1.5) * hp < max_hp && attacker->isUnderAttack()) {
		return true;
	}

	return false;
}

BWAPI::Unit ZerglingManager::getTarget(BWAPI::Unit attacker, const BWAPI::Unitset & targets) {
	//int highPriority = 0;
	double closestDist = std::numeric_limits<double>::infinity();
	BWAPI::Unit closestTarget = nullptr;

	// for each target possiblity
	for (auto & unit : targets)
	{
		//int priority = getAttackPriority(meleeUnit, unit);
		int distance = attacker->getDistance(unit);

		// if it's a higher priority, or it's closer, set it
		//if (!closestTarget || (priority > highPriority) || (priority == highPriority && distance < closestDist))
		if (closestDist > distance)
		{
			closestDist = distance;
			//highPriority = priority;
			closestTarget = unit;
		}
	}

	return closestTarget;
}

