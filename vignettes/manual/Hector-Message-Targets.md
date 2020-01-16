# Hector message targets

The following message targets are defined.

TODO:  indicate which ones are valid for reading and which are valid for writing, and give definitions for the less obvious ones.

```
// BC
 D_EMISSIONS_BC          

// Forcings
 D_RF_TOTAL              
 D_RF_T_ALBEDO           
 D_FTOT_CONSTRAIN        
 D_RF_BASEYEAR           
 D_RF_CO2                
 D_RF_N2O                
 D_RF_H2O                
 D_RF_O3                 
 D_RF_BC                 
 D_RF_OC                 
 D_RF_SO2d               
 D_RF_SO2i               
 D_RF_SO2                
 D_RF_VOL                
 D_RF_halocarbons        

// Halocarbon Forcings
 D_RF_CF4                
 D_RF_C2F6               
 D_RF_HFC23              
 D_RF_HFC32              
 D_RF_HFC4310            
 D_RF_HFC125             
 D_RF_HFC134a            
 D_RF_HFC143a            
 D_RF_HFC227ea           
 D_RF_HFC245fa           
 D_RF_SF6                
 D_RF_CFC11              
 D_RF_CFC12              
 D_RF_CFC113             
 D_RF_CFC114             
 D_RF_CFC115             
 D_RF_CCl4               
 D_RF_CH3CCl3            
 D_RF_HCF22              
 D_RF_HCF141b            
 D_RF_HCF142b            
 D_RF_halon1211          
 D_RF_halon1301          
 D_RF_halon2402          
 D_RF_CH3Cl              
 D_RF_CH3Br              

// Halocarbon Emissions
 D_EMISSIONS_CF4         
 D_EMISSIONS_C2F6        
 D_EMISSIONS_HFC23       
 D_EMISSIONS_HFC32       
 D_EMISSIONS_HFC4310     
 D_EMISSIONS_HFC125      
 D_EMISSIONS_HFC134a     
 D_EMISSIONS_HFC143a     
 D_EMISSIONS_HFC227ea    
 D_EMISSIONS_HFC245fa    
 D_EMISSIONS_SF6         
 D_EMISSIONS_CFC11       
 D_EMISSIONS_CFC12       
 D_EMISSIONS_CFC113      
 D_EMISSIONS_CFC114      
 D_EMISSIONS_CFC115      
 D_EMISSIONS_CCl4        
 D_EMISSIONS_CH3CCl3     
 D_EMISSIONS_HCF22       
 D_EMISSIONS_HCF141b     
 D_EMISSIONS_HCF142b     
 D_EMISSIONS_halon1211   
 D_EMISSIONS_halon1301   
 D_EMISSIONS_halon2402   
 D_EMISSIONS_CH3Cl       
 D_EMISSIONS_CH3Br       


// Methane
 D_ATMOSPHERIC_CH4       
 D_PREINDUSTRIAL_CH4     
 D_RF_CH4                
 D_EMISSIONS_CH4         
 D_NATURAL_CH4           
 D_CONVERSION_CH4        
 D_LIFETIME_SOIL         
 D_LIFETIME_STRAT        

// N2O
 D_ATMOSPHERIC_N2O       
 D_PREINDUSTRIAL_N2O     
 D_EMISSIONS_N2O         
 D_NAT_EMISSIONS_N2O     
 D_CONVERSION_N2O        
 D_INITIAL_LIFETIME_N2O  
 D_LIFETIME_N2O          

// OH
 D_LIFETIME_OH           
 D_INITIAL_LIFETIME_OH   
 D_COEFFICENT_NOX        
 D_COEFFICENT_CH4        
 D_COEFFICENT_NMVOC      
 D_COEFFICENT_CO         

// O3
 D_PREINDUSTRIAL_O3	 
 D_ATMOSPHERIC_O3	 
 D_ATMOSPHERIC_EM_CH4    
 D_EMISSIONS_NOX         
 D_EMISSIONS_CO          
 D_EMISSIONS_NMVOC       

// OC
 D_EMISSIONS_OC          

// Ocean
 D_OCEAN_CFLUX           
 D_OCEAN_C               
 D_TT                    
 D_TU                    
 D_TWI                   
 D_TID                   
 D_CIRC_TOPT             
 D_CIRC_T50_HIGH         
 D_CIRC_T50_LOW          
 D_HL_DO                 
 D_PH_HL                 
 D_PH_LL                 
 D_ATM_OCEAN_FLUX_HL     
 D_ATM_OCEAN_FLUX_LL     
 D_PCO2_HL               
 D_PCO2_LL               
 D_DIC_HL                
 D_DIC_LL                
 D_P0_LL                 
 D_CARBON_HL             
 D_CARBON_LL             
 D_CARBON_IO             
 D_CARBON_DO             
 D_OMEGACA_HL            
 D_OMEGACA_LL            
 D_OMEGAAR_HL            
 D_OMEGAAR_LL            
 D_TEMP_HL               
 D_TEMP_LL               
 D_SPINUP_CHEM           

 D_HEAT_FLUX             
 D_HEAT_UPTAKE_EFF       
 D_MAX_HEAT_UPTAKE_EFF   
 D_KAPPA50_TEMP          
 D_MIN_HEAT_UPTAKE_EFF   
 D_SLOPE_HEAT_UPTAKE_EFF 

 D_CO3_LL		 
 D_CO3_HL		 
 D_ATM_OCEAN_CONSTRAIN   
 D_TIMESTEPS             
 D_REVELLE_HL            
 D_REVELLE_LL            

// Land Carbon
 D_LAND_CFLUX            
 D_ATMOSPHERIC_CO2       
 D_PREINDUSTRIAL_CO2     
 D_ATMOSPHERIC_C         
 D_ATMOSPHERIC_C_RESIDUAL
 D_EARTHC                
 D_VEGC                  
 D_DETRITUSC             
 D_SOILC                 
 D_ANTHRO_EMISSIONS      
 D_LUC_EMISSIONS         
 D_CO2FERT               
 D_TEMPFERTD             
 D_TEMPFERTS             
 D_Q10_RH                
 D_NPP                   
 D_RH                    
 D_F_NPPV                
 D_F_NPPD                
 D_F_LITTERD             
 D_F_LUCV                
 D_F_LUCD                
 D_NPP_FLUX0             
 D_CA_CONSTRAIN          
 D_BETA                  
 D_WARMINGFACTOR         

// Sea Level Rise
 D_SL_RC                 
 D_SLR                   
 D_SL_RC_NO_ICE          
 D_SLR_NO_ICE            

// SO2
 D_NATURAL_SO2       
 D_2000_SO2     
 D_EMISSIONS_SO2         
 D_VOLCANIC_SO2          

// Temperature
 D_ECS                   
 D_GLOBAL_TEMP           
 D_GLOBAL_TEMPEQ         
 D_TGAV_CONSTRAIN        
 D_SO2D_B                
 D_SO2I_B                
 D_OC_B                  
 D_BC_B                  
```
