#include "LurkerManager.h"

using namespace UAlbertaBot;

LurkerManager::LurkerManager() {
}

void LurkerManager::executeMicro(const BWAPI::Unitset& targets) {
  const BWAPI::Unitset& lurkers = getUnits();

  const int lurker_attack_range = BWAPI::UnitTypes::Zerg_Lurker.groundWeapon().maxRange();

  for (auto& lurker : lurkers) {
    // Check if lurker is okay.
    if (!lurker || !lurker->exists() || !(lurker->getHitPoints() > 0)) {
      continue;
    }

    bool burrowed = lurker->isBurrowed();

    BWAPI::Unit closest_enemy = lurker->getClosestUnit(BWAPI::Filter::IsEnemy && !BWAPI::Filter::IsFlying, lurker_attack_range * 2);
    if (closest_enemy != nullptr && !closest_enemy->getType().isBuilding()) {
      // Close enemy! Burrow!
      if (!burrowed) {
        lurker->burrow();
      }
      continue;
    }
    
    // Find closest unit and move towards it.
    BWAPI::Position lurker_position = lurker->getPosition();
    closest_enemy = lurker->getClosestUnit(BWAPI::Filter::IsEnemy && !BWAPI::Filter::IsFlying);
    if (closest_enemy != nullptr) {
      if (closest_enemy->getDistance(lurker) < lurker_attack_range) {
        lurker->burrow();
      }
      else {
        BWAPI::Position enemy_position = closest_enemy->getPosition();
        if (enemy_position.isValid()) {

          // Is lurker burrowed?
          if (burrowed) {
            lurker->unburrow();
            continue;
          }

          lurker->move(enemy_position);
        }
      }
      continue;
    }

    // No enemies anywhere on the map, apparently... find a choke point and burrow.
    BWTA::Chokepoint* closest_choke = BWTA::getNearestChokepoint(lurker_position);
    if (closest_choke != nullptr) {
      if (!burrowed && lurker->getDistance(closest_choke->getCenter()) < 64) {
        lurker->burrow();
      }
      else {
        // Is lurker burrowed?
        if (burrowed) {
          lurker->unburrow();
          continue;
        }

        lurker->move(closest_choke->getCenter());
      }
    }
  }
}