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

#pragma once

#include <IdealApplication.h>
#include <Logger.h>
#include <utilities.h>

namespace smile {
namespace algorithm {
namespace atdoa {

class MobileApplication : public smile::IdealApplication
{
 public:
  MobileApplication() = default;
  MobileApplication(const MobileApplication& source) = delete;
  MobileApplication(MobileApplication&& source) = delete;
  ~MobileApplication();

  MobileApplication& operator=(const MobileApplication& source) = delete;
  MobileApplication& operator=(MobileApplication&& source) = delete;

 private:
  void initialize(int stage) override;

  void handleSelfMessage(cMessage* newMessage) override;

  void handleTxCompletionSignal(const smile::IdealTxCompletion& completion) override;

  void sendFrame();

  smile::Logger* framesLog{nullptr};
  SimTime localizationTxInterval;
  SimTime broadcastTxInterval;
  cMessage* localizationTxTimerMessage{nullptr};
  cMessage* broadcastTxTimerMessage{nullptr};
  SequenceNumberGenerator<unsigned int> sequenceNumberGenerator;
};

}  // namespace atdoa
}  // namespace algorithm
}  // namespace smile
