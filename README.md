[![Build Status](https://travis-ci.org/JGCRI/hector.svg?branch=master)](https://travis-ci.org/JGCRI/hector)

Hector
======

This is the repository for **Hector**, an open source, object-oriented, simple global climate carbon-cycle model. It  runs essentially instantaneously while still representing the most critical global scale earth system processes, and is one of a class of models heavily used for for emulating complex climate models and uncertainty analyses. For example, Hector's global temperature rise for the RCP 8.5 scenario, compared to observations and other model results, looks like this:

![](https://github.com/JGCRI/hector/wiki/rcp85.png)

The primary source of Hector model documentation is the Github [wiki](https://github.com/JGCRI/hector/wiki); please note that the wiki documents are included in the repository, but you'll need to use `git clone --recursive` to get them installed locally. The code is also well documented with [Doxygen](http://doxygen.org)-style comments. A formal model description paper ([Hartin et al. 2015](http://www.geosci-model-dev.net/8/939/2015/gmd-8-939-2015.html)) documents its science internals and performance relative to observed data, the [CMIP5](http://cmip-pcmdi.llnl.gov/cmip5/) archive, and the reduced-complexity [MAGICC](http://www.magicc.org) model.

This work is supported by the [Integrated Assessment Research Program](http://science.energy.gov/ber/research/cesd/integrated-assessment-of-global-climate-change/) of the Office of Science, U.S. Department of Energy. The Pacific Northwest National Laboratory is operated for DOE by Battelle Memorial Institute under contract DE-AC05-76RL01830.

## Tools and Software That Work with Hector

* [GCAM](https://github.com/JGCRI/gcam-core): Hector can be used as
  the climate component in the GCAM integrated assessment model.  
* [pyhector](https://github.com/openclimatedata/pyhector): a python
  interface to Hector.  
  
  
