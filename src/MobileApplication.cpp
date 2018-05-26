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
#include <inet/common/ModuleAccess.h>
#include <utilities.h>
#include "BeaconFrame_m.h"
#include "CsvLoggerExtensions.h"

namespace smile {
namespace algorithm {
namespace atdoa {

Define_Module(MobileApplication);

MobileApplication::~MobileApplication()
{
  cancelAndDelete(localizationTxTimerMessage);
  cancelAndDelete(broadcastTxTimerMessage);
}

void MobileApplication::initialize(int stage)
{
  IdealApplication::initialize(stage);

  if (stage == inet::INITSTAGE_LOCAL) {
    const auto& broadcastTxIntervalParameter = par("broadcastTxInterval");
    broadcastTxInterval = SimTime(broadcastTxIntervalParameter.intValue(), SIMTIME_MS);

    const auto& localizationTxIntervalParameter = par("localizationTxInterval");
    localizationTxInterval = SimTime(localizationTxIntervalParameter.intValue(), SIMTIME_MS);
  }

  if (stage == inet::INITSTAGE_APPLICATION_LAYER) {
    auto* mobilesLog = inet::getModuleFromPar<smile::Logger>(par("mobilesLoggerModule"), this, true);
    const auto entry =
        csv_logger::compose(getMacAddress(), getCurrentTruePosition(), localizationTxInterval, broadcastTxInterval);
    mobilesLog->append(entry);

    framesLog = inet::getModuleFromPar<smile::Logger>(par("mobileFramesLoggerModule"), this, true);
    localizationTxTimerMessage = new cMessage{"localizationTxTimerMessage"};
    broadcastTxTimerMessage = new cMessage{"broadcastTxTimerMessage"};

    scheduleAt(clockTime(), localizationTxTimerMessage);
  }
}

void MobileApplication::handleSelfMessage(cMessage* newMessage)
{
  if (newMessage == localizationTxTimerMessage) {
    sendFrame();
    scheduleAt(clockTime() + broadcastTxInterval, broadcastTxTimerMessage);
  }
  else if (newMessage == broadcastTxTimerMessage) {
    sendFrame();
    scheduleAt(clockTime() + localizationTxInterval, localizationTxTimerMessage);
  }
  else {
    throw cRuntimeError{"MobileApplication received unknown message: \"%s\"", newMessage->getFullName()};
  }
}

void MobileApplication::handleTxCompletionSignal(const smile::IdealTxCompletion& completion)
{
  const auto frame = omnetpp::check_and_cast<const BeaconFrame*>(completion.getFrame());
  const auto entry = csv_logger::compose(getMacAddress(), completion, *frame);
  framesLog->append(entry);
}

void MobileApplication::sendFrame()
{
  auto frame = createFrame<BeaconFrame>(inet::MACAddress::BROADCAST_ADDRESS);
  frame->setSequenceNumber(sequenceNumberGenerator.generateNext());
  frame->setBitLength(10);
  sendDelayed(frame.release(), 0, "out");
}

}  // namespace atdoa
}  // namespace algorithm
}  // namespace smile
