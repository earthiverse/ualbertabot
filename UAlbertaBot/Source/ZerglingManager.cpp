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
		//BWAPI::Broodwar->printf("Works");
		//TODO: Check order type? Attack or Defend?

		if (zerglingUnitShouldRetreat(zerg, targets)) {
			BWAPI::Broodwar->printf("Should retreat");
			// TODO : Flee from closest enemy
			BWAPI::Unit closest = zerg->getClosestUnit(BWAPI::Filter::IsEnemy);
			// Technique of retreat taken from Micro.cpp's SmartKiteTarget.
			try {
				BWAPI::Position fleeTo(zerg->getPosition() - closest->getPosition() + zerg->getPosition());
				BWAPI::Broodwar->drawLineMap(zerg->getPosition(), fleeTo, BWAPI::Colors::Cyan);
				Micro::SmartMove(zerg, fleeTo);
			}
			catch (...) {
				BWAPI::Broodwar->printf("Exception!");
			}
		} 
		else if (!zerglingTargets.empty()) {
			BWAPI::Unit target = getTarget(zerg, targets);
			Micro::SmartAttackUnit(zerg, target);
		}
		// No targets
		else {
			// Not near the order
			if (zerg->getDistance(order.getPosition()) > 100) {
				// Move closer (ie, regroup?
				Micro::SmartMove(zerg, order.getPosition());
			}
		}

	}
}

bool ZerglingManager::zerglingUnitShouldRetreat(BWAPI::Unit attacker, const BWAPI::Unitset & targets) {
	// NOTE: getting the initial hit points doesn't work....
	int hp = attacker->getHitPoints();
	int max_hp = attacker->getType().maxHitPoints();
	//BWAPI::Unit closest = attacker->getClosestUnit(BWAPI::Filter::IsEnemy);
	if ((2.0) * hp < max_hp) {
	//if ((1.5) * hp < max_hp) {
		//if (closest->isAttacking() || closest->isStartingAttack() || closest->isAttackFrame()) {
			return true;
		//}
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

