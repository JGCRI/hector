# How to contribute

We welcome and encourtage third-party patches, which are essential for advancing the science and architecture of Hector. 
But there are a few guidelines that we ask contributors to follow.

## Getting Started

* Make sure you have a [GitHub account](https://github.com/signup/free)
* **Open an issue** describing your proposed change or work (after making sure one does not already exist).
  * Clearly describe the issue including steps to reproduce when it is a bug.
  * Discuss how your change will affect Hector, and thus whether it's MAJOR, MINOR, or a PATCH.
  * Interact with the project maintainers to refine/change/prioritize your issue and identify what branch will be targeted (see below).
* Trivial changes to comments or documentation do not require creating a new issue. Just start the first line of a commit with '(doc)' instead of an issue number.
* Fork the repository on GitHub

## Making Changes

* **Start your work on the correct branch**.
  * Active branches are based on Hector's [version numbering system](https://github.com/JGCRI/hector/wiki/VersionNumbers).
  * If your change is a PATCH, it will typically be based on the current dev branch; if MINOR, the next minor release branch; if MAJOR, the next major release branch.
  * We will **never** accept pull requests to the `master` branch.
* Follow Hector's [coding style](https://github.com/JGCRI/hector/wiki/StyleGuide).
* Make commits of logical units.
* Check for unnecessary whitespace with `git diff --check` before committing.
* Make sure your commit messages are in the proper format and **reference the relevant issue number**.
* Make sure you have added the necessary tests for your changes.
* Run _all_ the tests to assure nothing else was accidentally broken.

## Submitting Changes

* Push your changes to your fork of the repository.
* Submit a pull request to the main Hector repository.
* **Your pull request should include one of the following two statements**:
   * You own the copyright on the code being contributed, and you hereby grant PNNL unlimited license to use this code in this version or any future version of Hector. You reserve all other rights to the code.
   * Somebody else owns the copyright on the code being contributed (e.g., your employer because you did it as part of your work for them); you are authorized by that owner to grant PNNL an unlimited license to use this code in this version or any future version of Hector, and you hereby do so. All other rights to the code are reserved by the copyright owner.
* The core team looks at Pull Requests on a regular basis, and will respond as soon as possible.
* After feedback has been given we expect responses within two weeks. After two
  weeks we may close the pull request if it isn't showing any activity.

# Additional Resources

* [General GitHub documentation](http://help.github.com/)
* [GitHub pull request documentation](http://help.github.com/send-pull-requests/)
