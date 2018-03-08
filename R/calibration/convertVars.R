##==============================================================================
## convertVars:
## Function to convert Hector outputs to match observational counterparts
## This should be called wherever we are comparing model output with obs
## (e.g. hector_DEoptim.R and hector_assimLikelihood.R)
##
## Input:
##  model.out	List of model output time series
##
## Output:
##  model.out	List with model output converted to match observations
##==============================================================================

# From BRICK
# Convert annual ocean heat flux (W/m^2) to cumulative ocean heat content anomaly (10^22 J)
flux.to.heat = function( heatflux )
{   
    flnd = 0.29                                  #fractional land area
    secs.per.year = 31556926.0
    earth.area = 5100656 * 10^8                  #m2
    ocean.area = ( 1-flnd ) * earth.area 
    powtoheat = ocean.area*secs.per.year / 10^22 #convert flux to total ocean heat
    
    ocean.heat = cumsum( heatflux ) * powtoheat  #10^22 J
    
    return( ocean.heat )
}

# Add additional conversions as needed and update convertVars accordingly


convertVars = function( model.out )
{
    for ( i in 1:length(model.out) ) {
    
        if ( names( model.out )[i] == "heatflux" ) {
	    names( model.out )[i] = "ocheat"
	    model.out[[i]] = flux.to.heat ( model.out[[i]] )
        }
    }
    return ( model.out )
}    
