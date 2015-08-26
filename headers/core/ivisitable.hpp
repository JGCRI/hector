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
#ifndef IVISITABLE_H
#define IVISITABLE_H
/*
 *  ivistable.h
 *  hector
 *
 *  Created by Pralit Patel on 10/29/10.
 *
 */

namespace Hector {
  
class AVisitor;

//------------------------------------------------------------------------------
/*! \brief IVisitable interface
 *
 *  An interface to allow a visitor to visit a subclass.
 */
class IVisitable {
public:
    inline virtual ~IVisitable();
    
    /*! \brief Accept a visitor to visitor your class.
     *
     *  This allows a subclass use the double dispatch design pattern to allow
     *  the visitor to identify which subclass it is visiting.
     */
    virtual void accept( AVisitor* visitor ) = 0 ; /* const ? */
};

// Inline methods
IVisitable::~IVisitable() {
}

}

#endif // IVISITABLE_H
