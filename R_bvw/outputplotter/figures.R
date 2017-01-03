# copied by Ben Vega-Westhoff May 2016 from
# https://github.com/cahartin/hector_documentation_figures/blob/master/figures.R
# R script to generate figures for documentation ms
# Ben Bond-Lamberty & Corinne Hartin
# June 2014

# Support functions and common definitions

INPUT_FILE		<- "test_default_rcps.csv"
SCRIPTNAME		<- "figures.R"
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

# -----------------------------------------------------------------------------
taylorcolor <- function( modelname ) { 
    tcolors <- rep("darkgrey", length( modelname ) )
    tcolors [ modelname %in% c( "CMIP5", "CMIP5 median") ] <- "red"
    tcolors [ modelname == "HECTOR"] <- "green"
    tcolors [ modelname == "MAGICC6"] <- "blue"
    
    return (tcolors)
}

# -----------------------------------------------------------------------------
cor_table <- function( d, normalizeYears=NA, refmodel="HECTOR" ) {
    printlog( "Making correlation & error table..." )
    
    d1 <- renormalize( d, normalizeYears )
    
    ddply_fields <- c( "model", "year", "value", "scenario", "vtag", "units" )
    d_ref <- subset( d1, model==refmodel )[ , ddply_fields ]
    d_others <- subset( d1, model!=refmodel )[ , ddply_fields ]
    d_merge <- merge( d_ref,d_others, by=c( "scenario", "vtag", "year" ) )
    d_merge <- d_merge[complete.cases(d_merge),]
    d_summary <- ddply( d_merge, .( scenario, vtag, model.y ), summarise,
                        minyear=min( year ),
                        maxyear=max( year ),
                        cor=round( cor( value.y, value.x ), 3 ),
                        cov=round( cov( value.y, value.x ), 3 ), 
   
   r2=round( summary( lm( value.y~value.x ) )$r.squared, 3 ),
                        rmse=round( summary( lm( value.y~value.x ) )$sigma, 3 ),
                        int0=round ( summary( lm( value.y~value.x ) )$coefficients[1,4], 3 ),
                        slope1=round ( slope.test( value.y, value.x, test.value=1 )$p, 3 ),
                        units=unique( units.x ) )
    return( d_summary )
}

# -----------------------------------------------------------------------------
# sets a reference model for the Taylor diagrams
fig_taylor <- function( d, vtagname, prettylabel, normalizeYears=NA, refmodel="HECTOR" ) {
    printlog( "Plotting", vtagname )
    d1 <- subset( d, vtag==vtagname )
    printdims( d1 )
    
    d1 <- renormalize( d1, normalizeYears )
    
    printlog( "Taylor plot..." )		# following code from taylor.diagram help
    library( plotrix )
    png( paste0( OUTPUT_DIR, "/", "taylor_", vtagname, ".png" ) )
    
    d_ref <- subset( d1, model==refmodel )[ , c( "model", "year", "value" ) ]
    d_others <- subset( d1, model!=refmodel )[ , c( "model", "year", "value" ) ]
    models <- unique( d_others$model )
#    colors <- rainbow( length( models )+1 )
    oldpar <- taylor.diagram( d_ref$value, d_ref$value, main="", 
    						col=taylorcolor( refmodel ),
                            xlab=prettylabel, ylab=prettylabel ) #colors[ 1 ] )
    for( m in 1:length( models ) ) {
        printlog( m, models[ m ] )
        d_mod <- subset( d_others, model==models[ m ] )
        d_mod$year <- round( d_mod$year )   # some models report 2001.5, 2002.5, etc.
        d_plot <- merge( d_ref, d_mod, by="year" )

        mod <- summary( lm( value.y~value.x, data=d_plot ) )
        printlog( "Hector versus", models[ m ], " R2=", round( mod$r.squared, 3 ), " RSE=", round( mod$sigma, 3 ) )
#        if( nrow( d_mod ) < 3 ) next
        taylor.diagram( d_plot$value.x, d_plot$value.y, add=T, col=taylorcolor( models[ m ] ) )
    }
    
    # get approximate legend position
    lpos <- 1.35 * sd( d_ref$value )
    # add a legend
    modellist <- c("HECTOR", "MAGICC6", "CMIP5 median", "CMIP5 model" )
    legend( lpos, lpos, legend=modellist, pch=19, col=taylorcolor( modellist ) )
    # now restore par values
    par( oldpar )
    dev.off()
}

# -----------------------------------------------------------------------------
fig_magicc_comparison <- function( d, vtagname, prettylabel ) {
	printlog( "Plotting", vtagname )
	d1 <- subset( d, vtag==vtagname & model %in% c( "HECTOR", "MAGICC6" ) )
	printdims( d1 )
    
	p <- ggplot( d1, aes( year, value, color=scenario ) ) 
	p <- p + geom_line( aes( color=scenario, linetype=model ) )
	p <- p + xlab( "Year" ) + ylab( prettylabel )
	p <- p + scale_color_discrete( "RCP", labels=c("2.6","4.5","6.0","8.5") )
	p <- p + scale_linetype_discrete( "Model", labels=c( "Hector", "MAGICC" ) )
	p <- p + xlim( c( 1850, 2300 ) )
	print( p )
	saveplot( paste0( "magicc_comparison_", vtagname ) )
	invisible( p )
}

# -----------------------------------------------------------------------------




# -----------------------------------------------------------------------------
cmip5_comparison <- function( d, vtagname, prettylabel, normalizeYears=NA, facet=T ) {
    printlog( "Plotting", vtagname )
    d1 <- subset( d, vtag==vtagname )
    d1 <- renormalize( d1, normalizeYears )
    d1$future <- d1$year > 2100
    printdims( d1 )
        
    p <- ggplot( d1, aes( year, value, color=model ) )
    p <- p + geom_line( size=2 )
    p <- p + geom_ribbon( aes( ymin=value-error, ymax=value+error, fill=model ), color=NA, alpha=0.5, show_guide=F )
    p <- p + geom_ribbon( aes( ymin=error_min, ymax=error_max, fill=model ), color=NA, alpha=0.25, show_guide=F )
    if (facet) p <- p + facet_grid( ~future, scales="free_x", space="free_x" )
    p <- p + theme( strip.text.x = element_blank() ) + theme( strip.background = element_blank() )
    p <- p + xlab( "Year" ) + ylab( prettylabel )
    p <- p + scale_color_discrete( "Model" )
    p <- p + scale_x_continuous( expand=c( 0, 0 ) )
    print( p )
    saveplot( paste0( "cmip5_comparison_", vtagname ) )
    invisible( p )
}


# ==============================================================================
# Main

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
loadlibs( c( "ggplot2", "reshape2", "plyr", "plotrix", "data.table", "smatr" ) )  # the hadleyverse
theme_set( theme_bw() )

#theme_update( ... )

# R reads big files a LOT faster if you pre-specify the column types
#cc <- c( 	"ctag"="character",
#			"variable"="character",
#			"component"="character",
#			"year"="numeric",
#			"run_name"="character",
#			"spinup"="numeric",
#			"value"="numeric",
#			"units"="character",
#			"source"="character",
#			"scenario"="character",
#			"model"="character",
#			"type"="character",
#			"error"="numeric", "error_min"="numeric", "error_max"="numeric",
#			"old_ctags"=NULL, "notes.x"=NULL ,
#			"vtag"="character",
#			"prettylabel"=NULL, "old_vtags"=NULL, "notes.y"=NULL,"variable.x"=NULL,"variable.y"=NULL )
#d <- read_csv( INPUT_FILE, colClasses=cc  )
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

# Figures comparing Hector and MAGICC (no CMIP)
printlog( "Figures comparing Hector and MAGICC (no CMIP)" )
fig_magicc_comparison( d, "atmos_co2", expression( Atmospheric~CO[2]~(ppmv) ) )
fig_magicc_comparison( d, "tgav", tgav_pretty )
fig_magicc_comparison( d, "ftot", expression( Forcing~(W~m^{-2}) ) )

# Figures comparing Hector and CMIP5 (plus MAGICC)
cmip5 <- subset( d, scenario %in% c( "rcp85", "historical", "esmHistorical", "esmrcp85", "Historical" ) &
                     model %in% c( "HECTOR", "MAGICC6", "CMIP5", "HadCRUT4", "Law Dome", "MaunaLoa", "GCP", "BATS", "HOTS") &
                     year >= 1850 )
cmip5_comparison( cmip5, "tgav", tgav_pretty, normalizeYears=1850 )
cmip5_comparison( cmip5, "ftot", tgav_pretty, normalizeYears=1850 )

# zoom in on first few years
p <- cmip5_comparison( cmip5, "tgav", tgav_pretty, normalizeYears=1850, facet=F ) 
print(p+coord_cartesian(xlim=c(1850, 2005), ylim=c(-2,2)))
saveplot("tgav_obs_comparison")

p<-cmip5_comparison( cmip5, "atmos_co2", expression( Atmospheric~CO[2]~(ppmv) ), facet=F )
print(p+coord_cartesian(xlim=c(1850,2100), ylim=c(200,1200)))
saveplot("atmos_co2")
print(p+coord_cartesian(xlim=c(1850,2005), ylim=c(250,400)))
saveplot("atmos_co2_hist")

p<-cmip5_comparison( cmip5, "atm_ocean_flux", oaflux_pretty)
print(p+coord_cartesian(ylim=c(-2,20)))
saveplot("aoflux_comparison")

# smoothing
cmip5_yr<-cmip5[order(cmip5$year), ] #sorts in order of year 
ma <- function( x, n=5, s=1 ){ as.numeric( filter( x, rep( 1/n, n ), sides=2 ) ) }
cmip5_sm <- ddply( cmip5_yr, .( ctag, vtag, model, type, units, scenario), mutate, value = ma(value), error = ma(error), 
                   error_min = ma(error_min), error_max = ma(error_max))
cmip5_comparison( cmip5_sm, "nbp", laflux_pretty)

p<-cmip5_comparison( cmip5, "ph_ll", ph_pretty, facet=F)
print(p+coord_cartesian(xlim=c(1850,2100), ylim=c(7.7, 8.2)))
saveplot("ph_ll_comparison")


# renormalize here
cmip5 <- subset( d, #scenario %in% c( "rcp85", "historical", "esmHistorical", "esmrcp85", "Historical" ) &
                     model %in% c( "HECTOR", "MAGICC6", "CMIP5", "HadCRUT4", "Law Dome", "MaunaLoa", "GCP", "BATS", "HOTS") &
                     year >= 1850 )
cmip5 <- renormalize( cmip5, 1850 )
cmip5 <- cmip5[ cmip5$year > 2015, ]

labels <- data.frame( scenario=c( "rcp26","rcp45","rcp60","rcp85"),lbl=c('(a)','(b)','(c)','(d)'))
p <- cmip5_comparison( cmip5, "tgav", tgav_pretty, normalizeYears=NA, facet=F )
p <- p + facet_wrap( ~scenario, scales="free_y" )+coord_cartesian(xlim=c(2005,2300) )
#p <- p + geom_text( data=labels, aes(label=lbl),x=2010,)
print( p )
saveplot( "tgav_allrcps" )

# Taylor plots comparing Hector to all CMIP5 models
cmip5indiv <- subset( d, scenario %in% c( "rcp85", "historical" ) &
                          year >= 1850 )
fig_taylor( cmip5indiv, "tgav", tgav_pretty, normalizeYears=1961:1990 )

#fig_taylor( cmip5indiv, "atm_ocean_flux", oaflux_pretty, normalizeYears=1850 )

# -----------------------------------------------------------------------------
# Make a nice table of correlation & error between Hector and everything else
cordata <- subset( d, scenario %in% c( "esmrcp85", "esmHistorical", "rcp85", "historical", "Historical" ) & year >= 1850 )

# Now we want to report combined values - for entire historical + rcp period
cordata$scenario <- "rcp85 & historical"  
table1<-( cor_table( cordata, 1850 ) )
write.csv(table1, "table_combined.csv", row.names = F)

# Models have totally inconsistent treatment of EVERYTHING
# In this case, we really want pre-2005 years to be labelled 'historical'
cordata$scenario<-"rcp85"
cordata[ cordata$year < 2005, "scenario" ] <- "historical" 
table2<-( cor_table( cordata, 1850 ) )

write.csv(table2, "historical_rcp_table.csv", row.names = F)

print( sessionInfo() )
printlog( "All done with", SCRIPTNAME )
sink()
