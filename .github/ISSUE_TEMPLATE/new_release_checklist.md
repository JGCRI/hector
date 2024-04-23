---
name: Release
about: Checklist! 
---

Since the dev branch only includes materials that have been peer reviewed at least once
this PR should be reasonably easy to get merged into main. That being said it is important 
that this PR is well documented with the major Hector behavior changes! 

* Open a PR into main titled with the new release number 
* Let Leeyabot generate a report detailing how the Hector output has changed! 
* Add text to the PR documenting the major model changes 
* Update the outputstreams via command line & commit them as part of the PR 
* Pass automated checks
* Change the version number in the DESCRIPTION file 
* Update the NEWS.md 
    * Does the this need any additional materials like a new naming file? 
* Check to make sure the internal package data is up to date (fxntable, inputstable, unitstable)

After merging PR 

* Create the release via zenodo 
* Add the zenodo links to README / NEWS & push directly to main 
* Create new dev branch & update the DESCRIPTION version number to support the pkgdown documentation for the developmental branch!


