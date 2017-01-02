# Make nice graphs of Hector stream output
# This script runs as a backend to the main Hector process
# The main graphing function

GRAPH_NUMBER	<- 1
DISPLAYPLOTS 	<- TRUE
SAVEPLOTS 		<- TRUE
WAITPLOTS 		<- FALSE
SAVEDATA		<- TRUE
PLOTFILETYPE	<- "pdf"

library( ggplot2 )				# currently ggplot 1.0.1
theme_set( theme_bw() )

# -----------------------------------------------------------------------------
# Moving average function
# Based on http://stackoverflow.com/questions/743812/calculating-moving-average-in-r
# n is window size; s=1 (for backward only) or 2 (centered window)
ma <- function( x, n=5, s=1 ){ as.numeric( filter( x, rep( 1/n, n ), sides=2 ) ) }

# -----------------------------------------------------------------------------
# General-purpose plotting function
# Puts all (or a subset of) variables on a single faceted plot
# Generates as many plots as there are model components
op_graph <- function( d, 								# Data
				ctaglist=unique( d[ , CTAG_FIELD ] ), 	# Component tags to plot (one per page)
				vtaglist=unique( d[ , VTAG_FIELD ] ), 	# Variable tags to plot
				color=TYPE_FIELD,						# Color field (TODO: can also be a string)
				linetype=NA, 							# Linetype field (can be a number)
				shape=NA, 								# Point shape field (can also be a number)
				size=NA,								# Size (can also be a number)
				facetx=".", facety=".",					# Faceting fields
				xlim=NA, ylim=NA, 						# X and Y axis limits
				pname=NA, fname=NA,	 					# Plot name and file name
				plot_points=FALSE,						# Plot points?
				plot_lines=TRUE,							# Plot lines?
				clean_directory=FALSE					# Clear output directory?
					) {

	printlog( SEPARATOR )
	printlog( "Welcome to", as.character( match.call()[[ 1 ]] ) )
	ctaglist <- tolower( ctaglist )
	vtaglist <- tolower( vtaglist )
	printlog( "-- requested ctags:", ctaglist )
	printlog( "-- requested vtags:", vtaglist )
	
	# Subset the data by component and variable, if necessary
	printlog( "-- full data,", nrow( d ), "rows" )
	if( !any( is.na( ctaglist ) ) )  d <- subset( d, ctag %in% ctaglist )
	printlog( "-- limiting by ctag,", nrow( d ), "rows" )
	if( !any( is.na( vtaglist ) ) ) d <- subset( d, vtag %in% vtaglist )
	printlog( "-- limiting by vtag,", nrow( d ), "rows" )

	if( nrow( d )==0 ) {
		printlog_warn( "this ctag/vtag combination does not exist!" )
		return()
	}
	
	# Create output directory, if necessary
	runs <- sort( unique( d[ , RUNNAME_FIELD ] ) )
	foldername <- paste( runs[ !is.na( runs ) ], collapse="_" )
	if( foldername=="" ) foldername <- "output"
	if( !file.exists( foldername ) ) {
		printlog( "Creating", foldername )
		dir.create( foldername )
	}
	if( clean_directory ) {
		printlog( "Removing old files in", foldername )
		file.remove( paste0( foldername, "/", list.files( foldername ) ) )
	}
	
	for( i in unique( d[ , CTAG_FIELD ] ) ) {
		printlog( i )
		d1 <- d[ d[ , CTAG_FIELD ]==i, ] # subset( d, ctag==i )
		if( nrow( d1 )==0 ) {
			printlog( "-- no data, skipping" )
			next
		}
		d1[ , TYPE_FIELD ] <- as.factor( as.character( d1[ , TYPE_FIELD ] ) )
		d1[ , PRETTY_FIELD ] <- as.factor( d1[ , PRETTY_FIELD ] )
		
		# Build file and plot names 
		gnum <- formatC( GRAPH_NUMBER, width=3, flag="0" )
		GRAPH_NUMBER <<- GRAPH_NUMBER + 1
		vtags <- unique( d1[ , VTAG_FIELD ] )
		vtags_string <- paste( vtags, collapse="." )
		
		if( is.na( fname ) )
			fname <- paste0( foldername, "/", gnum, ".", i, ".", vtags_string )
		
		pstring <- paste0( "ggplot( d1, aes( x=", YEAR_FIELD, ", y=", VALUE_FIELD, " ) )" )
		
		# ----- Points/lines -----
		colorstring <- ""
		if( !is.na( color ) ) colorstring <- paste0( "color=", color, ", fill=", color )
		shapestring <- ""
		if( !is.na( shape ) ) shapestring <- paste( ", shape=", shape )
		linetypestring <- ""
		if( !is.na( linetype ) ) linetypestring <- paste( ", linetype=", linetype )
		sizestring <- ""
		if( !is.na( size ) & is.numeric( size ) ) sizestring <- paste( ", size=", size )

		pointsize <- 1.5
		linesize <- 1.5
		if( is.numeric( size ) ) {
			pointsize <- size
			linesize <- size
		}
		
		if( plot_points )
			pstring <- paste0( pstring, " + geom_point( size=", pointsize, ", aes( ", colorstring, shapestring, ") )" )
		if( plot_lines )
			pstring <- paste0( pstring, " + geom_line( size=", linesize, ", aes( ", colorstring, linetypestring, ") )" )

		# ----- Faceting -----
		# facet_grid( facets=prettylabel~., scale="free", labeller=label_parsed )
		if( facetx != "." | facety != "." )
			pstring <- paste0( pstring, " + facet_grid( ", facety, "~", facetx, " )" )

		# ----- Axis limits -----
		xlimstring <- ""
		if( any( !is.na( xlim ) ) ) xlimstring <- "xlim=xlim"
		ylimstring <- ""
		if( any( !is.na( ylim ) ) ) ylimstring <- ", ylim=ylim"
#		commastring <- ""
#		if( xlimstring != "" & ylimstring != "" ) commastring <- ", "
		if( xlimstring != "" | ylimstring != "" )
			pstring <- paste0( pstring, " + coord_cartesian( ", xlimstring, ylimstring, " )" )

		# Annotate with time
		
#		if( length( unique( d1$vtag ) )==1 ) {
#			printlog( "Changing axis label" )
#			pl <- gsub( " ", "~", d1[ 1, "prettylabel" ] )	# pretty (expression) label
#			un <- gsub( " ", "~", d1[ 1, "units" ] )		# units, changing spaces to tildes
#			lb <- paste( pl, "~(", un, ")", sep="" )
#			printlog( lb )
#			p <- p + ylab( parse( text=lb ) )
#			fname <- paste( fname, d1[ 1, "vtag" ] )
#		}

		# ----- Error regions -----
		d1_minmax <- d1[ !is.na( d1[ , ERRORMIN_FIELD ] ) & !is.na( d1[ , ERRORMAX_FIELD ] ), ]
		if( nrow( d1_minmax ) ) {
			printlog( "-- adding error minmax data" )
			fillstring <- ""
			if( !is.na( color ) ) fillstring <- paste( ", fill=", color )			
			pstring <- paste( pstring, "+ geom_ribbon( aes( ymin=", ERRORMIN_FIELD, ", ymax=", ERRORMAX_FIELD, fillstring, " ), data=d1_minmax, alpha=0.15 )" )
		}

		d1_err <- d1[ !is.na( d1[ , ERROR_FIELD ] ), ]
		if( nrow( d1_err ) ) {
			printlog( "-- adding error data" )
			d1_err$err_max <- d1_err[ , VALUE_FIELD ] + d1_err[ , ERROR_FIELD ]
			d1_err$err_min <- d1_err[ , VALUE_FIELD ] - d1_err[ , ERROR_FIELD ]
			fillstring <- ""
			if( !is.na( color ) ) fillstring <- paste( ", fill=", color )			
			pstring <- paste( pstring, "+ geom_ribbon( aes( ymin=err_min, ymax=err_max", fillstring, " ), data=d1_err, alpha=0.30 )" )
		}
		
		# ----- Title -----
		if( is.na( pname ) ) pname <- paste( fname, "\n", date() )
		pstring <- paste0( pstring, " + ggtitle( '", pname, "' )" )
	
		# ----- Axis title
		if( length( vtags )==1 )
			pstring <- paste0( pstring, " + ylab( '", vtags, " (", unique( d1$units ), ")' )" )
			
		# ----- Evaluate the string we've built up -----
		errmsg <- NA
		printlog( "-- plotting", fname, "..." )
		printlog( pstring )	
		tryCatch( {
				p <- eval( parse( text=pstring ) )
				if( DISPLAYPLOTS ) print( p )
			}, error=function( err ) {
				printlog_warn( "Error trying to generate this graph" )
				p <- qplot( 0, 0, label=paste( "ERROR", as.character( err ), sep="\n" ), geom="text", main=pname )
				errmsg <<- as.character( err )
				printlog( as.character( err ) )
			}	
		)

		if( WAITPLOTS ) readline( "<return>" )
    
		if( is.na( fname ) || fname != "" ) {
			fname <- gsub( " ", "_", fname )					# no spaces allowed
			fname <- gsub( "[^./A-Za-z0-9_-]", "", fname )		# nor unsafe characters
			dname <- paste( fname, "csv", sep="." )
			fname <- paste( fname, PLOTFILETYPE, sep="." )
			if( SAVEPLOTS ) {
				printlog( "-- saving", fname )
				ggsave( fname, plot=p )
			}
			if( SAVEDATA ) {
				printlog( "-- saving", dname )
				write.csv( d1, file=dname, row.names = FALSE )
			}
		}
		# All done!	
	}
	
	invisible( list( d=d1, p=last_plot(), pstring=pstring ) )
}
