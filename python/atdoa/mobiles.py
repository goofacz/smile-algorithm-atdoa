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

import numpy as np

from smile.nodes import Nodes


class Mobiles(Nodes):
    def __init__(self, *args):
        super(Mobiles, self).__init__()
        self.column_names["localization_interval"] = 4
        self.column_names["broadcast_interval"] = 5

    @staticmethod
    def load_csv(file_path):
        return Mobiles(np.loadtxt(file_path, delimiter=',', ndmin=2))
