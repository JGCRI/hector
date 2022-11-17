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
find ./inst/input -name "*.ini" -print0 | xargs -0 -n1 $HECTOR

# Make sure the model handles year changes
echo "---------- Running: year changes ----------"
sed 's/startDate=1745/startDate=1746/' $INPUT/hector_ssp245.ini > $INPUT/hector_ssp245_time.ini
# Confirm that a change was made
if [[ $(diff -q $INPUT/hector_ssp245.ini $INPUT/hector_ssp245_time.ini | wc -c) -eq 0 ]]; then exit 1; fi
# ...and run
$HECTOR $INPUT/hector_ssp245_time.ini

sed 's/endDate=2300/endDate=2250/' $INPUT/hector_ssp245.ini > $INPUT/hector_ssp245_time.ini
if [[ $(diff -q $INPUT/hector_ssp245.ini $INPUT/hector_ssp245_time.ini | wc -c) -eq 0 ]]; then exit 1; fi
$HECTOR $INPUT/hector_ssp245_time.ini
rm $INPUT/hector_ssp245_time.ini

# Turn on tracking
echo "---------- Running: tracking ----------"
sed 's/trackingDate=9999/trackingDate=1850/' $INPUT/hector_ssp245.ini > $INPUT/hector_ssp245_tracking.ini
if [[ $(diff -q $INPUT/hector_ssp245.ini $INPUT/hector_ssp245_tracking.ini | wc -c) -eq 0 ]]; then exit 1; fi
$HECTOR $INPUT/hector_ssp245_tracking.ini

# Note that in the runs below, we're running the model with various constraints
# turned on, and verifying that it doesn't crash or anything like that.
# We don't check that the constraint actually *works*, i.e. that the (for example)
# Tgav outputs match the constraint input.

echo "---------- Running: tracking & CO2 constraint ----------"
sed 's/;[[:space:]]*CO2_constrain=csv:tables\/ssp245_emiss-constraints_rf.csv/CO2_constrain=csv:tables\/ssp245_emiss-constraints_rf.csv/' $INPUT/hector_ssp245_tracking.ini > $INPUT/hector_ssp245_tracking_co2.ini
if [[ $(diff -q $INPUT/hector_ssp245_tracking.ini $INPUT/hector_ssp245_tracking_co2.ini | wc -c) -eq 0 ]]; then exit 1; fi
$HECTOR $INPUT/hector_ssp245_tracking_co2.ini
rm $INPUT/hector_ssp245_tracking_co2.ini
rm $INPUT/hector_ssp245_tracking.ini

# Turn off spinup
echo "---------- Running: no spinup ----------"
sed 's/do_spinup=1/do_spinup=0/' $INPUT/hector_ssp245.ini > $INPUT/hector_ssp245_spinup.ini
if [[ $(diff -q $INPUT/hector_ssp245.ini $INPUT/hector_ssp245_spinup.ini | wc -c) -eq 0 ]]; then exit 1; fi
$HECTOR $INPUT/hector_ssp245_spinup.ini
rm $INPUT/hector_ssp245_spinup.ini

# Turn on the constraint settings one by one and run the model
# CO2
echo "---------- Running: CO2 constraint ----------"
sed 's/;[[:space:]]*CO2_constrain=csv:tables\/ssp245_emiss-constraints_rf.csv/CO2_constrain=csv:tables\/ssp245_emiss-constraints_rf.csv/' $INPUT/hector_ssp245.ini > $INPUT/hector_ssp245_co2.ini
if [[ $(diff -q $INPUT/hector_ssp245.ini $INPUT/hector_ssp245_co2.ini | wc -c) -eq 0 ]]; then exit 1; fi
$HECTOR $INPUT/hector_ssp245_co2.ini
rm $INPUT/hector_ssp245_co2.ini

# Temperature
echo "---------- Running: tas constraint ----------"
sed 's/;[[:space:]]*tas_constrain=csv:tables\/ssp245_emiss-constraints_rf.csv/tas_constrain=csv:tables\/tas_historical.csv/' $INPUT/hector_ssp245.ini > $INPUT/hector_ssp245_tas.ini
if [[ $(diff -q $INPUT/hector_ssp245.ini $INPUT/hector_ssp245_tas.ini | wc -c) -eq 0 ]]; then exit 1; fi
$HECTOR $INPUT/hector_ssp245_tas.ini
rm $INPUT/hector_ssp245_tas.ini

# Radiative forcing
echo "---------- Running: RF tot constraint ----------"
sed 's/;[[:space:]]*RF_tot_constrain=csv:tables\/ssp245_emiss-constraints_rf.csv/RF_tot_constrain=csv:tables\/ar6_historical_constraint.csv/' $INPUT/hector_ssp245.ini > $INPUT/hector_ssp245_ftot.ini
if [[ $(diff -q $INPUT/hector_ssp245.ini $INPUT/hector_ssp245_ftot.ini | wc -c) -eq 0 ]]; then exit 1; fi
$HECTOR $INPUT/hector_ssp245_ftot.ini
rm $INPUT/hector_ssp245_ftot.ini

# Net biome production
echo "---------- Running: NBP constraint ----------"
sed 's/;[[:space:]]*NBP_constrain=csv:tables\/ssp245_emiss-constraints_rf.csv/NBP_constrain=csv:tables\/nbp_output.csv/' $INPUT/hector_ssp245.ini > $INPUT/hector_ssp245_nbp.ini
if [[ $(diff -q $INPUT/hector_ssp245.ini $INPUT/hector_ssp245_nbp.ini | wc -c) -eq 0 ]]; then exit 1; fi
$HECTOR $INPUT/hector_ssp245_nbp.ini
rm $INPUT/hector_ssp245_nbp.ini

echo "All done."
