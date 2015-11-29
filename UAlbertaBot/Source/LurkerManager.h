#pragma once;

#include <Common.h>
#include "MicroManager.h"

namespace UAlbertaBot
{
  class LurkerManager : public MicroManager
  {
  public:
    LurkerManager();
    virtual void executeMicro(const BWAPI::Unitset & targets) override;
  };
}