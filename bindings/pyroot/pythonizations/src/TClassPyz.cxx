// Author: Enric Tejedor CERN  02/2019
// Original PyROOT code by Wim Lavrijsen, LBL

/*************************************************************************
 * Copyright (C) 1995-2019, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

// Bindings
#include "CPyCppyy.h"
#include "CPyCppyy/API.h"
#include "PyROOTPythonize.h"
#include "CPPInstance.h"
#include "Utility.h"
#include "ProxyWrappers.h"
#include "PyzCppHelpers.hxx"

// ROOT
#include "TClass.h"

using namespace CPyCppyy;

// Cast the void* returned by TClass::DynamicCast to the right type
PyObject *TClassDynamicCastPyz(PyObject *self, PyObject *args)
{
   // Parse arguments
   PyObject *pyclass = nullptr;
   PyObject *pyobject = nullptr;
   int up = 1;
   if (!PyArg_ParseTuple(args, const_cast<char *>("O!O|i:DynamicCast"),
                         &CPPInstance_Type, &pyclass,
                         &pyobject,
                         &up))
      return nullptr;

   // Perform actual cast - calls default implementation of DynamicCast
   TClass *cl1 = (TClass *)CPyCppyy::Instance_AsVoidPtr(self);
   TClass *cl2 = (TClass *)CPyCppyy::Instance_AsVoidPtr(pyclass);

   void *address = cl1->DynamicCast(cl2, CPyCppyy::Instance_AsVoidPtr(pyobject), up);

   if (CPPInstance_Check(pyobject)) {
      address = ((CPPInstance *)pyobject)->GetObject();
   } else if (PyInt_Check(pyobject) || PyLong_Check(pyobject)) {
      address = (void *)PyLong_AsLongLong(pyobject);
   } else {
      Utility::GetBuffer(pyobject, '*', 1, address, false);
   }

   // Now use binding to return a usable class
   TClass *klass = nullptr;
   if (up) {
      // Upcast: result is a base
      klass = (TClass *)GetTClass((CPPInstance *)pyclass)->DynamicCast(TClass::Class(), CPyCppyy::Instance_AsVoidPtr(pyclass));
   } else {
      // Downcast: result is a derived
      klass = (TClass *)GetTClass((CPPInstance *)self)->DynamicCast(TClass::Class(), cl1);
   }

   return BindCppObjectNoCast(address, Cppyy::GetScope(klass->GetName()));
}

////////////////////////////////////////////////////////////////////////////
/// \brief Add pythonization for TClass::DynamicCast.
/// \param[in] self Always null, since this is a module function.
/// \param[in] args Pointer to a Python tuple object containing the arguments
/// received from Python.
///
/// TClass::DynamicCast returns a void* that the user still has to cast (it
/// will have the proper offset, though). Fix this by providing the requested
/// binding if the cast succeeded.
PyObject *PyROOT::AddTClassDynamicCastPyz(PyObject * /* self */, PyObject *args)
{
   PyObject *pyclass = PyTuple_GetItem(args, 0);
   Utility::AddToClass(pyclass, "_TClass__DynamicCast", "DynamicCast");
   Utility::AddToClass(pyclass, "DynamicCast", (PyCFunction)TClassDynamicCastPyz);
   Py_RETURN_NONE;
}
