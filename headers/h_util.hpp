/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2 as
   published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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
#define MODEL_VERSION "1.1"

#define OUTPUT_DIRECTORY "output/"

#endif // H_UTIL_H
