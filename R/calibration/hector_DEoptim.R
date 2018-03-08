##==============================================================================
## hector_DEoptim.R
##
## Original code for BRICK by Tony Wong
## Modified for Hector by Ben Vega-Westhoff
##==============================================================================
## minimize_residuals_hector:
## Run Hector and calculate the uncertainty-weighted residuals 
##   of the model output w.r.t. observations.
## Called in the DEoptim section of the calibration driver.
## Part of the differential evolution optimization to find 
##   initial parameters of the MCMC.
##==============================================================================

minimize_residuals_hector = function(	parameters.in, parnames.in , in.hector.in     ,
					sections.in  , calib.folder, forcing.in       , 
					ini.template , output.vars , output.components,   
					mod.time     , l.project   ,
					oidx, midx, obs, obs.err, ind.norm.data )
{
    parvals = parameters.in
   
    ## Scale forcing if using it as a constraint
    if(!is.null(forcing.in)){
        alpha = parvals[match("alphatemperature",paste0(parnames.in,sections.in))]
        if(is.na(alpha)){ alpha = 1 } 
        forcing.total = forcing_total(  forcing  =forcing.in, alpha.doeclim =alpha      ,
                                        l.project=l.project , begyear       =mod.time[1],
                                        endyear  =mod.time[length(mod.time)])
        df_out = data.frame(year=mod.time,Ftot_constrain=forcing.total)
        forcing.file = paste0( calib.folder,"/DEoptim_temp_RF.csv" )
        write.table(df_out,file=forcing.file, quote=FALSE,row.names=FALSE,col.names=TRUE,sep=",")
        parvals[match("alphatemperature",paste0(parnames.in,sections.in))] = 1 #Forcing already scaled, no need to do it in Hector
    }
    else {
        forcing.file = NULL
    }

    ## Run Hector
    model.out = hectorwrapper( parnames.in[in.hector.in],
                               parvals[in.hector.in],
                               sections.in[in.hector.in],
                               chain.str="DEoptim",
			       working.dir=calib.folder,
			       forcing.file=forcing.file,
			       ini.template=ini.template,
			       output.vars=output.vars,
			       output.components=output.components,
			       mod.time )

    ## Convert model.out to match the variables/units of the observations
    model.out = convertVars ( model.out )	

    ## Normalize, add offsets, and then sum the errors of all of the model outputs
    err.sum = 0
    for ( i in 2:length( names(model.out) ) ) { # Time is the first list item here
        ts = names( model.out )[i]

	if( !is.na( ind.norm.data[match(ts,ind.norm.data[,1]),2] ) ) {
            itmp = ind.norm.data[match(ts,ind.norm.data[,1]),2]:ind.norm.data[match(ts,ind.norm.data[,1]),3]
            model.out[[ts]] = model.out[[ts]] - mean(model.out[[ts]][itmp])
        }
        offset = parameters.in[match(paste0("offset",ts,"_obs"),paste0(parnames.in,sections.in))]
        err.sum = err.sum + sum( abs( obs[[ts]][oidx[[ts]]] - 
				      ( model.out[[ts]][midx[[ts]]] + offset )    ) / 
		                 obs.err[[ts]][oidx[[ts]]] )
    }
    if(is.nan(err.sum)) err.sum=Inf
    
    return(err.sum)
}

##==============================================================================
## End
##==============================================================================
