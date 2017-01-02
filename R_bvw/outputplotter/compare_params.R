# Ben V-W June 2016
# Use plotting functions from Ben B-L, figures.R
# Compare results from runs with different parameters

INPUT_FILE		<- "doeclim.csv"
SCRIPTNAME		<- "compare_params.R"
OUTPUT_DIR		<- "outputs/"
LOG_DIR			<- "logs/"
SEPARATOR		<- "-------------------"

# -----------------------------------------------------------------------------
# Time-stamped output function
printlog <- function( msg="", ..., ts=TRUE, cr=TRUE ) {
    if( ts ) cat( date(), " " )
    cat( msg, ... )
    if( cr ) cat( "\n")
} # printlog

# -----------------------------------------------------------------------------
# Print dimensions of data frame
printdims <- function( d, dname=deparse( substitute( d ) ) ) {
    stopifnot( is.data.frame( d ) )
    printlog( dname, "rows =", nrow( d ), "cols =", ncol( d ) )
} # printdims

# -----------------------------------------------------------------------------
# Save a ggplot figure
saveplot <- function( pname, p=last_plot(), ptype=".pdf" ) {
    #stopifnot( file.exists( OUTPUT_DIR ) )
    fn <- paste0( OUTPUT_DIR, "/", pname, ptype )
    printlog( "Saving", fn )
    ggsave( fn, p )
} # saveplot

# -----------------------------------------------------------------------------
# Save a data frame
savedata <- function( df, extension=".csv" ) {
    stopifnot( file.exists( OUTPUT_DIR ) )
    fn <- paste0( OUTPUT_DIR, "/", deparse( substitute( df ) ), extension )
    printlog( "Saving", fn )
    write.csv( df, fn, row.names=F )
} # saveplot

# -----------------------------------------------------------------------------
# Open a csv file and return data
read_csv <- function( fn, datadir=".", ... ) {
    fqfn <- paste( normalizePath( datadir ), fn, sep="/" )
    printlog( "Opening", fqfn )
    stopifnot( file.exists( fqfn ) )
    read.csv( fqfn, stringsAsFactors=F, ... )
} # read_csv

# -----------------------------------------------------------------------------
# Load requested libraries
loadlibs <- function( liblist ) {
    printlog( "Loading libraries..." )
    loadedlibs <- vector()
    for( lib in liblist ) {
        printlog( "- loading", lib )
        loadedlibs[ lib ] <- require( lib, character.only=T )
        if( !loadedlibs[ lib ] )
        warning( "this package is not installed!" )
    }
    invisible( loadedlibs )
} # loadlibs

# ==============================================================================
# Figure functions

# -----------------------------------------------------------------------------
fig_timeseries <- function( d, vtagname, prettylabel ) {
    
    d1 <- subset( d1, vtag==vtagname &
    model %in% c( "CMIP5", "HECTOR", "MAGICC 5.3", "MAGICC6" )) #, "GCP", "BATS", "HOTS"))
    
}


# -----------------------------------------------------------------------------
fig_datasummary <- function( d ) {
    # Make a summary plot showing what variables are available to plot
    dgrp <- group_by( d, scenario, type, model )	# dplyr
    dsum <- na.omit( summarise( dgrp, count=n() ) )
    dsum$count_grp <- cut(dsum$count,c(1,10,100,1000,10000,100000))
    
    dev.new( height=10, width=10 )
    p <- qplot( scenario, model, data=subset( dsum, type!="observed" ), geom="tile", fill=count_grp )
    print( p )
    saveplot( "summary_scenario_model" )
    
    dgrp <- group_by( d, vtag, type, model )	# dplyr
    dsum <- na.omit( summarise( dgrp, count=n() ) )
    dsum$count_grp <- cut( dsum$count,c( 1, 10,100,1000,3000))
    p <- qplot( vtag, model, data=subset( dsum, type!="observed" ), geom="tile", fill=count_grp )
    p <- p +  theme( axis.text.x  = element_text( angle=90 ) )
    print( p )
    saveplot( "summary_vtag_model" )
    dev.off()
    
}

# ==============================================================================
renormalize <- function( d, normalizeYears ) {
    if( all( !is.na( normalizeYears ) ) ) {
        stopifnot( is.numeric( normalizeYears ) )
        
        # Compute mean value during the normalization period
        # NOTE: 'scenario' isn't included in the ddply call below, because we want
        # RCPs to be normalized by a historical period. But if you're plotting
        # multiple scenarios in a single plot, and normalizing by a future period,
        # this will fail.
        #d_norm <- ddply( d[ d$year %in% normalizeYears, ],
        #.( ctag, vtag, model, scenario ), summarise,
        #value_norm_mean = mean( value ) )
        d_norm <- ddply( d[ d$year >= min(normalizeYears) & d$year <= max(normalizeYears) ],
        .( ctag, vtag, model, scenario ), summarise,
        value_norm_mean = mean( value ) )
        d <- merge( d, d_norm, by= c("ctag", "vtag", "model", "scenario"))
        
        # Relativize the error_min and max, normalize, then recompute them
        d$error_min <- with( d, error_min - value )
        d$error_max <- with( d, error_max - value )
        d <- ddply( d, .( ctag, vtag, model, scenario, type, units), mutate, value = value - value_norm_mean,
        error_min = error_min , error_max = error_max )
        d$error_min <- with( d, error_min + value )
        d$error_max <- with( d, error_max + value )
        d$value_norm_mean <- NULL
        #d <- ddply( d, .( ctag, vtag, model, scenario, type, units), mutate, value = value - value_norm_mean)
        #d$value_norm_mean <- NULL
    }
    d
}

offsetSLR <- function(d, offsetYear, H0, scenario){
    #Offset values (specifically used for SLR where we have some 1880 initial height given in Ruckert and Rahmstorf papers
    #-14.64 for R07 (model slr has alpha=0.34), -15.35 for Ruckert 2016 (model slr has alpha=0.2)
    offsetSLR = H0 - mean( d$value[ d$vtag == "slr" & d$scenario == scenario & !is.na( d$value ) & d$year >= offsetYear & d$year < ( offsetYear+1 ) ] )
    d$value[ d$vtag=="slr" & d$scenario == scenario & !is.na( d$value ) ] = d$value[ d$vtag== "slr" & d$scenario == scenario & !is.na( d$value ) ] + offsetSLR
    d
}

offsetSLRall <- function(d, offsetYear, H0){
    #same as above but just do for all hector model scenarios (assuming we've one of them for all Hector runs).
    slist = unique(d$scenario[d$model=="HECTOR"])
    for (sc in slist[!is.na(slist)]){
        offsetSLR = H0 - mean( d$value[ d$vtag == "slr" & d$scenario == sc & d$model == "HECTOR" & !is.na( d$value ) & d$year >= offsetYear & d$year < ( offsetYear+1 ) ] )
        d$value[ d$vtag=="slr" & d$scenario == sc & !is.na( d$value ) ] = d$value[ d$vtag== "slr" & d$scenario == sc & !is.na( d$value ) ] + offsetSLR }
    d
}

renormalizeSLR <- function( d, normalizeYears, scenario){
    #Renormalize some scenario over a normalization period. doin it this way because only VR09 has no H0 listed, so we renormalize @ 1980.
    offsetSLR = mean( d$value[ d$vtag == "slr" & d$scenario == scenario & !is.na( d$value ) & d$year >= min(normalizeYears) & d$year <= ( max(normalizeYears) ) ] )
    d$value[ d$vtag=="slr" & d$scenario == scenario & !is.na( d$value ) ] = d$value[ d$vtag== "slr" & d$scenario == scenario & !is.na( d$value) ] - offsetSLR
    d
}
# -----------------------------------------------------------------------------
# Time series of w/e variable (e.g. tgav) for all scenarios (hard-coded here atm)
fig_param_comparison <- function( d, vtagname, prettylabel, xl=c(1850,2100), normalizeYears=NA, opt_yl = NULL ) {
    printlog( "Plotting", vtagname )
    d1 <- subset( d, vtag==vtagname & model %in% c( "HECTOR" ) & year >= xl[1] & year <= xl[2] )
    #d1 = renormalize(d1,normalizeYears)
    printdims( d1 )
    
    p <- ggplot( d1, aes( year, value, color=scenario ) )
    p <- p + geom_line( aes( color=scenario, linetype=model), size=1.7 )
    p <- p + xlab( "Year" ) + ylab( prettylabel )
    p <- p + scale_color_discrete( "SLR param source")#, labels=c("VR09","Ruckert","R07") )
    #p <- p + scale_linetype_discrete( "Model", labels=c( "Hector", "MAGICC" ) )
    if (!is.null(opt_yl)) {p <- p + ylim( opt_yl)}
    p <- p + theme(text = element_text(size=15))
    print( p )
    saveplot( paste0( "param_comparison_", vtagname,"_source" ) )
    invisible( p )
}

# -----------------------------------------------------------------------------
# Historical time series of w/e variable (e.g. tgav) w/ all scenarios
hist_comparison <- function( d, vtagname, prettylabel,comparisonmodels=c(), xl=c(1850,2015), normalizeYears=NA,scenarios=c("rcp85"),fsfx="") {
    printlog( "Plotting", vtagname )
    d1 <- subset( d, vtag==vtagname & model %in% union(comparisonmodels,"HECTOR") & year >= xl[1] & year <= xl[2] & scenario %in% scenarios )
    #d1 <- subset(d, year >=xl[1] & year <= xl[2])
    #d1 = renormalize(d1,normalizeYears)
    printdims( d1 )
    p <- ggplot( d1, aes( year, value, color=scenario, linetype=model ) )
    p <- p + geom_line( aes( color=scenario, linetype=model), size=1.0)#size=1.7 )
    p <- p + xlab( "Year" ) + ylab( prettylabel )
    p <- p + scale_color_discrete( "Run type")#, labels=c("Obs.","VR09","Ruckert","R07") )
    p <- p + scale_linetype_discrete("Data source")#,labels=union(rev(comparisonmodels),"Hector"))
    p <- p + theme(text = element_text(size=15))
    print( p )
    saveplot( paste0( "hist_comparison_", vtagname,fsfx ) )
    invisible( p )
}


# ==============================================================================
# Main

loadlibs( c( "ggplot2", "reshape2", "plyr", "plotrix", "data.table", "smatr" ) )  # the hadleyverse

    
OUTPUT_DIR <- normalizePath( OUTPUT_DIR )
if( !file.exists( OUTPUT_DIR ) ) {
    printlog( "Creating", OUTPUT_DIR )
    dir.create( OUTPUT_DIR )
}
LOG_DIR <- normalizePath( LOG_DIR )
if( !file.exists( LOG_DIR ) ) {
    printlog( "Creating", LOG_DIR )
    dir.create( LOG_DIR )
}

sink( paste( LOG_DIR, paste0( SCRIPTNAME, ".txt" ), sep="/" ), split=T )

printlog( "Welcome to", SCRIPTNAME )

#loadlibs( c( "ggplot2", "reshape2", "plyr", "lubridate", "plotrix", "data.table", "smatr" ) )	# the hadleyverse
#theme_set( theme_bw() )
#loadlibs( c( "ggplot2", "reshape2", "plyr", "plotrix", "data.table", "smatr" ) )  # the hadleyverse
theme_set( theme_bw() )

d <- as.data.table( fread( INPUT_FILE ) )  # now using data.table's fread
printdims( d )
d <- subset( d, !spinup | is.na( spinup ) )		# remove all spinup data
d$spinup <- NULL    # ...and a lot of crap fields
d$notes.x <- NULL
d$notes.y <- NULL
d$variable.x <- NULL
d$variable.y <- NULL
d$old_ctags <- NULL
d$old_vtags <- NULL
printdims( d )

# Definitions for 'pretty' axis labels
tgav_pretty <- expression( Temperature~change~group("(",paste(degree,C),")") )
oaflux_pretty <- expression( Atmosphere-ocean~flux~(Pg~C~yr^{-1}))
laflux_pretty <- expression( Atmosphere-Land~Flux~(Pg~C~yr^{-1}))
ph_pretty <- expression( Low~Latitude~pH )
atmos_co2_pretty <- expression(Atmospheric~CO[2]~(ppmv))
ftot_pretty <- expression( Forcing~(W~m^{-2}) )

print("Plotting comparisons of the different runs")
#fig_param_comparison(d,"atmos_co2",atmos_co2_pretty)
#fig_param_comparison(d,"tgav",tgav_pretty)
#fig_param_comparison(d,"ftot",ftot_pretty)

#because we're looking at SLR, offset it with appropriate H0 values first
#d = offsetSLR(d,1880,-14.64,"R07")
#d = offsetSLR(d,1880,-15.35,"Ruckert")
#d = renormalizeSLR(d,c(1989.5:1990.5),"VR09")
#d = offsetSLRall(d, 1880, -15.35) #adopting Ruckert parameterization for now

#fig_param_comparison(d,"slr","Sea Level (cm)")
hist_comparison(d,"tgav",tgav_pretty,xl=c(1850,2050),scenarios=c("default","doeclim","volcanoes","doeclim volcanoes"),fsfx="_doeclim")
hist_comparison(d,"tgav",tgav_pretty,comparisonmodels=c("HadCRUT4","MAGICC6"),xl=c(1850,2020),scenarios=c("Historical","rcp85","doeclim volcanoes"),fsfx="_doeclim2")
hist_comparison(d,"tgav",tgav_pretty,comparisonmodels=c("MAGICC6"),xl=c(1850,2120),scenarios=c("rcp85","doeclim volcanoes", "volcanoes"),fsfx="_doeclim3")
#hist_comparison(d,"tgav",tgav_pretty,comparisonmodels=c("HadCRUT4","MAGICC6"),xl=c(1850,2020),scenarios=c("Historical","rcp85","emissions-based F","prescribed F"))
#hist_comparison(d,"ftot",ftot_pretty,comparisonmodels=c("MAGICC6"),xl=c(1850,2020),scenarios=c("rcp85","prescribed F","emissions-based F"))
#hist_comparison(d,"slr","Sea Level (cm)",comparisonmodels=c("Church"),xl=c(1880,2050))
#hist_comparison(d,"slr_no_ice","thermosteric slr (cm)",comparisonmodels=c("Levitus"),xl=c(1970,2015),normalizeYears=1990:2000)
