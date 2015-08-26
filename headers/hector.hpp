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
/*
  Hector external header file.

  This file will include all of the Hector header files needed by external users
  of the Hector API.  Most of these are definitions for data types and symbolic
  constants, along with the externally facing methods of the Core class.   If
  for some reason you need to access Hector's internals (this is *not
  recommended*, but it's your funeral), you can always include those header
  files directly.

  This header is for external codes that wish to use the API *only*.  Hector
  source files should *not* include this file.

*/

#ifndef HECTOR_H
#define HECTOR_H

/* Data types and constants */
#include "components/component_data.hpp"
#include "data/unitval.hpp"
#include "data/message_data.hpp"
#include "h_exception.hpp"

/* Core functions */
#include "core/core.hpp"

/* Setup functions */
#include "input/ini_to_core_reader.hpp"

/* Output functions */
#include "visitors/csv_outputstream_visitor.hpp"


#endif
