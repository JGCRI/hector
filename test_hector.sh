#!/usr/bin/env bash

# Run Hector through a bunch of tests
# Intended for GitHub Actions, but can be used locally too
# BBL January 2022

# exit when any command fails
set -e

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

# Run all INI files (the basic SSPs)
echo "---------- Running: basic SSPs ----------"
find ./inst/input -name "*.ini" -exec $HECTOR {} +

# Make sure the model handles year changes
echo "---------- Running: year changes ----------"
sed 's/startDate=1745/startDate=1740/' $INPUT/hector_ssp245.ini > $INPUT/hector_ssp245_time.ini
$HECTOR $INPUT/hector_ssp245_time.ini
sed 's/endDate=2300/endDate=2250/' $INPUT/hector_ssp245.ini > $INPUT/hector_ssp245_time.ini
$HECTOR $INPUT/hector_ssp245_time.ini
rm $INPUT/hector_ssp245_time.ini

# Turn on tracking
echo "---------- Running: tracking ----------"
sed 's/trackingDate=9999/trackingDate=1850/' $INPUT/hector_ssp245.ini > $INPUT/hector_ssp245_tracking.ini
$HECTOR $INPUT/hector_ssp245_tracking.ini

# Note that in the runs below, we're running the model with various constraints
# turned on, but not actually verifying that the constraint actually *works*

echo "---------- Running: tracking & CO2 constraint ----------"
sed 's/;CO2_constrain=csv:tables\/ssp245_emiss-constraints_rf.csv/CO2_constrain=csv:tables\/ssp245_emiss-constraints_rf.csv/' $INPUT/hector_ssp245_tracking.ini > $INPUT/hector_ssp245_tracking_co2.ini
$HECTOR $INPUT/hector_ssp245_tracking_co2.ini
rm $INPUT/hector_ssp245_tracking_co2.ini
rm $INPUT/hector_ssp245_tracking.ini

# Turn off spinup
echo "---------- Running: no spinup ----------"
sed 's/do_spinup=1/do_spinup=0/' $INPUT/hector_ssp245.ini > $INPUT/hector_ssp245_spinup.ini
$HECTOR $INPUT/hector_ssp245_spinup.ini
rm $INPUT/hector_ssp245_spinup.ini

# Turn on the constraint settings one by one and run the model  CO2
echo "---------- Running: CO2 constraint ----------"
sed 's/;CO2_constrain=csv:tables\/ssp245_emiss-constraints_rf.csv/CO2_constrain=csv:tables\/ssp245_emiss-constraints_rf.csv/' $INPUT/hector_ssp245.ini > $INPUT/hector_ssp245_co2.ini
$HECTOR $INPUT/hector_ssp245_co2.ini
rm $INPUT/hector_ssp245_co2.ini

# Temperature
echo "---------- Running: Tgav constraint ----------"
sed 's/;tgav_constrain=csv:tables\/CONSTRAINT.csv/tgav_constrain=csv:tables\/tgav_historical.csv/' $INPUT/hector_ssp245.ini > $INPUT/hector_ssp245_tgav.ini
$HECTOR $INPUT/hector_ssp245_tgav.ini
rm $INPUT/hector_ssp245_tgav.ini

# Radiative forcing
echo "---------- Running: Ftot constraint ----------"
sed 's/;Ftot_constrain=csv:tables\/CONSTRAINT.csv/Ftot_constrain=csv:tables\/ssp245_emiss-constraints_rf.csv/' $INPUT/hector_ssp245.ini > $INPUT/hector_ssp245_ftot.ini
$HECTOR $INPUT/hector_ssp245_ftot.ini
rm $INPUT/hector_ssp245_ftot.ini

# Ocean-atmosphere C flux
# Disabled as the relevant file doesn't seem to currently exist
#sed 's/;atm_ocean_constrain/atm_ocean_constrain/' input/hector_rcp45.ini > input/hector_rcp45_ocean.ini
#$HECTOR input/hector_rcp45_ocean.ini
#rm input/hector_rcp45_ocean.ini

echo "All done."
