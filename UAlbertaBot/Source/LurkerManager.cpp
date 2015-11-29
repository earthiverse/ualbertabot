#include "LurkerManager.h"

using namespace UAlbertaBot;

LurkerManager::LurkerManager() {
}

void LurkerManager::executeMicro(const BWAPI::Unitset& targets) {
  const BWAPI::Unitset& lurkers = getUnits();

  int attack_range = BWAPI::UnitTypes::Zerg_Lurker.groundWeapon().maxRange();

  for (auto& lurker : lurkers) {
    BWAPI::Position lurker_position = lurker->getPosition();
    BWAPI::Unit closest_enemy = lurker->getClosestUnit(BWAPI::Filter::IsEnemy && !BWAPI::Filter::IsFlying && BWAPI::Filter::IsVisible);
    int enemy_distance;
    bool enemy_can_move;
    if (closest_enemy) {
      enemy_distance = lurker->getDistance(closest_enemy);
      enemy_can_move = closest_enemy->canMove();
    }
    else {
      enemy_distance = lurker->getType().sightRange();
      enemy_can_move = false;
    }
    BWTA::Chokepoint* closest_choke = BWTA::getNearestChokepoint(lurker_position);
    // TODO: What if there's no choke point?
    // TODO: What if the choke point isn't reachable?
    BWAPI::Position choke_position = closest_choke->getCenter();
    int choke_distance = lurker->getDistance(choke_position);

    bool burrowed = lurker->isBurrowed();

    size_t order_type = order.getType();
    BWAPI::Position order_position = order.getPosition();

    // Lurker might be in trouble
    // TODO: If enemy is moving away from us, follow it instead of just burrowing.
    if ((enemy_can_move && enemy_distance <= attack_range * 2)
      || (!enemy_can_move && enemy_distance <= attack_range)) {
      if (!burrowed) {
        BWAPI::Broodwar->printf("There's a close unit, I'm Burrowing!");
        lurker->burrow();
      }
      continue;
    }

    // We're on the offensive!
    if (order_type == SquadOrderTypes::Attack) {
      if (lurker->isBurrowed())
        lurker->unburrow();
      else
        lurker->move(order_position);
      continue;
    }

    // We're on the defensive!
    if (order_type == SquadOrderTypes::Defend) {
      BWAPI::Broodwar->printf("We're defending");
      // Fall back!
      if (lurker->isBurrowed())
        lurker->unburrow();
      else
        lurker->move(order_position);
      continue;
    }
  }
}