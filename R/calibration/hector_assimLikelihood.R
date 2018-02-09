##==============================================================================
## hector_assim_likelihood.R
## 
## Original code by Yawen Guan
## Modified for SLR model by Kelsey Ruckert
## Modified for BRICK model framework by Tony Wong
## Modified for Hector model framework by Ben Vega-Westhoff
##==============================================================================
## log.post  
## -Computes the log likelihood for a zero-mean AR1 process from
##   observations as described in Ruckert et al. (2016).
## -For further description and references, please read the paper
##   and the appendix.
##
## NOTE: Descriptions of how to use this for other observation and models
##   can be found in the R package in review "VAR1"
##==============================================================================

logl.ar1 = function(r,sigma1,rho1,eps1=0) # default obs error is 0
{
    n = length(r) # r is the residuals

    logl=0
    if(n>1) {
      w = r[2:n] - rho1*r[1:(n-1)] # this process whitens the residuals
      logl = logl + sum(dnorm(w,sd=sqrt((sigma1)^2+(eps1[c(-1)])^2),log=TRUE)) # add in the sum of
			# density of the whitened residuals with a standard deviation of the
			# variance and the obs. errors
      # eps1 using errors estimates from temp and ocean heat which change with year -> heteroscedastic
    }
  return(logl)
}
##==============================================================================
## rest of the statistical model
##==============================================================================
log.lik = function( parameters.in    , parnames.in , in.hector.in,
		    sections.in      , chain.str   , calib.folder, 
		    forcing.in       , ini.template, output.vars , 
		    output.components, mod.time    , l.project   ,
		    oidx, midx, obs, obs.err, ind.norm.data      )
{
        parvals = parameters.in

        ## Scale forcing if using it as a constraint
        if(!is.null(forcing.in)){
            alpha = parameters.in[match("alphatemperature",paste0(parnames.in,sections.in))]
            if(is.na(alpha)){ alpha = 1 }   
            forcing.total = forcing_total(  forcing  =forcing.in, alpha.doeclim =alpha      ,
                                            l.project=l.project , begyear       =mod.time[1],
                                            endyear  =mod.time[length(mod.time)])
            df_out = data.frame(year=mod.time,Ftot_constrain=forcing.total)
            forcing.file = paste0( calib.folder, "/", chain.str, "temp_RF.csv" )
            write.table(df_out,file=forcing.file,quote=FALSE,row.names=FALSE,col.names=TRUE,sep=",")
            parvals[match("alphatemperature",paste0(parvals,sections.in))] = 1 #Forcing already scaled, no need scale in Hector
	}
        else {
            forcing.file = NULL
        }
	
	## Run Hector
	model.out = hectorwrapper( parnames.in[in.hector.in],
				   parvals[in.hector.in],
				   sections.in[in.hector.in],
				   chain.str=chain.str,
				   working.dir=calib.folder,
				   forcing.file=forcing.file,
				   ini.template=ini.template,
				   output.vars=output.vars,
				   output.components=output.components,
				   mod.time )

        ## Convert model.out to match the variables/units of the observations
        model.out = convertVars ( model.out )
        
        ## Normalize, add offsets, and then sum the likelihoods of all of the model outputs
        llik = 0
        for ( i in 2:length( names(model.out) ) ) { # Time is the first list item here
	    ts = names( model.out ) [i]

            if( !is.na( ind.norm.data[match(ts,ind.norm.data[,1]),2] ) ) {
                itmp = ind.norm.data[match(ts,ind.norm.data[,1]),2]:ind.norm.data[match(ts,ind.norm.data[,1]),3]
                model.out[[ts]] = model.out[[ts]] - mean(model.out[[ts]][itmp])
            }
            offset = parameters.in[match(paste0('offset',ts,'_obs'),paste0(parnames.in,sections.in))]
	    sigma = parameters.in[match(paste0('sigma'  ,ts,'_obs'),paste0(parnames.in,sections.in))]
	    rho   = parameters.in[match(paste0('rho'    ,ts,'_obs'),paste0(parnames.in,sections.in))]
            resid = obs[[ts]][oidx[[ts]]] - (model.out[[ts]][midx[[ts]]] + offset)
	    llik.i = logl.ar1(resid,sigma,rho,obs.err[[ts]][oidx[[ts]]]) # AR(1)
	    llik = llik + llik.i # Currently, assume all likelihoods are independent! Big assumption!
        }

        return(llik)
}
##==============================================================================
## (log of the) prior probability. Currently all uniform.
## Could try something more complicated for some params at some point?
log.pri = function( parameters.in,
                    parnames.in,
                    bound.lower.in,
                    bound.upper.in )
{
	in.range = all(parameters.in >= bound.lower.in) & all(parameters.in <= bound.upper.in)
  	
	if(in.range){
		lpri.uni = 0	# Sum of all uniform priors (log(1)=0)
		lpri = lpri.uni
	} else {
		lpri = -Inf
	}

	return(lpri)
}
##==============================================================================
## (log of the) posterior distribution:  posterior ~ likelihood * prior
log.post = function(  parameters.in    , parnames.in   , in.hector.in,
	              sections.in      , chain.str     , calib.folder, 
		      forcing.in       , ini.template  , output.vars , 
		      output.components, mod.time      , l.project   , 
		      bound.lower.in   , bound.upper.in,
                      oidx, midx, obs, obs.err, ind.norm.data     )
{

	lpri = log.pri( parameters.in=parameters.in  ,
                        parnames.in=parnames.in      ,
                        bound.lower.in=bound.lower.in,
                        bound.upper.in=bound.upper.in)

  	if(is.finite(lpri)) { # evaluate likelihood if nonzero prior probability
    	    lpost = log.lik(  parameters.in=parameters.in,
                              parnames.in=parnames.in,
			      in.hector.in=in.hector.in,
			      sections.in=sections.in,
			      chain.str=chain.str,
			      calib.folder=calib.folder,
			      forcing.in=forcing.in,
			      ini.template=ini.template,
 			      output.vars=output.vars,
			      output.components=output.components,
			      mod.time=mod.time,
                              l.project=l.project,
			      oidx=oidx, midx=midx,
                              obs=obs  , obs.err=obs.err,
                              ind.norm.data             
			   ) + lpri
  	} else {
    	lpost = -Inf
  	}
  	return(lpost)
}
##==============================================================================
## End
##==============================================================================
