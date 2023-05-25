/* Hector -- A Simple Climate Model
   Copyright (C) 2022  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
#ifndef H_UTIL_H
#define H_UTIL_H
/*
 *  h_util.hpp
 *  hector
 *
 *  Created by Ben on 2/24/11.
 *  Copyright 2010 DOE Pacific Northwest Lab. All rights reserved.
 *
 *  Provides utility services
 */

#include <iostream>

#define H_STRINGIFY_VAR(var) #var

#define MODEL_NAME "hector"

/*!
 * \brief The model version number to be included in logs and outputs.
 * \note  Manually update the git tag to match this.
 */
#define MODEL_VERSION "3.1.1"

#define OUTPUT_DIRECTORY "output/"

namespace Hector {

void ensure_dir_exists(const std::string &dir);

}

#endif // H_UTIL_H
