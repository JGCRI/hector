# Make nice graphs of Hector stream output
# This script runs as a backend to the main Hector process

# -----------------------------------------------------------------------------
#				    			SETUP
# -----------------------------------------------------------------------------

# Support functions
source( "op_support.R" )    # miscellaneous support functions
source( "op_grapher.R" )		# function definition only
source( "op_parser.R" )			# first part of the program - functions to read data
source( "op_specials.R" )		# special graphs that can't be handled by op_graph

printlog( "Welcome to op.R, the outputstream plotter" )

# 'INFILES' lists one or more input files to read in
# This should be specified by whoever calls us

if( !exists( 'INFILES' ) )
  stop ("INFILES and optionally SCENARIOS need to be defined")

# 'SCENARIOS' can be NA, or multiple values (same number as INFILES)
if( !exists( 'SCENARIOS' ) ) {
  SCENARIOS <- NA
}
stopifnot( is.na( SCENARIOS ) | ( length( SCENARIOS ) == length( INFILES ) ) )

# 'PLOTFILETYPE' controls default graph type
if( !exists( 'PLOTFILETYPE' ) ) {
  PLOTFILETYPE <- "png"	# will affect all plots below
}


# -----------------------------------------------------------------------------
#				    			DATA
# -----------------------------------------------------------------------------

# 1. Read primary (Hector) data
printlog( SEPARATOR )
printlog( "Reading model output data..." )
d_sim <- read_output_data()

# 'COMPARISON_DATA' is a directory path
# This should be specified by whoever calls us

# 2. Read observed (comparison) data
if(exists("COMPARISON_DATA")) {
  printlog( SEPARATOR )
  if( !exists( "d_obs" ) ) {		# only do once per session
    d_obs <- read_compdata( names( d_sim ) )	
  } else {
    printlog( "Skipping read of comparison data" )
  }
} else {
  printlog( "No comparison data defined" )
  d_obs <- data.frame() 
}

# 3. Do tag matching, etc., and combine
d <- matchdata( d_sim, d_obs )

# To add or re-read a single data file:
#		add_compdata_file( "filename", d_obs, names( d_sim ) )
#		d <- matchdata( d_sim, d_obs )

# -----------------------------------------------------------------------------
#				    AND WE'RE DONE - CALLER DOES PLOTTING
# -----------------------------------------------------------------------------

printlog( "All done with op_R." )

