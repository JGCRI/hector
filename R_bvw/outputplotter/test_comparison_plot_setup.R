#test_comparison_plot_setup.R
#----Combines hector output from several runs w/ 
#    comparison data into large .csv.
#    This is set-up for plots comparing hector 
#    output with that of MAGICC and CMIP5.
#Instructions -
#1 - run hector with your several .ini files (one per RCP)
#2 - run this
#3 - run figures.R (or compare_params.R if comparing results w/ diff parameter values)

in_root = "../../output/outputstream_rcp"
#INFILES <- c(paste0(in_root,"26.csv"),paste0(in_root,"45.csv"),paste0(in_root,"60.csv"),paste0(in_root,"85.csv"))
in_root = paste0(in_root,"85")
INFILES <- c(paste0(in_root,".csv"),paste0(in_root,"_doeclim.csv"),paste0(in_root,"_volcanoes.csv"), paste0(in_root,"_doeclim_volcanoes.csv"))
#c(paste0(in_root,".csv"),paste0(in_root,"_prescribedF.csv"))#,paste0(in_root,"k3.csv"),paste0(in_root,"k4.csv"),paste0(in_root,"k5.csv"),paste0(in_root,"k6.csv"))

#SCENARIOS <-c("rcp26","rcp45","rcp60", "rcp85")
#SCENARIOS <-c("0.1","0.3","0.5","0.7", "0.9", "1.1")
#SCENARIOS <-c("emissions-based F","prescribed F")
SCENARIOS <-c("default","doeclim","volcanoes","doeclim volcanoes")

COMPARISON_DATA <- "comparison_data/GMD_2015/"

source("op.R")

csv_file = "doeclim.csv"
#csv_file = "prescribedF.csv"
#write.csv(d,"test_default_rcps.csv")
write.csv(d,csv_file)

print(paste0("combined hector data and comparison data into ",csv_file," for plotting."))
