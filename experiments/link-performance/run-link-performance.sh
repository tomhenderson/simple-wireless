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

# This program runs the link-performance example for different configurations
# and produces a plot of PER vs. the parameter under test.  The current
# version steps through the distance between two devices, but other parameters
# can be substituted for distance.

# Results will be stored in a timestamped 'results' directory

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

set -x
set -o errexit

dirname=link-performance
resultsDir=`pwd`/results/$dirname-`date +%Y%m%d-%H%M%S`
experimentDir=`pwd`
mkdir -p ${resultsDir}

# need this as otherwise waf won't find the executables
cd ../../../../

# Random number generator seed
RngRun=1

plotName="link-performance-summary.pdf"
minDistance=25
maxDistance=115
stepSize=5
maxPackets=1000
noisePower=-75

./waf build
for distance in `seq $minDistance $stepSize $maxDistance`; do
  ./waf --run "link-performance --maxPackets=${maxPackets} --noisePower=${noisePower} --distance=${distance} --metadata=${distance}"
done

mv link-performance-summary.dat ${experimentDir} 
rm -rf link-performance-rssi.dat

cd ${experimentDir}

if [[ ! -f ../utils/plot-lines-with-error-bars.py ]]; then
  echo 'plot file not found, exiting...'
  exit
fi

# Specify where the columns of data are to plot.  Here, the xcolumn data
# (distance) is in column 5, the y column data (PER) in column 3, and the
# length of the error bar is in column 4 
/usr/bin/python ../utils/plot-lines-with-error-bars.py --title='PER vs. distance' --xlabel='distance (m)' --ylabel='Packet Error Ratio (PER)' --xcol=5 --ycol=3 --yerror=4 --fileName=link-performance-summary.dat --plotName=${plotName}

# Copy files to the results directory
mv $plotName ${resultsDir} 
mv link-performance-summary.dat ${resultsDir} 
cp $0 ${resultsDir}
cp ../utils/plot-lines-with-error-bars.py ${resultsDir}
