#!/usr/bin/env bash

# Run Hector through a bunch of tests
# Intended for Travis-CI, but can be used locally too
# BBL May 2020

if [ $# -eq 0 ]
  then
    echo "One argument (path to executable) required"
    exit 1
fi

HECTOR=$1
INPUT=./inst/input

if [ ! -f $HECTOR ]; then
    echo "Hector executable not found!"
    exit 1
fi
if [ ! -d $INPUT ]; then
    echo "Input directory not found!"
    exit 1
fi

# Run the basic RCPs
$HECTOR $INPUT/hector_rcp26.ini
$HECTOR $INPUT/hector_rcp45.ini
$HECTOR $INPUT/hector_rcp60.ini
$HECTOR $INPUT/hector_rcp85.ini

# Make sure the model handles year changes
sed 's/startDate=1745/startDate=1740/' $INPUT/hector_rcp45.ini > $INPUT/hector_rcp45_time.ini
$HECTOR $INPUT/hector_rcp45_time.ini
sed 's/endDate=2300/endDate=2250/' $INPUT/hector_rcp45.ini > $INPUT/hector_rcp45_time.ini
$HECTOR $INPUT/hector_rcp45_time.ini
rm $INPUT/hector_rcp45_time.ini

# Turn off spinup
sed 's/do_spinup=1/do_spinup=0/' $INPUT/hector_rcp45.ini > $INPUT/hector_rcp45_spinup.ini
$HECTOR $INPUT/hector_rcp45_spinup.ini
rm $INPUT/hector_rcp45_spinup.ini

# Turn on the constraint settings one by one and run the model
# CO2
sed 's/;CO2_constrain=csv:constraints\/lawdome_co2.csv/CO2_constrain=csv:constraints\/lawdome_co2.csv/' $INPUT/hector_rcp45.ini > $INPUT/hector_rcp45_co2.ini
$HECTOR $INPUT/hector_rcp45_co2.ini
rm $INPUT/hector_rcp45_co2.ini

# Temperature
sed 's/;tgav_constrain/tgav_constrain/' $INPUT/hector_rcp45.ini > $INPUT/hector_rcp45_tgav.ini
$HECTOR $INPUT/hector_rcp45_tgav.ini
rm $INPUT/hector_rcp45_tgav.ini

# Radiative forcing
sed 's/;Ftot_constrain/Ftot_constrain/' $INPUT/hector_rcp45.ini > $INPUT/hector_rcp45_ftot.ini
$HECTOR $INPUT/hector_rcp45_ftot.ini
rm $INPUT/hector_rcp45_ftot.ini

# Ocean-atmosphere C flux
# Disabled as the relevant file doesn't seem to currently exist
#sed 's/;atm_ocean_constrain/atm_ocean_constrain/' input/hector_rcp45.ini > input/hector_rcp45_ocean.ini
#$HECTOR input/hector_rcp45_ocean.ini
#rm input/hector_rcp45_ocean.ini

echo "All done."
