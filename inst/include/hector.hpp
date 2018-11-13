/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
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
#include "component_data.hpp"
#include "unitval.hpp"
#include "message_data.hpp"
#include "h_exception.hpp"

/* Core functions */
#include "core.hpp"

/* Setup functions */
#include "ini_to_core_reader.hpp"

/* Output functions */
#include "csv_outputstream_visitor.hpp"


#endif
