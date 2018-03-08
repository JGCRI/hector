# -----------------------------------------------------------------------------
# subparam.R
# Useful function to replace values in a list of lines read in from Hector .ini.
# Returns the lines with updated parameter values.
# From Ben Bond-Lamberty's batchrunner.R.
# -----------------------------------------------------------------------------
# Substitute a new parameter value into an input file
subparam <- function( flines, section, parameter, newvalue ) {
    # Find 'section' and the next one after that
    secline <- grep( paste0( "^\\[", section, "\\]$" ), flines )
    stopifnot( length( secline ) > 0 )
    nextsecline <- secline + grep( "^\\[[a-zA-Z0-9_]*\\]$", flines[secline+1:length(flines)] )[1]
    if( length(nextsecline) == 0 ) nextsecline <- length(flines)

    # Find variable name occuring within the section
    varline <- secline - 1 + grep( paste0( "^", parameter ), flines[secline:nextsecline] )
    stopifnot( length(varline) == 1 )
 
   # Overwrite with new value
    flines[varline] <- paste0( parameter, "=", newvalue, " ; [MODIFIED]" )
    flines
}
