---
name: Release
about: Checklist! 
---

Since the dev branch only includes materials that have been peer-reviewed at least once this PR review process should be relatively smooth should. That being said, before merging this branch into main and the new release is created several things must be completed first. 

* Open a PR into main branch, it should be titled with the new release number  
* Let Leeyabot generate a report detailing how the Hector output has changed! 
* Add text to the PR documenting the major model changes. The PR is going to serve as an important record 
* Pass automated checks
* Change the version number in the DESCRIPTION file 
* Update the NEWS.md, are links additional materials necessary? Does the release need a new naming file? 
* Check to make sure the internal package data is up to date (fxntable, inputstable, unitstable)

After merging PR 

* Create the release via zenodo 
* Add the zenodo links to README / NEWS & push directly to main 
* Create new dev branch & update the DESCRIPTION version number to support the pkgdown documentation for the developmental branch!
* Add archived outputstreams to release assets!

