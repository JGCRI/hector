##==============================================================================
## Function to compute the indices within a vector of observational data and a
## vector of model output, that are of the same time points, for comparison.
##
## Input:
##  obs.time      times of observations
##  mod.time      times of model output
##
## Output:
##  oidx          indices within observational data vector that are same as
##  midx          these indices within the model output vector
##
## Original code is from Kelsey Ruckert (klr324@psu.edu).
## Modified for BRICK by Tony Wong (twong@psu.edu).
## Now also used for Hector calibration
##==============================================================================

compute_indices <- function(
                              obs.time,
                              mod.time
                              ){

  oidx = NULL; midx = NULL
  endyear = mod.time[(length(mod.time))]
  if(min(obs.time) <= endyear) {
    oidx = which(obs.time <= endyear)
    midx = obs.time[oidx] - mod.time[1] + 1
    oidx = oidx[which(midx>0)] # only positive indices are within the simulation
    midx = midx[which(midx>0)]
  }
  return(list(oidx=oidx,midx=midx))
}

##==============================================================================
## End
##==============================================================================
