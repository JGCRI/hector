##==============================================================================
## obs_readData.R
##
## Original GSIC version by Kelsey Ruckert
## Modified for DOECLIM by Tony Wong
## Modified for Hector by Ben Vega-Westhoff
##
## Load observational time series and their associated uncertainty time series,
## for use in Hector calibration
##
## Currently includes:
##   -Surface temperature, HadCRUT4 global means (1850-2016), Morice et al (2012)
##   -Ocean heat, 0-3000 m (1953-1996), Gouretski and Koltermann (2007)
## 
##==============================================================================

##==============================================================================
## Temperature
# HADCRUT4 annual global mean surface temperature
# Note: all ensemble member files have same time series of uncertainties, so just
# grabbing the first one.
dat = read.table("obs_constraints/temperature/HadCRUT.4.4.0.0.annual_ns_avg.txt")
obs.temp = dat[,2]
obs.temp.time = dat[,1]
dat = read.table("obs_constraints/temperature/HadCRUT.4.4.0.0.annual_ns_avg_realisations/HadCRUT.4.4.0.0.annual_ns_avg.1.txt")
obs.temp.err = dat[,3]

# Normalize temperature anomaly so 1850-1870 mean is 0
norm.lower = 1850; norm.upper = 1870
ibeg=which(obs.temp.time==norm.lower)
iend=which(obs.temp.time==norm.upper)
obs.temp = obs.temp - mean(obs.temp[ibeg:iend])

idx = compute_indices(obs.time=obs.temp.time, mod.time=mod.time)
oidx.temp = idx$oidx; midx.temp = idx$midx

hadcrut_obs = list( midx = midx.temp, oidx = oidx.temp,
		 obs  = obs.temp,  obs.time = obs.temp.time,
		 obs.err = obs.temp.err, norm.lower = norm.lower,
		 norm.upper = norm.upper )
##==============================================================================
# Annual global ocean heat content (0-3000 m)
dat = read.table("obs_constraints/ocheat/gouretski_ocean_heat_3000m.txt",skip=1)
obs.ocheat = dat[,2]
obs.ocheat.time = dat[,1]
obs.ocheat.err = dat[,3]

# Don't normalize these ocheat anomalies
norm.lower = NA; norm.upper = NA

idx = compute_indices(obs.time=obs.ocheat.time, mod.time=mod.time)
oidx.ocheat = idx$oidx; midx.ocheat = idx$midx

gouretski_obs = list( midx = midx.ocheat, oidx = oidx.ocheat,
		   obs = obs.ocheat, obs.time = obs.ocheat.time,
                   obs.err = obs.ocheat.err, norm.lower = norm.lower,
		   norm.upper = norm.upper )  
##==============================================================================
# !!!Add additional observational data sets to calibrate against

##==============================================================================
# !!!Add/switch in additional observational data sets to the obs.all list
obs.all = list( hadcrut_obs, gouretski_obs )
names(obs.all) = c( "Tgav", "ocheat" ) #Should match names in hector_calib_params.R
##==============================================================================
## End
##==============================================================================
