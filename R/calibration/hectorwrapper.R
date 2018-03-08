##==============================================================================
## hectorwrapper.R
##
## Ben Vega-Westhoff
## replace_run_hector() is modified function from
## Ben Bond-Lamberty's batchrunner.R
## 
## -Hector wrapper for calibration
## -Adjusts input parameters with a temporary .ini file
## -Runs Hector with temporary .ini file
## -Reads desired outputs from output csv and return
##
##==============================================================================

# Hard-coded file locations
# ==============================================================================
# You will likely have to change this. Either an absolute location or relative to working.dir.
EXECUTABLE = "../../../source/hector"
# ==============================================================================

# -----------------------------------------------------------------------------
# Run model with replacement param values, save output to new .csv files
# From Ben Bond-Lamberty's batchrunner.R
replace_run_hector <- function( infile, replacevars, newstr ) {

    results <- data.frame()
    f <- infile
    stopifnot( file.exists(f) )
    flines <- readLines(f)

    for( vn in seq_len( length(replacevars) ) ) {
        v <- names(replacevars)[vn]
        flines <- subparam( flines,
                            section=replacevars[[vn]]$section,
                            parameter=v,
                            newvalue=replacevars[[vn]]$value )
    }

    # At this point we've substituted in all new parameter values

    # Write out new ini file
    new_ini <- paste0( "temp", newstr, ".ini" )
    file.create( new_ini )
    cat( flines, file=new_ini, sep="\n" )

    # Run Hector
    cmd <- paste0( EXECUTABLE, " ", new_ini )
    error <- try( system( cmd, ignore.stdout = TRUE ) )
    if( error ) {
        print( "Hector error - autofilling rest of run with Inf (will have 0 likelihood in a calibration)." )
    }
    system( paste0( "rm ", new_ini ) )
}

# -----------------------------------------------------------------------------
# Create input file, run Hector, and return observations for calibration
#
# Input time series options:
#  Hector can currently only be constrained by total
#  forcing, not by a set of forcing contributions. Therefore, only total forcing, and
#  it should already include any scaling of aerosol forcing due to alpha.
#
# Input parameters:
#  par.names         - Vector of parameters to be changed in the input file
#  par.vals          - Vector of associated values
#  par.sections      - Vector of associated sections of the input file
#  chain.str         - Name suffix for Hector output files
#  working.dir       - Either absolute, or relative to this function
#  forcing.file      - Should already include any scaling of aerosol forcing. 
#		       If null, just use emissions specified in ini.template.
#  ini.template      - Template for the new .ini file we pass to Hector
#  output.vars       - Which output Hector variables do we want to return?
#  output.components - And their associated Hector components
#
# Output:
#  model.output      - List of the output.vars time series 
# -----------------------------------------------------------------------------
hectorwrapper = function( par.names,
			  par.vals,
			  par.sections,
                          chain.str = "",
			  working.dir = ".", #Absolute location or relative to this function
			  forcing.file = NULL, #Absolute location or relative to working.dir
			  ini.template = "../input_templates/default_nowrite.ini", #Absolute location or relative to working.dir
                          output.vars = c( "Tgav", "heatflux" ),
			  output.components = c( "temperature", "temperature" ), 
			  mod.time )
{
    # Write .ini file with updated input parameters
    nparam = length( par.names )
    vardata = vector( "list" )
    for ( i in 1:nparam ){
        vardata[[i]] = list( section = par.sections[i], value = par.vals[i] )
	names(vardata)[i] = par.names[i]
    }
    vardata[[nparam + 1]] = list( section = "core", value = paste0( "temp", chain.str ) )
    names(vardata)[nparam+1] = "run_name"
    if ( !is.null( forcing.file ) ) { 
        vardata[[nparam + 2]] = list( section = "forcing", value = paste0( "csv:", forcing.file ) )
        names(vardata)[nparam + 2] = "Ftot_constrain"
    }
    # Specify component outputs we want in the .ini
    output.sections = unique( output.components )
    noutput.sections = length( output.sections )
    varnames.output = rep( "output", noutput.sections )
    for (i in 1:noutput.sections ) {
        vardata[[length(vardata) + 1]] = list( section = output.sections[i], value = 1 )
	names(vardata)[length(vardata)] = varnames.output[i]
    }

    # Create working and output directories if needed    
    output_dir = paste0( working.dir, "/output" ) #Required for Hector output to be saved
    if( ( working.dir != ".") & (!file.exists( working.dir ) ) ) { system( paste0( "mkdir ", working.dir ) ) }
    if( !file.exists( output_dir ) ) { system( paste0( "mkdir ", output_dir ) ) }

    # Save runs to a log file 
    logfile = paste0( working.dir, "/", chain.str, "hector_runs_log.txt" )
    if( !file.exists( logfile ) ) {write( paste( par.names, collapse = " "), file=logfile ) }
    write( paste0( paste( par.vals, collapse = " " ), ' ', Sys.time() ), file=logfile, append=TRUE )

    # Move to working.dir and run Hector with new parameters
    original_wd = getwd()
    setwd( working.dir )
    replace_run_hector( ini.template, vardata, chain.str )
    setwd( original_wd )
    
    # Read in outputs from outputstream csv
    output = paste0( output_dir, "/outputstream_temp", chain.str, ".csv" )
    fout = read.csv( file = output, header = TRUE, skip=1 )
    # No spinup
    fout = fout[fout$spinup == 0, ]
    # Get rid of extra fields
    fout$run_name <- fout$spinup <- fout$units <- NULL
    # Put model output into our return list
    model.output = list( time = mod.time )
    for ( i in 1:length(output.vars) ) {
      varname = output.vars[i]
      time = fout$year[fout$variable == varname]
      val  = fout$value[fout$variable == varname]
      # Handle runs that end prematurely by filling with Inf
      model.output[[varname]] = c( val, rep( Inf, times = length( setdiff( mod.time, time ) ) ) )
    }

    system( paste0( "rm ", output ) )
    if( file.exists( paste0( output_dir, "/output.csv" ) ) ) {
      system( paste0( "rm ", output_dir, "/output.csv" ) ) #also created at run time
    }
    
    return( model.output )
}
