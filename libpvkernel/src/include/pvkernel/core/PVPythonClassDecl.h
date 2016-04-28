/**
 * @file
 *
 * @copyright (C) Picviz Labs 2010-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef PVCORE_PVPYTHONCLASSREGISTER
#define PVCORE_PVPYTHONCLASSREGISTER

#ifdef ENABLE_PYTHON_SUPPORT

namespace PVCore
{

class PVPythonInitializer;

class PVPythonClassDecl
{
	friend class PVPythonInitializer;

  public:
	virtual ~PVPythonClassDecl() {}

  protected:
	virtual void declare() = 0;
	virtual PVPythonClassDecl* clone() const = 0;
};

class PVPythonClassRegister
{
  public:
	PVPythonClassRegister(PVPythonClassDecl const& c);
};
}

#define PYTHON_EXPOSE()                                                                            \
  public:                                                                                          \
	struct __PythonDecl : public PVCore::PVPythonClassDecl {                                       \
		void declare();                                                                            \
		PVCore::PVPythonClassDecl* clone() const { return new __PythonDecl(); }                    \
	};

#define PYTHON_EXPOSE_IMPL(T)                                                                      \
	static PVCore::PVPythonClassRegister __python_register__ =                                     \
	    PVCore::PVPythonClassRegister(T::__PythonDecl());                                          \
	void T::__PythonDecl::declare()

#endif

#endif
