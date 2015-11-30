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
		if (!zerg || !zerg->exists() || !(zerg->getHitPoints() > 0)) {
			continue;
		}

		//BWAPI::Broodwar->printf("Works");
		//TODO: Check order type? Attack or Defend?

		if (zerglingUnitShouldRetreat(zerg, targets)) {
			BWAPI::Broodwar->printf("Should retreat");
			// TODO : Flee from closest enemy
			//BWAPI::Unit closest = zerg->getClosestUnit(BWAPI::Filter::IsEnemy);
			BWAPI::Unit target = getTarget(zerg, targets);
			if (!target || !target->exists() || !(target->getHitPoints() > 0)) {
				continue;
			}

			// Technique of retreat taken from Micro.cpp's SmartKiteTarget.
			try {
				BWAPI::Position fleeTo(zerg->getPosition() - target->getPosition() + zerg->getPosition());
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
	int highPriority = 0;
	double closestDist = std::numeric_limits<double>::infinity();
	BWAPI::Unit closestTarget = nullptr;

	// for each target possiblity
	for (auto & unit : targets)
	{
		int priority = getAttackPriority(attacker, unit);
		int distance = attacker->getDistance(unit);

		// if it's a higher priority, or it's closer, set it
		if (!closestTarget || (priority > highPriority) || (priority == highPriority && distance < closestDist))
		{
			closestDist = distance;
			highPriority = priority;
			closestTarget = unit;
		}
	}

	return closestTarget;
}

int ZerglingManager::getAttackPriority(BWAPI::Unit attacker, BWAPI::Unit unit) {
	BWAPI::UnitType type = unit->getType();

	if (attacker->getType() == BWAPI::UnitTypes::Protoss_Dark_Templar
		&& unit->getType() == BWAPI::UnitTypes::Terran_Missile_Turret
		&& (BWAPI::Broodwar->self()->deadUnitCount(BWAPI::UnitTypes::Protoss_Dark_Templar) == 0))
	{
		return 13;
	}

	if (attacker->getType() == BWAPI::UnitTypes::Protoss_Dark_Templar && unit->getType().isWorker())
	{
		return 12;
	}

	// highest priority is something that can attack us or aid in combat
	if (type == BWAPI::UnitTypes::Terran_Bunker)
	{
		return 11;
	}
	else if (type == BWAPI::UnitTypes::Terran_Medic ||
		(type.groundWeapon() != BWAPI::WeaponTypes::None && !type.isWorker()) ||
		type == BWAPI::UnitTypes::Terran_Bunker ||
		type == BWAPI::UnitTypes::Protoss_High_Templar ||
		type == BWAPI::UnitTypes::Protoss_Reaver ||
		(type.isWorker() && unitNearChokepoint(unit)))
	{
		return 10;
	}
	// next priority is worker
	else if (type.isWorker())
	{
		return 9;
	}
	// next is special buildings
	else if (type == BWAPI::UnitTypes::Zerg_Spawning_Pool)
	{
		return 5;
	}
	// next is special buildings
	else if (type == BWAPI::UnitTypes::Protoss_Pylon)
	{
		return 5;
	}
	// next is buildings that cost gas
	else if (type.gasPrice() > 0)
	{
		return 4;
	}
	else if (type.mineralPrice() > 0)
	{
		return 3;
	}
	// then everything else
	else
	{
		return 1;
	}
}


