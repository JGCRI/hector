/* Hector -- A Simple Climate Model
   Copyright (C) 2022  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
/*
 *  h_util.cpp
 *  hector
 *
 *  Created by Pralit Patel on 1/14/22.
 *
 */

#include "h_util.hpp"
#include "h_exception.hpp"

// If using the R package, use Rcpp to call R's file processing
// functions. Otherwise (e.g. if building standalone Hector),
// use std::filesystem (which is available since the C++ 17 standard)
// if available and finally fall back to boost::filesystem (which
// needs to be installed).

// Language feature detection (to use __cpp_lib_filesystem) isn't even
// available unil the C++20 standard.  Luckily we can use boost to get
// it in the meantime.
#include <boost/config.hpp>

#ifdef USE_RCPP
#include <Rcpp.h>
#elif __cpp_lib_filesystem || __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
typedef std::error_code fs_error_code;
#else
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
typedef boost::system::error_code fs_error_code;
#endif

namespace Hector {

using namespace std;
using namespace boost;

/*!
 * \brief Checks if the given directory exists. If not it attempts to create
 *        it and if it was unable to do that it will raise an exception.
 * \param dir The directory to check/create.
 */
void ensure_dir_exists(const std::string &dir) {
#ifdef USE_RCPP
  // Load R functions for path management
  Rcpp::Environment base("package:base");
  Rcpp::Environment utils("package:utils");
  Rcpp::Function dirCreate = base["dir.create"];
  Rcpp::Function file_test = utils["file_test"];
  // use file_test -d to see if dir exists and is a directory
  if (!Rcpp::as<bool>(file_test("-d", dir))) {
    // either does not exist or is a file
    // we can try to create it and if it still fails it must
    // have been a file or a permissions error
    if (!Rcpp::as<bool>(dirCreate(dir))) {
      // no luck, throw exception
      H_THROW("Directory " + dir + " does not exist and could not create it.");
    }
  }
#else
  fs::path fs_dir(dir);
  // first check to see if dir exists and is a directory
  if (!fs::is_directory(fs_dir)) {
    // either does not exist or is a file
    // we can try to create it and if it still fails it must
    // have been a file or a permissions error
    fs_error_code status;
    if (!fs::create_directory(fs_dir, status)) {
      H_THROW("Directory " + dir + " does not exist and could not create it.");
    }
  }
#endif
}

} // namespace Hector
