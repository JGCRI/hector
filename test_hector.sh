#!/usr/bin/env bash

# Run Hector through a bunch of tests
# Intended for Travis-CI, but can be used locally too
# BBL January 17, 2017

if [ $# -eq 0 ]
  then
    echo "One argument (path to executable) required"
    exit 1
fi

HECTOR=$1


# Run the basic RCPs
$HECTOR input/hector_rcp26.ini
$HECTOR input/hector_rcp45.ini
$HECTOR input/hector_rcp60.ini
$HECTOR input/hector_rcp85.ini

# Make sure the model handles year changes
sed 's/startDate=1745/startDate=1740/' input/hector_rcp45.ini > input/hector_rcp45_time.ini
$HECTOR input/hector_rcp45_time.ini
sed 's/endDate=2300/endDate=2250/' input/hector_rcp45.ini > input/hector_rcp45_time.ini
$HECTOR input/hector_rcp45_time.ini
rm input/hector_rcp45_time.ini

# Turn off spinup
sed 's/do_spinup=1/do_spinup=0/' input/hector_rcp45.ini > input/hector_rcp45_spinup.ini
$HECTOR input/hector_rcp45_spinup.ini
rm input/hector_rcp45_spinup.ini

# Turn on the constraint settings one by one and run the model
# CO2
sed 's/;Ca_constrain=csv:constraints\/lawdome_co2.csv/Ca_constrain=csv:constraints\/lawdome_co2.csv/' input/hector_rcp45.ini > input/hector_rcp45_co2.ini
$HECTOR input/hector_rcp45_co2.ini
rm input/hector_rcp45_co2.ini

# Temperature
sed 's/;tgav_constrain/tgav_constrain/' input/hector_rcp45.ini > input/hector_rcp45_tgav.ini
$HECTOR input/hector_rcp45_tgav.ini
rm input/hector_rcp45_tgav.ini

# Radiative forcing
sed 's/;Ftot_constrain/Ftot_constrain/' input/hector_rcp45.ini > input/hector_rcp45_ftot.ini
$HECTOR input/hector_rcp45_ftot.ini
rm input/hector_rcp45_ftot.ini

# Ocean-atmosphere C flux
# Disabled as the relevant file doesn't seem to currently exist
#sed 's/;atm_ocean_constrain/atm_ocean_constrain/' input/hector_rcp45.ini > input/hector_rcp45_ocean.ini
#$HECTOR input/hector_rcp45_ocean.ini
#rm input/hector_rcp45_ocean.ini

echo "All done."
