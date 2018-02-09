##==============================================================================
## forcing_total:
## Function to add up all the forcing for either an RCP scenario (projections)
## or data (hindcasts).
##
## Input:
##  forcing           input forcing data, broken down into components (to apply aerosol scaling)
##  alpha.doeclim     DOECLIM (Hector's temperature component) parameter, aerosol scaling factor
##  l.project         if TRUE, expect RCP scenario format
##                    if FALSE, expect hindcast data format
##  begyear           beginning year of model simulation
##  endyear           ending year of model simulation
##  flnd              area land fraction (0.29 by default)
##
## Output:
##  forcing.total     total forcing, accounting for aerosol scaling
##
## Original code for BRICK by Tony Wong
## Modified (slightly) for Hector by Ben Vega-Westhoff
##==============================================================================

forcing_total <- function(  forcing       ,
                            alpha.doeclim ,
                            l.project     ,
                            begyear       ,
                            endyear       ,
                            flnd = 0.29
                            )
{
  if(!l.project) {
    ## Hindcasts
    forcing.land = forcing$co2 + forcing$nonco2.land + alpha.doeclim*forcing$aerosol.land + forcing$solar.land + forcing$volc.land
    forcing.ocean = forcing$co2 + forcing$nonco2.ocean + alpha.doeclim*forcing$aerosol.ocean + forcing$solar.ocean + forcing$volc.ocean
    forcing.total = flnd*forcing.land + (1-flnd)*forcing.ocean
  } else {
    ## Projections
    forcing.total = forcing$co2 + forcing$nonco2 + alpha.doeclim*forcing$aerosol.direct + alpha.doeclim*forcing$aerosol.indirect +
		                forcing$solar + forcing$volcanic + forcing$other
  }

  ## Clip forcing at the beginning and end of the model simulation
  ibeg=which(forcing$year==begyear)
  iend=which(forcing$year==endyear)
  if(length(ibeg)==0 | length(iend)==0) print('ERROR - begyear/endyear not within forcing data')
  forcing.total = forcing.total[ibeg:iend]
  forcing.total[1] = 0 # Hector normalizes forcing to that in begyear. 
		       # We do this to counteract that when the input 
		       # forcings are already normalized to preindustrial 
  return(forcing.total)
}

##==============================================================================
## End
##==============================================================================
