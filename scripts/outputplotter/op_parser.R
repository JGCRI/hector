# File parser and tag processing routines for the Hector backend

INFILESOURCE 		<- "HECTOR"
INFILETYPE 			<- "HECTOR"
INFILEMODEL			<- "HECTOR"

COMPARISON_TYPE 	<- "observed"

COMPONENT_FIELD		<- "component"
ERROR_FIELD			<- "error"
ERRORMAX_FIELD		<- "error_max"
ERRORMIN_FIELD		<- "error_min"
MODEL_FIELD			<- "model"
MULTIVALUES_PREFIX	<- "value_"
PRETTY_FIELD		<- "prettylabel"
RUNNAME_FIELD		<- "run_name"
SCENARIO_FIELD		<- "scenario"
SOURCE_FIELD		<- "source"
TYPE_FIELD			<- "type"
VALUE_FIELD			<- "value"
VARIABLE_FIELD		<- "variable"
YEAR_FIELD			<- "year"
UNITS_FIELD     <- "units"

CMAPFILE 			<- "mappings/component_map.csv"
VMAPFILE 			<- "mappings/variable_map.csv"
CTAG_FIELD			<- "ctag"
OLD_CTAG_FIELD		<- "old_ctags"
VTAG_FIELD			<- "vtag"
OLD_VTAG_FIELD		<- "old_vtags"
TAG_SEPARATOR		<- "."
OLD_TAG_SEPARATOR	<- ","

REQUIRED_FIELDS 	<- c( YEAR_FIELD, CTAG_FIELD, VTAG_FIELD, VALUE_FIELD, SCENARIO_FIELD )


# Comparison data are grabbed automatically if placed into COMPARISON_DATA.
# For details on their structure, see readme file in that directory.

# TODO: y axis label isn't parsed, so looks goofy
# TODO: prettylabel
# TODO: move these TODOs to GitHub issues

library(readr)        # 0.1.0, for speed
library( reshape2 )
library( stringr )	  # currently stringr 1.6
library( dplyr )			# currently dplyr 0.4.2

# -----------------------------------------------------------------------------
# Read the Hector model output.
read_output_data <- function() {
  
  if( any( !is.na( SCENARIOS )) & length( INFILES ) != length( SCENARIOS ) )
    stop( "INFILES and SCENARIOS must be of equal length" )
  d_sim <- data.frame()
  for( i in seq_along( INFILES ) ) {
    f <- INFILES[ i ]
    printlog( f )
    if( !file.exists( f ) )
      stop( "File does not seem to exist!" )
    d <- as.data.frame(readr::read_csv( f, skip=1 ))
    names( d ) <- tolower( names( d ) )
    printlog( "-- fields:", names( d ) )
    printlog( "-- rows:", nrow( d ) )
    printlog( "-- scenario:", SCENARIOS[ i ] )
    
    if( nrow( d )==0 ) {
      printlog_warn( "-- no data read!" )
      next
    }
    d[ SOURCE_FIELD ] <- f
    d[ SCENARIO_FIELD ] <- SCENARIOS[ i ]
    d_sim <- rbind( d_sim, d )
  }
  d_sim[ MODEL_FIELD ] <- INFILEMODEL
  d_sim[ COMPONENT_FIELD ] <- tolower( d_sim[ , COMPONENT_FIELD ] )	
  d_sim[ VARIABLE_FIELD ] <- tolower( d_sim[ , VARIABLE_FIELD ] )	
  d_sim[ TYPE_FIELD ] <- INFILETYPE
  d_sim[ CTAG_FIELD ] <- NA
  d_sim[ VTAG_FIELD ] <- NA
  d_sim[ ERROR_FIELD ] <- NA
  d_sim[ ERRORMIN_FIELD ] <- NA
  d_sim[ ERRORMAX_FIELD ] <- NA
  
  printlog( "Total rows:", nrow( d_sim ) )
  
  d_sim
}

# -----------------------------------------------------------------------------
# Read a single CSV comparison data file
read_compdata_file <- function( f, fieldnames ) {
  stopifnot( is.character( f ) )
  stopifnot( is.vector( fieldnames ) )
  
  printlog( f, COMPARISON_DATA )
  if( !file.exists( file.path( COMPARISON_DATA, f ) ) ) {
    printlog_warn( "file not found" )
    return( NULL )
  }
  d <- read.csv( file.path( COMPARISON_DATA, f ), comment.char="#", na.strings="NA", stringsAsFactors = FALSE )
  names( d ) <- tolower( names( d ) )
  if( CTAG_FIELD %in% names( d ) )
    d[ , CTAG_FIELD ] <- tolower( d[ , CTAG_FIELD ] )
  if( VTAG_FIELD %in% names( d ) )
    d[ , VTAG_FIELD ] <- tolower( d[ , VTAG_FIELD ] )
  if( !SOURCE_FIELD %in% names( d ) )	 # if source not specified, get from filename
    d[ SOURCE_FIELD ] <- strsplit( basename( f ), ".", fixed = TRUE )[[ 1 ]][ 1 ]	
  if( !TYPE_FIELD %in% names( d ) )	 # if type not specified, assume "observed"
    d[ TYPE_FIELD ] <- COMPARISON_TYPE	
  d[ , TYPE_FIELD ] <- tolower( d[ , TYPE_FIELD ] )
  if( UNITS_FIELD %in% names( d ) ) {
    d[ , UNITS_FIELD ] <- as.character( d[ , UNITS_FIELD ] )  # force even numeric units (e.g. pH) to be character    
  }
  
  # If the data have fields value_xxx, value_yyy, etc., then there are multiple
  # variables per row (we have to assume same type and units)
  multivarnames <- grep( MULTIVALUES_PREFIX, names( d ) )
  if( any( multivarnames ) ) {
    printlog( "-- melting multiple variables in columns", multivarnames )
    if( VTAG_FIELD %in% names( d ) ) {
      printlog_warn( "   found pre-existing", VTAG_FIELD, " field; skipping file" )
      return( NULL )
    }
    dmelt <- melt( d, measure.vars=multivarnames, variable.name=VTAG_FIELD )
    dmelt[ VTAG_FIELD ] <- str_split_fixed( dmelt[ , VTAG_FIELD ], MULTIVALUES_PREFIX, 2 )[ , 2 ]
    d <- dmelt
  }
  
  # There's a minimum (required) set of fields that have to be present
  missings <- !( REQUIRED_FIELDS %in% names( d ) )
  if( any( missings ) ) {
    printlog_warn( "-- required field(s) not found:", REQUIRED_FIELDS[ missings ] )
    return( NULL )	
  }
  
  printlog( "--", nrow( d ), "observations" )
  printlog( "-- ctags:", unique( d[ , CTAG_FIELD ] ) )
  printlog( "-- vtags:", unique( d[ , VTAG_FIELD ] ) )
  
  d
}

# -----------------------------------------------------------------------------
# Add a single CSV comparison data file to existing d_obs data
# TODO: these functions are not very efficient
# TODO: should perhaps replace data, not dumbly add
add_compdata_file <- function( f, d_obs, fieldnames ) {
  stopifnot( is.character( f ) )
  stopifnot( is.data.frame( d_obs ) )
  stopifnot( is.vector( fieldnames ) )
  
  d <- read_compdata_file( f, fieldnames )	
  if( !is.null( d ) ) d_obs <- dplyr::rbind_list( d_obs, d )
  d_obs 
}

# -----------------------------------------------------------------------------
# Read CSVs in the COMPARISON_DATA directory (and subdirectories)
read_compdata <- function( fieldnames ) {
  stopifnot( is.vector( fieldnames ) )
  
  d_obs <- data.frame()
  for( f in list.files( COMPARISON_DATA, pattern="*.csv$", recursive = TRUE ) ) {
    d_obs <- add_compdata_file( f, d_obs, fieldnames )	# NOT efficient
  } # for
  
  printlog( "All done with reading files" )
  if( COMPONENT_FIELD %in% names( d_obs ) ) {
    printlog( "Removing", COMPONENT_FIELD, "field (comparison data must use tags)" )
    d_obs[ COMPONENT_FIELD ] <- NA
  }
  
  if( VARIABLE_FIELD %in% names( d_obs ) ) {
    printlog( "Removing", VARIABLE_FIELD, "field (comparison data must use tags)" )
    d_obs[ VARIABLE_FIELD ] <- NA
  }
  
  # At this point, toss out any fields that we're not going to need
  printlog( "-- removing", sum( !names( d_obs ) %in% fieldnames ), "extra fields" )
  
  printlog( "Finished reading comparison data sets" )
  d_obs[ names( d_obs ) %in% fieldnames ]	# remove extraneous fields and return
}

# -----------------------------------------------------------------------------
# Look for unmatched tags and print a warning
warn_unmatched <- function( d, checkfield, printfield ) {
  unmatched <- d[ is.na( d[ , checkfield ] ), ]
  if( nrow( unmatched ) ) {
    printlog_warn( "there are no tags for", printfield, ":", unique( unmatched[ , printfield ] ) )
  }
}

# -----------------------------------------------------------------------------
# Build a tag map. This lets us translate from raw names in the outputstream
# (or used by comparison data) to a single tag name that's prettier, descriptive,
# and common across all data
# This would be trivial except that tags might change, so we may need to add more
# entries to the list
build_tagmap <- function( fn, datafield, tagfield, oldtagfield, prefixfield=NA ) {
  
  printlog( "Reading tag database", fn, "..." )
  tagmap <- read.csv( fn, comment.char="#", stringsAsFactors = FALSE )
  names( tagmap ) <- tolower( names( tagmap ) )
  tagmap[ , tagfield ] <- tolower( tagmap[ , tagfield ] )
  tagmap[ , datafield ] <- tolower( tagmap[ , datafield ] )
  tagmap[ , oldtagfield ] <- tolower( tagmap[ , oldtagfield ] )
  printlog( "--", nrow( tagmap ), "tag entries" )
  
  if( any( tagmap[ , tagfield ]=="" ) )
    printlog_warn( "there are empty", tagfield, "fields" )
  if( any( tagmap[ , datafield ]=="" ) )
    printlog_warn( "there are empty", datafield, "fields" )
  
  # Now need to build old-to-new tag mapping. Start with self-reference
  selfs <- tagmap
  selfs[ oldtagfield ] <- selfs[ tagfield ]
  selfs[ datafield ] <- NA
  
  # Now deal with any truly old tags that need to point to new ones
  if( !is.na( oldtagfield ) ) {
    for( i in 1:nrow( tagmap ) ) {
      otf <- tagmap[ i, oldtagfield ]
      if( !is.na( otf ) & otf != "" ) {
        #				printlog( "-- processing", otf )
        tagmap[ i, oldtagfield ] <- NA	# remove multi-term entry
        olds <- strsplit( otf, OLD_TAG_SEPARATOR )[[ 1 ]]
        for( j in olds ) {		# add a new row
          printlog( "-- adding entry", j, "->", tagmap[ i, tagfield ] )
          tagmap[ nrow( tagmap )+1, ] <- tagmap[ i, ]
          tagmap[ nrow( tagmap ), oldtagfield ] <- j
          tagmap[ nrow( tagmap ), datafield ] <- NA
        }
      }
    }
  }

  rbind( tagmap, selfs )
}

# -----------------------------------------------------------------------------
# The variable tag system includes support for calculated tags - e.g.,
# "Fhalos" which is a tag to be associated with the sum of all the halocarbon
# variables. This function looks for any calculated tags and handles them.
calculated_tags <- function( d_sim, vmap ) {
  
  # dplyr grouping by a vector of strings (column names) is awkward. This solution from
  # http://stackoverflow.com/questions/21208801/group-by-multiple-columns-in-dplyr-using-string-vector-input
  # Columns to group by
  grp_cols <- c(YEAR_FIELD, SCENARIO_FIELD, COMPONENT_FIELD, CTAG_FIELD, RUNNAME_FIELD, SOURCE_FIELD, TYPE_FIELD)
  # Convert character vector to list of symbols
  dots <- lapply(grp_cols, as.symbol)
  
  printlog( "Checking for calculated vtags..." )
  vmap <- vmap[ grepl( "+", vmap[ , VARIABLE_FIELD ], fixed = TRUE ), ]	# filter
  if( nrow( vmap ) ) {
    for( i in 1:nrow( vmap ) ) {
      varstosum <- strsplit( vmap[ i, VARIABLE_FIELD ],"+", fixed = TRUE )[[ 1 ]]
      d1 <- d_sim[ d_sim[ , CTAG_FIELD ]==vmap[ i, CTAG_FIELD ], ]
      d1 <- d1[ d1[ , VARIABLE_FIELD ] %in% varstosum, ]
      d2 <- d1 %>%
        group_by_(.dots = dots) %>%
        summarise(value = sum(value))
      printlog( "-- calculating", vmap[ i, VTAG_FIELD ], ":", nrow( d1 ), "to", nrow( d2 ), "rows" )
      if( nrow( d2 ) ) {
        d2[ , VARIABLE_FIELD ] <- vmap[ i, VARIABLE_FIELD ]
        d2[ , VTAG_FIELD ] <- vmap[ i, VTAG_FIELD ]
        d_sim <- dplyr::rbind_list( d_sim, d2 )
      }
    }
  }
  d_sim
}

# -----------------------------------------------------------------------------
# The workhorse function: match (merge) observed and simulated data
matchdata <- function( d_sim, d_obs ) {
  stopifnot( is.data.frame( d_sim ) )
  stopifnot( is.data.frame( d_obs ) )
  
  # -----------------------------------------------------------------------------
  # 1. Read tag databases and build maps
  
  # The model output refers to model components names that may change, so we want to
  # map them to a tag name (which is stable). The comparison data refers to these
  # tags; but it may be referring to old tag names, so need to map those to new tags.
  # The old->new map also includes new->new mappings (so is universal).
  printlog( SEPARATOR )
  cmap <- build_tagmap( CMAPFILE, COMPONENT_FIELD, CTAG_FIELD, OLD_CTAG_FIELD )
  vmap <- build_tagmap( VMAPFILE, VARIABLE_FIELD, VTAG_FIELD, OLD_VTAG_FIELD )
  vmap[ , CTAG_FIELD ] <- tolower( vmap[ , CTAG_FIELD ] )  # will not have been done by built_tagmap
  
  # -----------------------------------------------------------------------------
  # 2. Component tags
  # The model output component names are easy: map them to corresponding tags.
  printlog( SEPARATOR )
  printlog( "Merging model output with component tag map..." )
  if( CTAG_FIELD %in% names( d_sim ) ) {
    d_sim[ CTAG_FIELD ] <- NULL
  }
  d_sim <- dplyr::left_join( d_sim, cmap, by=COMPONENT_FIELD )

  warn_unmatched( d_sim, CTAG_FIELD, COMPONENT_FIELD )
  
  # Comparison data is basically the same, except that we have to match the data's
  # ctags to the old_ctags in the map (renaming ctag field first)
  if(nrow(d_obs)) {  # there might not be any comparison data
    printlog( "Updating ctags in comparison data..." )
    names( d_obs )[ which( CTAG_FIELD == names( d_obs ) ) ] <- OLD_CTAG_FIELD
    d_obs <- merge( d_obs, cmap, by=OLD_CTAG_FIELD, all = TRUE )
    warn_unmatched( d_obs, CTAG_FIELD, OLD_CTAG_FIELD )
  }
  
  # -----------------------------------------------------------------------------
  # 3. Variable tags
  # For the model output, we merge the variable tag map with the output, by component
  # and variable (since the latter is nested within the former). This gives us our mapping.
  
  # First, however, look through the vtag map and see if there's any calculation
  # requests (i.e. variable of the form x+y+....z). If so, sum the listed variables,
  # producing a new data chunk with a new tag.
  d_sim <- calculated_tags( d_sim, vmap )
  
  # Now do the merge
  printlog( "Merging model output with variable tag map..." )
  if( VTAG_FIELD %in% names( d_sim ) )
    d_sim[ VTAG_FIELD ] <- NULL
  d_sim <- merge( d_sim, vmap, by=c( CTAG_FIELD, VARIABLE_FIELD ) )
  warn_unmatched( d_sim, VTAG_FIELD, VARIABLE_FIELD )
  
  # For comparison data, the tagnames is filled with <COMPONENT.VARIABLE>, and then
  # we match 'em up.
  if(nrow(d_obs)) {
    printlog( "Updating vtags in comparison data..." )
    names( d_obs )[ which( VTAG_FIELD == names( d_obs ) ) ] <- OLD_VTAG_FIELD
    d_obs <- merge( d_obs, vmap, by=c( CTAG_FIELD, OLD_VTAG_FIELD ), all = TRUE )
    warn_unmatched( d_obs, VTAG_FIELD, OLD_VTAG_FIELD )
  }
  
  # -----------------------------------------------------------------------------
  # 4. Combine data
  printlog( SEPARATOR )
  printlog( "Combining data..." )
  d <- as.data.frame(dplyr::rbind_list( d_sim, d_obs ))
  
  todelete <- is.na( d[ CTAG_FIELD ] ) | is.na( d[ VTAG_FIELD ] )
  printlog( "-- deleting", sum( todelete ), "records with unknown tags" )
  d <- d[ !todelete, ]
  
  printlog( "Available ctags:", unique( d[ , CTAG_FIELD ] ) )
  printlog( "Available vtags:", unique( d[ , VTAG_FIELD ] ) )
  printlog( "Available scenarios:", unique( d[ , SCENARIO_FIELD ] ) )
  printlog( "Year range:", range( d[ , YEAR_FIELD ], na.rm = TRUE ) )
  d
}

