//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "MobileApplication.h"
#include <CsvLogger.h>
#include <inet/common/ModuleAccess.h>
#include <utilities.h>
#include "BeaconFrame_m.h"

namespace smile {
namespace algorithm {
namespace atdoa {

Define_Module(MobileApplication);

void MobileApplication::initialize(int stage)
{
  IdealApplication::initialize(stage);

  if (stage == inet::INITSTAGE_APPLICATION_LAYER) {
    auto* mobilesLog = inet::getModuleFromPar<smile::Logger>(par("mobilesLoggerModule"), this, true);
    const auto entry = csv_logger::compose(getMacAddress(), getCurrentTruePosition());
    mobilesLog->append(entry);

    framesLog = inet::getModuleFromPar<smile::Logger>(par("mobileFramesLoggerModule"), this, true);
  }
}

}  // namespace atdoa
}  // namespace algorithm
}  // namespace smile
