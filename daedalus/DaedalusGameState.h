#pragma once
#include <list>
#include <vector>
#include "DATFile.h"
#include "DaedalusStdlib.h"

namespace Daedalus {
  class DaedalusVM;

  namespace GameState {
    struct LogTopic {
      enum ELogStatus{
        LS_Running  = 1,
        LS_Success  = 2,
        LS_Failed   = 3,
        LS_Obsolete = 4
        };

      enum ESection {
        LT_Mission = 0,
        LT_Note    = 1
        };

      std::vector<std::string> entries;

      ELogStatus status;
      ESection section;
      };
    }  // namespace GameState
  }  // namespace Daedalus
