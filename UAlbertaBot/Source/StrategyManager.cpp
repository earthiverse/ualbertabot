#include "Common.h"
#include "StrategyManager.h"
#include "UnitUtil.h"

using namespace UAlbertaBot;

// constructor
StrategyManager::StrategyManager() 
	: _selfRace(BWAPI::Broodwar->self()->getRace())
	, _enemyRace(BWAPI::Broodwar->enemy()->getRace())
    , _emptyBuildOrder(BWAPI::Broodwar->self()->getRace())
{
	
}

// get an instance of this
StrategyManager & StrategyManager::Instance() 
{
	static StrategyManager instance;
	return instance;
}

const int StrategyManager::getScore(BWAPI::Player player) const
{
	return player->getBuildingScore() + player->getKillScore() + player->getRazingScore() + player->getUnitScore();
}

const BuildOrder & StrategyManager::getOpeningBookBuildOrder() const
{
    auto buildOrderIt = _strategies.find(Config::Strategy::StrategyName);

    // look for the build order in the build order map
	if (buildOrderIt != std::end(_strategies))
    {
        return (*buildOrderIt).second._buildOrder;
    }
    else
    {
        UAB_ASSERT_WARNING(false, "Strategy not found: %s, returning empty initial build order", Config::Strategy::StrategyName.c_str());
        return _emptyBuildOrder;
    }
}

const bool StrategyManager::shouldExpandNow() const
{
	// if there is no place to expand to, we can't expand
	if (MapTools::Instance().getNextExpansion() == BWAPI::TilePositions::None)
	{
        BWAPI::Broodwar->printf("No valid expansion location");
		return false;
	}

	size_t numDepots    = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Terran_Command_Center)
                        + UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Protoss_Nexus)
                        + UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Zerg_Hatchery)
                        + UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Zerg_Lair)
                        + UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Zerg_Hive);
	int frame           = BWAPI::Broodwar->getFrameCount();
    int minute          = frame / (24*60);

	// if we have a ton of idle workers then we need a new expansion
	if (WorkerManager::Instance().getNumIdleWorkers() > 10)
	{
		return true;
	}

    // if we have a ridiculous stockpile of minerals, expand
    if (BWAPI::Broodwar->self()->minerals() > 3000)
    {
        return true;
    }

    // we will make expansion N after array[N] minutes have passed
    std::vector<int> expansionTimes = {5, 10, 20, 30, 40 , 50};

    for (size_t i(0); i < expansionTimes.size(); ++i)
    {
        if (numDepots < (i+2) && minute > expansionTimes[i])
        {
            return true;
        }
    }

	return false;
}

void StrategyManager::addStrategy(const std::string & name, Strategy & strategy)
{
    _strategies[name] = strategy;
}

const MetaPairVector StrategyManager::getBuildOrderGoal()
{
    BWAPI::Race myRace = BWAPI::Broodwar->self()->getRace();

    if (myRace == BWAPI::Races::Protoss)
    {
        return getProtossBuildOrderGoal();
    }
    else if (myRace == BWAPI::Races::Terran)
	{
		return getTerranBuildOrderGoal();
	}
    else if (myRace == BWAPI::Races::Zerg)
	{
		return getZergBuildOrderGoal();
	}

    return MetaPairVector();
}

const MetaPairVector StrategyManager::getProtossBuildOrderGoal() const
{
	// the goal to return
	MetaPairVector goal;

	int numZealots          = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Protoss_Zealot);
    int numPylons           = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Protoss_Pylon);
	int numDragoons         = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Protoss_Dragoon);
	int numProbes           = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Protoss_Probe);
	int numNexusCompleted   = BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Nexus);
	int numNexusAll         = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Protoss_Nexus);
	int numCyber            = BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Cybernetics_Core);
	int numCannon           = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Protoss_Photon_Cannon);
    int numScout            = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Protoss_Corsair);
    int numReaver           = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Protoss_Reaver);
    int numDarkTeplar       = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Protoss_Dark_Templar);

    if (Config::Strategy::StrategyName == "Protoss_ZealotRush")
    {
        goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Zealot, numZealots + 8));

        // once we have a 2nd nexus start making dragoons
        if (numNexusAll >= 2)
        {
            goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Dragoon, numDragoons + 4));
        }
    }
    else if (Config::Strategy::StrategyName == "Protoss_DragoonRush")
    {
        goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Dragoon, numDragoons + 6));
    }
    else if (Config::Strategy::StrategyName == "Protoss_Drop")
    {
        if (numZealots == 0)
        {
            goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Zealot, numZealots + 4));
            goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Shuttle, 1));
        }
        else
        {
            goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Zealot, numZealots + 8));
        }
    }
    else if (Config::Strategy::StrategyName == "Protoss_DTRush")
    {
        goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Dark_Templar, numDarkTeplar + 2));

        // if we have a 2nd nexus then get some goons out
        if (numNexusAll >= 2)
        {
            goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Dragoon, numDragoons + 4));
        }
    }
    else
    {
        UAB_ASSERT_WARNING(false, "Unknown Protoss Strategy Name: %s", Config::Strategy::StrategyName.c_str());
    }

    // if we have 3 nexus, make an observer
    if (numNexusCompleted >= 3)
    {
        goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observer, 1));
    }
    
    // add observer to the goal if the enemy has cloaked units
	if (InformationManager::Instance().enemyHasCloakedUnits())
	{
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Robotics_Facility, 1));
		
		if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Robotics_Facility) > 0)
		{
			goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observatory, 1));
		}
		if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Observatory) > 0)
		{
			goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observer, 1));
		}
	}

    // if we want to expand, insert a nexus into the build order
	if (shouldExpandNow())
	{
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Nexus, numNexusAll + 1));
	}

	return goal;
}

const MetaPairVector StrategyManager::getTerranBuildOrderGoal() const
{
	// the goal to return
	std::vector<MetaPair> goal;

    int numWorkers      = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Terran_SCV);
    int numCC           = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Terran_Command_Center);            
    int numMarines      = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Terran_Marine);
	int numMedics       = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Terran_Medic);
	int numWraith       = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Terran_Wraith);
    int numVultures     = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Terran_Vulture);
    int numGoliath      = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Terran_Goliath);
    int numTanks        = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode)
                        + UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode);
    int numBay          = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Terran_Engineering_Bay);

    if (Config::Strategy::StrategyName == "Terran_MarineRush")
    {
	    goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Terran_Marine, numMarines + 8));

        if (numMarines > 5)
        {
            goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Terran_Engineering_Bay, 1));
        }
    }
    else if (Config::Strategy::StrategyName == "Terran_4RaxMarines")
    {
	    goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Terran_Marine, numMarines + 8));
    }
    else if (Config::Strategy::StrategyName == "Terran_VultureRush")
    {
        goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Terran_Vulture, numVultures + 8));

        if (numVultures > 8)
        {
            goal.push_back(std::pair<MetaType, int>(BWAPI::TechTypes::Tank_Siege_Mode, 1));
            goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode, 4));
        }
    }
    else if (Config::Strategy::StrategyName == "Terran_TankPush")
    {
        goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode, 6));
        goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Terran_Goliath, numGoliath + 6));
        goal.push_back(std::pair<MetaType, int>(BWAPI::TechTypes::Tank_Siege_Mode, 1));
    }
    else
    {
        BWAPI::Broodwar->printf("Warning: No build order goal for Terran Strategy: %s", Config::Strategy::StrategyName.c_str());
    }



    if (shouldExpandNow())
    {
        goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Terran_Command_Center, numCC + 1));
        goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Terran_SCV, numWorkers + 10));
    }

	return goal;
}

int CountIdealMiners(int radius = 500) {
  BWAPI::Unitset minerals_and_gas = BWAPI::Unitset();

  BWAPI::Unitset units = BWAPI::Broodwar->self()->getUnits();
  for (auto& unit : units) {
    if (unit->getType().isResourceDepot()) {
      // find near mineral patches
      for (auto& resource : BWAPI::Broodwar->getUnitsInRadius(unit->getPosition(), radius, BWAPI::Filter::IsResourceContainer)) {
        minerals_and_gas.insert(resource);
      }
    }
  }

  int num_drones = 1;
  for (auto& resource : minerals_and_gas) {
    if (resource->getType().isMineralField()) {
      // Mineral field
      num_drones += 2;
    }
    else {
      // Gas
      num_drones += Config::Macro::WorkersPerRefinery;
    }
  }

  return num_drones;
}

int CountIdealGasThingies(int radius = 500) {
  BWAPI::Unitset gas_thingies = BWAPI::Unitset();

  BWAPI::Unitset units = BWAPI::Broodwar->self()->getUnits();
  for (auto& unit : units) {
    if (unit->getType().isResourceDepot()) {
      // find near mineral patches
      for (auto& resource : BWAPI::Broodwar->getUnitsInRadius(unit->getPosition(), radius, BWAPI::Filter::IsResourceContainer && !BWAPI::Filter::IsMineralField)) {
        if (resource->getType() == BWAPI::UnitTypes::Resource_Vespene_Geyser)
          gas_thingies.insert(resource);
      }
    }
  }

  return gas_thingies.size() * Config::Macro::WorkersPerRefinery;
}

int CountMineralThingiesRemaining(int radius = 500) {
  BWAPI::Unitset mineral_thingies = BWAPI::Unitset();

  BWAPI::Unitset units = BWAPI::Broodwar->self()->getUnits();
  for (auto& unit : units) {
    if (unit->getType().isResourceDepot()) {
      // find near mineral patches
      for (auto& resource : BWAPI::Broodwar->getUnitsInRadius(unit->getPosition(), radius, BWAPI::Filter::IsMineralField)) {
        mineral_thingies.insert(resource);
      }
    }
  }

  return mineral_thingies.size();
}

const MetaPairVector StrategyManager::getZergBuildOrderGoal() const
{
  // the goal to return
  std::vector<MetaPair> goal;
  BWAPI::Player self = BWAPI::Broodwar->self();

  int second = BWAPI::Broodwar->getFrameCount() / 24;
  int minerals = self->minerals();
  int gas = self->gas();

  // Get our current status
  unsigned short num_larva = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Zerg_Larva);

  unsigned short num_drones = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Zerg_Drone);
  unsigned short num_hatcheries = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Zerg_Hatchery);
  unsigned short num_lairs = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Zerg_Lair);
  unsigned short num_hives = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Zerg_Hive);

  unsigned short num_colonys = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Zerg_Creep_Colony);
  unsigned short num_sunkens = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Zerg_Sunken_Colony);
  unsigned short num_spores = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Zerg_Spore_Colony);

  unsigned short num_zerglings = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Zerg_Zergling);
  unsigned short num_hydralisks = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Zerg_Hydralisk);
  unsigned short num_lurkers = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Zerg_Lurker);

  unsigned short num_mutalisks = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Zerg_Mutalisk);
  unsigned short num_scourge = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Zerg_Scourge);

  unsigned short num_evolution_chambers = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Zerg_Evolution_Chamber);
  unsigned short num_hydralisk_den = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Zerg_Hydralisk_Den);
  unsigned short num_extractors = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Zerg_Extractor);

  bool has_evolution_chamber = num_evolution_chambers > 0;
  bool has_hydralisk_den = num_hydralisk_den > 0;
  bool has_extractor = num_extractors > 0;

  unsigned short num_mains = num_hatcheries + num_lairs + num_hives;
  bool has_lair = num_lairs > 0 || num_hives > 0;
  bool has_hive = num_hives > 0;

  bool researching_zergling_speed = self->isUpgrading(BWAPI::UpgradeTypes::Metabolic_Boost);
  int has_zergling_speed = self->getUpgradeLevel(BWAPI::UpgradeTypes::Metabolic_Boost);
  bool researching_attack_speed = self->isUpgrading(BWAPI::UpgradeTypes::Adrenal_Glands);
  int has_zergling_attack_speed = self->getUpgradeLevel(BWAPI::UpgradeTypes::Adrenal_Glands);
  bool spawning_pool_is_researching = researching_zergling_speed || researching_attack_speed;

  int zerg_melee_attack_level = self->getUpgradeLevel(BWAPI::UpgradeTypes::Zerg_Melee_Attacks);
  bool is_upgrading_melee_attack = self->isUpgrading(BWAPI::UpgradeTypes::Zerg_Melee_Attacks);
  int zerg_range_attack_level = self->getUpgradeLevel(BWAPI::UpgradeTypes::Zerg_Missile_Attacks);
  bool is_upgrading_range_attack = self->isUpgrading(BWAPI::UpgradeTypes::Zerg_Missile_Attacks);
  int zerg_ground_defense_level = self->getUpgradeLevel(BWAPI::UpgradeTypes::Zerg_Carapace);
  bool is_upgrading_ground_defense = self->isUpgrading(BWAPI::UpgradeTypes::Zerg_Carapace);

  bool is_overlord_speed_upgrading = self->isUpgrading(BWAPI::UpgradeTypes::Pneumatized_Carapace);
  int has_overlord_speed = self->getUpgradeLevel(BWAPI::UpgradeTypes::Pneumatized_Carapace);
  bool is_overlord_sight_upgrading = self->isUpgrading(BWAPI::UpgradeTypes::Antennae);
  int has_overlord_sight = self->getUpgradeLevel(BWAPI::UpgradeTypes::Antennae);
  bool is_overlord_carrying_upgrading = self->isUpgrading(BWAPI::UpgradeTypes::Ventral_Sacs);
  int has_overlord_carrying = self->getUpgradeLevel(BWAPI::UpgradeTypes::Ventral_Sacs);
  bool lair_is_researching = is_overlord_speed_upgrading || is_overlord_sight_upgrading || is_overlord_carrying_upgrading;

  bool researching_hydralisk_speed = self->isUpgrading(BWAPI::UpgradeTypes::Muscular_Augments);
  bool has_hydralisk_speed = self->getUpgradeLevel(BWAPI::UpgradeTypes::Muscular_Augments) == 1;
  bool researching_hydralisk_range = self->isUpgrading(BWAPI::UpgradeTypes::Grooved_Spines);
  bool has_hydralisk_range = self->getUpgradeLevel(BWAPI::UpgradeTypes::Grooved_Spines) == 1;
  bool researching_lurkers = self->isResearching(BWAPI::TechTypes::Lurker_Aspect);
  bool has_lurker_research = self->hasResearched(BWAPI::TechTypes::Lurker_Aspect);
  bool hydralisk_den_is_researching = researching_lurkers || researching_hydralisk_range || researching_hydralisk_speed;
  bool evolution_chamber_is_researching = is_upgrading_range_attack || is_upgrading_ground_defense || is_upgrading_melee_attack;


  // TODO: Find out number of expands with gas. (because ualberta bot only expands to those...)
  // TODO: Find out number of expands without gas.

  if (Config::Strategy::StrategyName == "Zerg_ZerglingRush") {
    goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Zergling, num_zerglings + 6));

  }
  else if (Config::Strategy::StrategyName == "Zerg_2HatchHydra") {
    if (has_hydralisk_den)
      goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Hydralisk, num_hydralisks + 1));
    else
      goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Hydralisk_Den, 1));

    if (num_hydralisks > 2 && !has_hydralisk_range && !hydralisk_den_is_researching) {
      hydralisk_den_is_researching = true;
      goal.push_back(std::pair<MetaType, int>(BWAPI::UpgradeTypes::Grooved_Spines, 1));
    }
    if (num_hydralisks > 5 && !has_hydralisk_speed && !hydralisk_den_is_researching) {
      goal.push_back(std::pair<MetaType, int>(BWAPI::UpgradeTypes::Muscular_Augments, 1));
    }
    if (num_hydralisks > 10) {
      // TODO: Upgrade Hydralisk Attack
    }

    goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Drone, num_drones + 1));

    return goal;

  }
  else if (Config::Strategy::StrategyName == "Zerg_3HatchMuta") {
    goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Hydralisk, num_hydralisks + 12));
    goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Drone, num_drones + 4));

  }
  else if (Config::Strategy::StrategyName == "Zerg_3HatchScourge") {
    if (num_scourge > 40) {
      goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Hydralisk, num_hydralisks + 12));
    }
    else {
      goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Scourge, num_scourge + 12));

    }

    goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Drone, num_drones + 4));
  }
  else if (Config::Strategy::StrategyName == "Butts") {
    /*
    The lowest of
    - # of bases * 16
    - Previous number of drones + 1
    */
    int num_desired_drones = std::min(num_mains * 16, num_drones + 1);
    goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Drone, num_desired_drones));

    // 25 seconds are up - start teching to zerglings
    if (second >= 25) {
      /*
      The lowest of:
      - # of 30 second intervals since the start of the game,
      - Previous # of zerglings + 2,
      - 20 zerglings
      */
      int num_desired_zerglings = std::min(second / 30, std::min(20, num_zerglings + 2));
      goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Zergling, num_desired_zerglings));

      if (has_hydralisk_den) {
        /*
        The lowest of
        - Previous # of hydras + 1
        - 20 hydras
        */
        BWAPI::Broodwar->printf("let's add hydras, woo!");
        int num_desired_hydras = std::min(20, num_hydralisks + 1);
        goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Hydralisk, num_desired_hydras));
      }
    }

    // contingency (stuff to do if we have a bunch of resources because the strategy above is not very good)
    if (BWAPI::Broodwar->self()->minerals() > 500 && BWAPI::Broodwar->self()->gas() > 250) {
      if (!has_hydralisk_den) {
        goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Hydralisk_Den, 1));
      }
      else if (!has_lair) {
        goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Lair, 1));
      }
      else if (num_mains < 3) {
        goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Hatchery, num_mains + 1));
      }
      else if (num_larva == 0) {
        // todo: find something to upgrade
      }
    }
    return goal;
  }
  else if (Config::Strategy::StrategyName == "Sunkens") {
    // MAKE DRONES!
    /*
    The lowest of
    - # of bases * 16
    - Previous number of drones + 1
    */

    int num_desired_drones = std::min(CountIdealMiners(), num_drones + 1);
    goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Drone, num_desired_drones));

    // Make sure we have a hydralisk den

    const int max_sunkens = 9;
    const int max_spores = 3;

    // MAKE SUNKENS & SPORES!
    // Step 1: Make colonies
    if (num_sunkens + num_spores + num_colonys < max_sunkens + max_spores) {
      goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Creep_Colony, 1));
    }
    // Step 2: Make sunkens & spores
    if (num_sunkens < max_sunkens && num_colonys > 0) {
      goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Sunken_Colony, num_sunkens + num_colonys));
    }
    else if (num_spores < max_spores && num_colonys > 0) {
      goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Spore_Colony, num_spores + num_colonys));
    }

    // MAKE HYDRAS!
    // Step 1: Get a hydralisk den
    if (num_sunkens >= 5 && !has_hydralisk_den)
      goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Hydralisk_Den, 1));
    // Step 2: Make hydras
    if (has_hydralisk_den)
      goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Hydralisk, num_hydralisks + 1));
    // Step 3: Upgrade hydras
    if (num_hydralisks > 0 && has_hydralisk_den) {
      if (!has_hydralisk_speed && !hydralisk_den_is_researching) {
        goal.push_back(std::pair<MetaType, int>(BWAPI::UpgradeTypes::Muscular_Augments, 1));
        hydralisk_den_is_researching = true;
      }
      else if (!has_hydralisk_range && !hydralisk_den_is_researching) {
        goal.push_back(std::pair<MetaType, int>(BWAPI::UpgradeTypes::Grooved_Spines, 1));
        hydralisk_den_is_researching = true;
      }
    }

    // MAKE LURKERS!
    if (num_hydralisks > 5) {
      if (!has_lurker_research && has_lair && !hydralisk_den_is_researching) {
        goal.push_back(std::pair<MetaType, int>(BWAPI::TechTypes::Lurker_Aspect, 1));
        hydralisk_den_is_researching = true;
      }
      else if (has_lurker_research && num_lurkers < num_hydralisks / 4) {
        goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Lurker, num_lurkers + 1));
      }
    }

    if (num_hydralisks > 14) {
      // Upgrade some things!
      if (!has_evolution_chamber) {
        // Make an evolution chamber!
        goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Evolution_Chamber, 1));
      }

      // Get overlord speed!
      if (has_lair && !lair_is_researching) {
        if (!has_overlord_speed) {
          goal.push_back(std::pair<MetaType, int>(BWAPI::UpgradeTypes::Pneumatized_Carapace, 1));
        }
      }
      
      if (!evolution_chamber_is_researching && zerg_range_attack_level < 1) {
        goal.push_back(std::pair<MetaType, int>(BWAPI::UpgradeTypes::Zerg_Missile_Attacks, 1));
      }
      else if (!evolution_chamber_is_researching && zerg_ground_defense_level < 1) {
        goal.push_back(std::pair<MetaType, int>(BWAPI::UpgradeTypes::Zerg_Carapace, 1));
      }
      else if (!evolution_chamber_is_researching && zerg_range_attack_level < 2 && has_lair) {
        goal.push_back(std::pair<MetaType, int>(BWAPI::UpgradeTypes::Zerg_Missile_Attacks, 2));
      }
      else if (!evolution_chamber_is_researching && zerg_ground_defense_level < 2 && has_lair) {
        goal.push_back(std::pair<MetaType, int>(BWAPI::UpgradeTypes::Zerg_Carapace, 2));
      }
    }

	// If we have enough drones and we haven't expanded in a while
	if (num_drones > CountIdealMiners() - 6 && num_mains * 240 < second && num_hydralisks > 4 * num_mains
		|| num_lurkers > pow(2, num_mains)) {
      // IT'S HATCHERY TIME!
      goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Hatchery, num_hatcheries + 1));
    }
    if (num_hatcheries >= 2 && num_lairs == 0 && num_hives == 0) {
      // THIS IS DANGEROUS UNTIL CHURCHILL FIXES SOME THINGS
      goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Lair, 1));
    }
    if (CountMineralThingiesRemaining() < 3) {
      // Minerals are running thin! Make a hatchery!
      goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Hatchery, num_hatcheries + 1));
    }

    // Get more gas!
    if (CountIdealGasThingies() > num_extractors && second > 600) {
      goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Extractor, num_extractors + 1));
    }

    return goal;
  }
  else if (Config::Strategy::StrategyName == "Lurkers") {
    // MAKE DRONES!
    /*
    The lowest of
    - # of bases * 15
    - Previous number of drones + 1
    */
    int num_desired_drones = std::min(CountIdealMiners(), num_drones + 1);
    goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Drone, num_desired_drones));

    goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Hydralisk, num_hydralisks + 2));
    return goal;

    // expand
    if (num_desired_drones % 15 == 0 && num_desired_drones < 24 && num_lurkers > 4 * num_mains) {
      goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Hatchery, num_mains + 1));
      goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Drone, 2));
    }

    goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Hydralisk, num_hydralisks + 3));
    goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Lurker, num_lurkers + 1));

    return goal;
  } else if (Config::Strategy::StrategyName == "4Pool") {
    // Can we do this?
    Config::Modules::UsingScoutManager = false;

    goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Zergling, num_zerglings + 1));

    // Get some more drones and gas so we can upgrade
    if (second > 240) {
      goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Drone, num_drones + 1));
      goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Zergling, num_zerglings + 2));
    }

    if (minerals > 1000) {
      BWAPI::Broodwar->printf("LET'S MAKE HATCHERIES!");
      num_hatcheries++;
      goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Hatchery, num_hatcheries));

      if (!has_zergling_speed && !spawning_pool_is_researching) {
        if (!has_extractor)
          goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Extractor, 1));
        goal.push_back(std::pair<MetaType, int>(BWAPI::UpgradeTypes::Metabolic_Boost, 1));
      }
    }

    return goal;
  }

  if (shouldExpandNow()) {
    goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Hatchery, num_mains + 1));
    goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Drone, num_drones + 10));
  }

  return goal;
}

void StrategyManager::readResults()
{
    if (!Config::Modules::UsingStrategyIO)
    {
        return;
    }

    std::string enemyName = BWAPI::Broodwar->enemy()->getName();
    std::replace(enemyName.begin(), enemyName.end(), ' ', '_');

    std::string enemyResultsFile = Config::Strategy::ReadDir + enemyName + ".txt";
    
    std::string strategyName;
    int wins = 0;
    int losses = 0;

    FILE *file = fopen ( enemyResultsFile.c_str(), "r" );
    if ( file != nullptr )
    {
        char line [ 4096 ]; /* or other suitable maximum line size */
        while ( fgets ( line, sizeof line, file ) != nullptr ) /* read a line */
        {
            std::stringstream ss(line);

            ss >> strategyName;
            ss >> wins;
            ss >> losses;

            //BWAPI::Broodwar->printf("Results Found: %s %d %d", strategyName.c_str(), wins, losses);

            if (_strategies.find(strategyName) == _strategies.end())
            {
                //BWAPI::Broodwar->printf("Warning: Results file has unknown Strategy: %s", strategyName.c_str());
            }
            else
            {
                _strategies[strategyName]._wins = wins;
                _strategies[strategyName]._losses = losses;
            }
        }

        fclose ( file );
    }
    else
    {
        //BWAPI::Broodwar->printf("No results file found: %s", enemyResultsFile.c_str());
    }
}

void StrategyManager::writeResults()
{
    if (!Config::Modules::UsingStrategyIO)
    {
        return;
    }

    std::string enemyName = BWAPI::Broodwar->enemy()->getName();
    std::replace(enemyName.begin(), enemyName.end(), ' ', '_');

    std::string enemyResultsFile = Config::Strategy::WriteDir + enemyName + ".txt";

    std::stringstream ss;

    for (auto & kv : _strategies)
    {
        const Strategy & strategy = kv.second;

        ss << strategy._name << " " << strategy._wins << " " << strategy._losses << "\n";
    }

    Logger::LogOverwriteToFile(enemyResultsFile, ss.str());
}

void StrategyManager::onEnd(const bool isWinner)
{
    if (!Config::Modules::UsingStrategyIO)
    {
        return;
    }

    if (isWinner)
    {
        _strategies[Config::Strategy::StrategyName]._wins++;
    }
    else
    {
        _strategies[Config::Strategy::StrategyName]._losses++;
    }

    writeResults();
}

void StrategyManager::setLearnedStrategy()
{
    // we are currently not using this functionality for the competition so turn it off 
    return;

    if (!Config::Modules::UsingStrategyIO)
    {
        return;
    }

    const std::string & strategyName = Config::Strategy::StrategyName;
    Strategy & currentStrategy = _strategies[strategyName];

    int totalGamesPlayed = 0;
    int strategyGamesPlayed = currentStrategy._wins + currentStrategy._losses;
    double winRate = strategyGamesPlayed > 0 ? currentStrategy._wins / static_cast<double>(strategyGamesPlayed) : 0;

    // if we are using an enemy specific strategy
    if (Config::Strategy::FoundEnemySpecificStrategy)
    {        
        return;
    }

    // if our win rate with the current strategy is super high don't explore at all
    // also we're pretty confident in our base strategies so don't change if insufficient games have been played
    if (strategyGamesPlayed < 5 || (strategyGamesPlayed > 0 && winRate > 0.49))
    {
        BWAPI::Broodwar->printf("Still using default strategy");
        return;
    }

    // get the total number of games played so far with this race
    for (auto & kv : _strategies)
    {
        Strategy & strategy = kv.second;
        if (strategy._race == BWAPI::Broodwar->self()->getRace())
        {
            totalGamesPlayed += strategy._wins + strategy._losses;
        }
    }

    // calculate the UCB value and store the highest
    double C = 0.5;
    std::string bestUCBStrategy;
    double bestUCBStrategyVal = std::numeric_limits<double>::lowest();
    for (auto & kv : _strategies)
    {
        Strategy & strategy = kv.second;
        if (strategy._race != BWAPI::Broodwar->self()->getRace())
        {
            continue;
        }

        int sGamesPlayed = strategy._wins + strategy._losses;
        double sWinRate = sGamesPlayed > 0 ? currentStrategy._wins / static_cast<double>(strategyGamesPlayed) : 0;
        double ucbVal = C * sqrt( log( (double)totalGamesPlayed / sGamesPlayed ) );
        double val = sWinRate + ucbVal;

        if (val > bestUCBStrategyVal)
        {
            bestUCBStrategy = strategy._name;
            bestUCBStrategyVal = val;
        }
    }

    Config::Strategy::StrategyName = bestUCBStrategy;
}