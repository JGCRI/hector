
library(readr)
library(ggplot2)
theme_set(theme_bw())

# Step 1: permafrost off versus on
# Cperm=0 versus Cperm=782 (value from DW paper) subtracting from Csoil
x1 <- read_csv("~/Desktop/outputstream_ssp245_default.csv",
               skip = 1, col_types = "iciccdc")
x1$which <- "Default"
default <- x1
x2 <- read_csv("~/Desktop/outputstream_ssp245_pf.csv",
               skip = 1, col_types = "iciccdc")
x2$which <- "Permafrost"
x <- rbind(x1, x2)

y <- x[x$variable %in% c("permafrost_c", "atmos_co2", "global_tas", "NPP"),]
y <- y[!y$spinup,]

p1 <- ggplot(y, aes(year, value, color = which)) +
    geom_line() +
    facet_wrap(~variable, scales = "free") +
    ggtitle("ssp245")
print(p1)

# Step 2: multi-biome permafrost
x1 <- read_csv("/Users/bpbond/Desktop/outputstream_ssp245_mb_pf_default.csv",
               skip = 1, col_types = "iciccdc")
x1$which <- "Multi-biome pf default"
x2 <- read_csv("/Users/bpbond/Desktop/outputstream_ssp245_mb_pf_warm.csv",
               skip = 1, col_types = "iciccdc")
x2$which <- "Multi-biome pf warm"
x <- rbind(x1, x2, default)

y <- x[x$variable %in% c("boreal.permafrost_c", "tropical.permafrost_c", "atmos_co2", "global_tas", "boreal.NPP", "tropical.NPP"),]
y <- y[!y$spinup,]

p2 <- ggplot(y, aes(year, value, color = which)) +
    geom_line() +
    facet_wrap(~variable, scales = "free") +
    ggtitle("ssp245")
print(p2)

