#!/bin/bash
#
# Copyright (c) 2015 University of Washington
# Copyright (c) 2015 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation;
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# Authors: Tom Henderson <tomh@tomh.org> and Nicola Baldo <nbaldo@cttc.es>
#

control_c()
{
  echo "exiting"
  exit $?
}

trap control_c SIGINT

if test ! -f ../../../../waf ; then
    echo "please run this program from within the directory `dirname $0`, like this:"
    echo "cd `dirname $0`"
    echo "./`basename $0`"
    exit 1
fi

outputDir=`pwd`/results
if [ -d $outputDir ] && [ "$1" != "-a" ]; then
    echo "$outputDir directory exists; exiting!"
    echo ""
    echo "Pass the '-a' option if you want to append; else move results/ out of the way"
    echo ""
    exit 1
else
    mkdir -p "${outputDir}"
fi

if [ -d $outputDir ] && [ "$1" == "-a" ]; then
    echo "Appending results to existing results/ directory"
fi

set -x
set -o errexit

# need this as otherwise waf won't find the executables
cd ../../../../

# Random number generator seed
RngRun=1

minDistance=20
maxDistance=25

for distance in $(seq $minDistance $maxDistance); do
  ./waf --run "link-performance --distance=${distance}"
  mv link-performance-rssi.dat ${outputDir}/link-performance-rssi-${distance}.dat 
done
