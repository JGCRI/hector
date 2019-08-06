/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

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


#define H_STRINGIFY_VAR(var) \
    #var

#define MODEL_NAME "hector"

/*!
 * \brief The model version number to be included in logs and outputs.
 * \note This must be updated manually when the model version changes.
 */
#define MODEL_VERSION "2.4.0"

#define OUTPUT_DIRECTORY "output/"

#endif // H_UTIL_H
