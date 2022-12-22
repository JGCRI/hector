/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
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
