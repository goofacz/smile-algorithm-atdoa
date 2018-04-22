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

#include "AnchorApplication.h"
#include <CsvLogger.h>
#include <Logger.h>
#include <inet/common/ModuleAccess.h>
#include <utilities.h>
#include "BeaconFrame_m.h"
#include "CsvLoggerExtensions.h"

namespace smile {
namespace algorithm {
namespace atdoa {

Define_Module(AnchorApplication);

void AnchorApplication::initialize(int stage)
{
  IdealApplication::initialize(stage);

  if (stage == inet::INITSTAGE_APPLICATION_LAYER) {
    auto* anchorsLog = inet::getModuleFromPar<smile::Logger>(par("anchorsLoggerModule"), this, true);
    const auto entry = csv_logger::compose(getMacAddress(), getCurrentTruePosition());
    anchorsLog->append(entry);

    framesLog = inet::getModuleFromPar<smile::Logger>(par("anchorFramesLoggerModule"), this, true);
  }
}

void AnchorApplication::handleIncommingMessage(cMessage* newMessage)
{
  std::unique_ptr<cMessage>{newMessage};
}

void AnchorApplication::handleRxCompletionSignal(const IdealRxCompletion& completion)
{
  const auto frame = omnetpp::check_and_cast<const BeaconFrame*>(completion.getFrame());
  const auto entry = csv_logger::compose(getMacAddress(), completion, *frame);
  framesLog->append(entry);
}

}  // namespace atdoa
}  // namespace algorithm
}  // namespace smile
