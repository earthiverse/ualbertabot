--- Instructions on how to compile and run ---

Load solution into and compile.

--- Functions that were changed ---

For a visual/flashier representation, please see the following:
https://github.com/davechurchill/ualbertabot/compare/master...earthiverse:master#diff-4e93374cc0a9cc3d1a3bd80f85fe9112

BOSS/source/ActionType.cpp
  canBuild()
BOSS/source/DFBB_BuildOrderSmartSearch.cpp
  DFBB_BuildOrderSmartSearch::setPrerequisiteGoalMax()
BOSS/source/GameState.cpp
  GameState::whyIsNotLegal()
SparCraft/external_binaries/dll/SDL.dll
SparCraft/external_binaries/dll/SDL_image.dll
SparCraft/external_binaries/dll/libpng15-15.dll
SparCraft/external_binaries/dll/opengl32.dll
UAlbertaBot/Source/BOSSManager.cpp
UAlbertaBot/Source/BuildingManager.cpp
  Include statements
  BuildingManager::getBuildingLocation()
UAlbertaBot/Source/CombatCommander.h
  class CombatCommander
UAlbertaBot/Source/Config.cpp

UAlbertaBot/Source/GameCommander.cpp
  GameCommander::update()
  GameCommander::handleUnitAssignments()
  GameCommander::isAssigned()
  GameCommander::setScoutUnits()
  GameCommander::setOverlords()
  GameCommander::getFirstSupplyProvider()
UAlbertaBot/Source/GameCommander.h
  include statements
  class GameCommander
UAlbertaBot/Source/LurkerManager.cpp
  Entirely new file created
UAlbertaBot/Source/LurkerManager.h
  Entirely new file created
UAlbertaBot/Source/MapTools.cpp
  MapTools::getNextExpansion()
UAlbertaBot/Source/MeleeManager.cpp
  MeleeManager::assignTargetsOld()
UAlbertaBot/Source/Micro.cpp
  Micro::SmartKiteTarget()
UAlbertaBot/Source/OverlordManager.cpp
  Entirely new file created
UAlbertaBot/Source/OverlordManager.h
  Entirely new file created
UAlbertaBot/Source/ProductionManager.cpp
  ProductionManager::ProductionManager()
  ProductionManager::update()
  ProductionManager::manageBuildOrderQueue()
UAlbertaBot/Source/ProductionManager.h
  class ProductionManager
UAlbertaBot/Source/RangedManager.cpp
  RangedManager::assignTargetsOld()
  Hard to tell as git diff believes every line altered.
UAlbertaBot/Source/Squad.cpp
  Squad::update()
  Squad::isEmpty()
UAlbertaBot/Source/Squad.h
  include statements
  class Squad
UAlbertaBot/Source/StrategyManager.cpp
  StrategyManager::getZergBuildOrderGoal()
  CountIdealMiners()
  CountIdealGasThingies()
  CountMineralThingiesRemaining()
  StrategyManager::getZergBuildOrderGoal()
UAlbertaBot/Source/UnitUtil.cpp
  UnitUtil::IsCombatUnit()
UAlbertaBot/Source/ZerglingManager.cpp
  entirely new file created
UAlbertaBot/Source/ZerglingManager.h
  entirely new file created
UAlbertaBot/UAlbertaBot_Config.txt
  Strategies
UAlbertaBot/UAlberta_Config.txt
  Strategies
