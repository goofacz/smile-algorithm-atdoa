#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see http:#www.gnu.org/licenses/.
#

import importlib
import os.path

import numpy as np
import scipy.constants as scc

import smile.algorithms.common as scommon
import smile.area as sarea
import smile.simulation as ssimulation
from atdoa.mobiles import Mobiles
from smile.filter import Filter
from smile.frames import Frames
from smile.nodes import Nodes
from smile.results import Results


class Simulation(ssimulation.Simulation):
    def __init__(self, configuration):
        self._configuration = configuration

        assert (scc.unit('speed of light in vacuum') == 'm s^-1')
        self._c = scc.value('speed of light in vacuum')
        self._c = self._c * 1e-12  # m/s -> m/ps

        solver_configuration = self._configuration['algorithms']['tdoa']['solver']
        solver_module = importlib.import_module(solver_configuration['module'])
        self._Solver = getattr(solver_module, solver_configuration['class'])

        area_configuration = self._configuration['area']
        area_file_path = area_configuration['file']
        self._area = sarea.Area(area_file_path)

    def run_offline(self, directory_path):
        anchors = Nodes.load_csv(os.path.join(directory_path, 'atdoa_anchors.csv'))
        mobiles = Mobiles.load_csv(os.path.join(directory_path, 'atdoa_mobiles.csv'))
        anchor_frames = Frames.load_csv(os.path.join(directory_path, 'atdoa_anchor_beacons.csv'))
        mobile_frames = Frames.load_csv(os.path.join(directory_path, 'atdoa_mobile_beacons.csv'))

        results = None
        for mobile_node in mobiles:
            mobile_results = self._localize_mobile(mobile_node, anchors, mobile_frames, anchor_frames)
            if results is None:
                results = mobile_results
            else:
                results = Results(np.concatenate((results, mobile_results), axis=0))

        return results

    def _localize_mobile(self, mobile_node, anchors, all_mobile_frames, all_anchor_frames):
        broadcast_interval = mobile_node["broadcast_interval"]

        frame_filter = Filter()
        frame_filter.equal("node_mac_address", mobile_node["mac_address"])
        mobile_frames = frame_filter.execute(all_mobile_frames)

        min_sequence_number = np.min(mobile_frames["sequence_number"])
        max_sequence_number = np.max(mobile_frames["sequence_number"])

        for first_sequence_number in np.arange(min_sequence_number, max_sequence_number - 1, 2):
            second_sequence_number = first_sequence_number + 1
            final_positions = []

            frame_filter = Filter()
            frame_filter.equal("sequence_number", first_sequence_number)
            frame_filter.equal("source_mac_address", mobile_node["mac_address"])
            first_round_frames = frame_filter.execute(all_anchor_frames)

            frame_filter = Filter()
            frame_filter.equal("sequence_number", second_sequence_number)
            frame_filter.equal("source_mac_address", mobile_node["mac_address"])
            second_round_frames = frame_filter.execute(all_anchor_frames)

            if min(first_round_frames.shape[0], second_round_frames.shape[0]) < 3:
                continue

            final_anchors = []
            final_time_differences = []

            for anchor in anchors:
                frame_filter = Filter()
                frame_filter.equal("node_mac_address", anchor["mac_address"])
                first_round_frame = frame_filter.execute(first_round_frames)
                if first_round_frame.size == 0:
                    continue

                frame_filter = Filter()
                frame_filter.equal("node_mac_address", anchor["mac_address"])
                second_round_frame = frame_filter.execute(second_round_frames)
                if second_round_frame.size == 0:
                    continue

                time_difference = second_round_frame["end_clock_timestamp"] - first_round_frame["end_clock_timestamp"]
                #time_difference -= broadcast_interval

                final_anchors.append(anchor)
                final_time_differences.append(time_difference[0])

            final_anchors = Nodes(final_anchors)
            final_time_differences = np.array(final_time_differences)

            coordinates = final_anchors[1:4, "position_2d"]
            distances = final_time_differences[1:4] * self._c
            sorted_anchors_coordinates, sorted_distances = scommon.sort_measurements(coordinates, distances)

            # Compute position
            try:
                solver = self._Solver(sorted_anchors_coordinates, sorted_distances)
                positions = solver.localize()

                # Choose better position
                positions = [position for position in positions if self._area.contains(position)]
                if positions:
                    final_positions += positions
                else:
                    final_positions.append((np.nan, np.nan))
            except ValueError:
                final_positions.append((np.nan, np.nan))



        return None
