#!/usr/bin/env python
#
# Nagios Monitor for the Spark Core
# Copyright (C) 2014 Nicolas Cortot
# https://github.com/ncortot/spark-nagios-monitor
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
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

import os
import sys

from make_sample import BUFFER_SIZE


RESOURCES_START = 0x00080000
RESOURCES_END = 0x000200000


def write_header(header, resources):
    lines = [
        '#ifndef RESOURCES_H_',
        '#define RESOURCES_H_',
        '',
        '#define RESOURCE_BUFFER_SIZE {0}'.format(BUFFER_SIZE),
        '',
    ]

    start = RESOURCES_START

    for path in resources:
        name = path[:-4].upper()
        size = os.path.getsize(path)
        end = start + size

        lines += [
            '#define RESOURCE_{0}_START 0x{1:08X}'.format(name, start),
            '#define RESOURCE_{0}_END 0x{1:08X}'.format(name, end),
            '',
        ]

        start = end

    if end > RESOURCES_END:
        sys.stderr.write(
            'Resources overflow, 0x{0:08X} > 0x{1:08X}\n'
            .format(end, RESOURCES_END))
        sys.exit(-2)

    lines += [
        '#endif  // RESOURCES_H_',
        ''
    ]

    fp = open(header, 'w')
    fp.write('\n'.join(lines))
    fp.close()


if __name__ == '__main__':
    if len(sys.argv) < 3:
        sys.stderr.write(
            'Usage: {0} resources.h file1.bin file2.bin...\n'
            .format(sys.argv[0]))
        sys.exit(-1)
    else:
        write_header(sys.argv[1], sys.argv[2:])
