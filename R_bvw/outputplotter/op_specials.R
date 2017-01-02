# Make nice graphs of Hector stream output
# This script runs as a backend to the main Hector process
# Functions for 'special' graphs that can't be handled by op_graph()

ipcc_forcings <- function( d, yr, fn="ipcc.pdf" ) {		# data and year

	printlog( "Welcome to ipcc_forcings" )
	
	forcings_to_plot <- tolower( c( "Ftot", "FCO2", "Fhalos", "FN2O", "Fbc", "Foc" ) )
	
	d <- subset( d, year==yr & vtag %in% forcings_to_plot )
	d <- subset( d, type=="HECTOR" | source=="IPCC" )

	p <- ggplot( d, aes( x=vtag, y=value, fill=source ) )
	p <- p + geom_bar( stat="identity", position="dodge" )
	p <- p + geom_errorbar( aes( ymin=error_min, ymax=error_max ), 
		position=position_dodge( width=0.9 ), width=0.2 )
	p <- p + xlab( "Forcing agent" ) + ylab( "W/m2" ) + ggtitle( yr )
	p <- p + coord_flip()
	print( p )
	ggsave( fn )
}
