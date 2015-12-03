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
			if (!targets.empty()) {
				Micro::SmartMove(zerg, getRetreatPosition(zerg , targets));
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

	if (targets.empty()) {
		return false;
	}

	// NOTE: getting the initial hit points doesn't work....
	int hp = attacker->getHitPoints();
	int max_hp = attacker->getType().maxHitPoints();
	//BWAPI::Unit closest = attacker->getClosestUnit(BWAPI::Filter::IsEnemy);
	if ((1.25) * hp < max_hp) {
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
	
	// With Zerglings, we want to attack: hydralisks, tanks, marines, dragoons

	if (type == BWAPI::UnitTypes::Zerg_Hydralisk
		|| type == BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode
		|| type == BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode
		|| type == BWAPI::UnitTypes::Terran_Marine
		|| type == BWAPI::UnitTypes::Protoss_Dragoon)
	{
		return 14;
	}

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
	// With Zerglings, we want to avoid firebats, archons, zealots, ultralisks
	else if (type == BWAPI::UnitTypes::Terran_Firebat
		|| type == BWAPI::UnitTypes::Protoss_Archon
		|| type == BWAPI::UnitTypes::Protoss_Zealot
		|| type == BWAPI::UnitTypes::Zerg_Ultralisk)
	{
		return 7;
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

// Idea is to find the direction that is most away from the nearest enemy units.
BWAPI::Position ZerglingManager::getRetreatPosition(BWAPI::Unit attacker, const BWAPI::Unitset & targets) {

	//BWAPI::Broodwar->drawCircleMap(attacker->getPosition(), 400, BWAPI::Colors::Red);

	// Units get stuck in corners. Corners not usually by chokes. Maybe if we can see that we are far in corner,
	// away from choke, we should head to choke, then hopefully get into the open again.
	/*if (!unitNearChokepoint(attacker)) {
		return ;
	}*/

	double retreatVector_x = 0.0;
	double retreatVector_y = 0.0;

	for (auto & target : targets) {

		// ignore far away targets
		if (attacker->getDistance(target) > 400) {
			continue;
		}
		//BWAPI::Broodwar->drawLineMap(attacker->getPosition(), target->getPosition(), BWAPI::Colors::Red);
		double target_x = attacker->getPosition().x - target->getPosition().x;
		double target_y = attacker->getPosition().y - target->getPosition().y;
		double target_length = getVectorLength(target_x, target_y);

		// make vector a unit vector
		target_x /= target_length;
		target_y /= target_length;

		// add vectors up
		retreatVector_x += target_x;
		retreatVector_y += target_y;
	}

	BWAPI::Position retreatPosition((int)(retreatVector_x * 50) + attacker->getPosition().x, ((int)retreatVector_y * 50) + attacker->getPosition().y);
	//BWAPI::Broodwar->drawLineMap(attacker->getPosition(), retreatPosition, BWAPI::Colors::Green);

	return retreatPosition;
}

double ZerglingManager::getVectorLength(double x, double y) {
	return sqrt((x*x) + (y*y));
}
