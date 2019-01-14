[![Build Status](https://travis-ci.org/JGCRI/hector.svg?branch=master)](https://travis-ci.org/JGCRI/hector)
[![Build status](https://ci.appveyor.com/api/projects/status/2jmhvq7n2ap75u1o/branch/master?svg=true)](https://ci.appveyor.com/project/rplzzz/hector/branch/master)

Hector
======

This is the repository for **Hector**, an open source, object-oriented, simple global climate carbon-cycle model. It  runs essentially instantaneously while still representing the most critical global scale earth system processes, and is one of a class of models heavily used for for emulating complex climate models and uncertainty analyses. For example, Hector's global temperature rise for the RCP 8.5 scenario, compared to observations and other model results, looks like this:

![](https://github.com/JGCRI/hector/wiki/rcp85.png)

The primary source of Hector model documentation is the Github [wiki](https://github.com/JGCRI/hector/wiki). Please note that the wiki documents are included in the repository as a git [submodule](https://git-scm.com/book/en/v2/Git-Tools-Submodules). To clone the repository as well as the documentation, you can use `git clone --recursive`. If you have already cloned non-recursively, you can fetch (download) the documentation by running `git submodule init; git submodule update`. The code is also well documented with [Doxygen](http://doxygen.org)-style comments. A formal model description paper ([Hartin et al. 2015](http://www.geosci-model-dev.net/8/939/2015/gmd-8-939-2015.html)) documents its science internals and performance relative to observed data, the [CMIP5](http://cmip-pcmdi.llnl.gov/cmip5/) archive, and the reduced-complexity [MAGICC](http://www.magicc.org) model.

This research was supported by the U.S. Department of Energy, Office of Science, as part of research in Multi-Sector Dynamics, Earth and Environmental System Modeling Program. The Pacific Northwest National Laboratory is operated for DOE by Battelle Memorial Institute under contract DE-AC05-76RL01830.

## Tools and Software That Work with Hector

* [GCAM](https://github.com/JGCRI/gcam-core): Hector can be used as
  the climate component in the GCAM integrated assessment model.  
* [pyhector](https://github.com/openclimatedata/pyhector): a python
  interface to Hector.  
  
  
