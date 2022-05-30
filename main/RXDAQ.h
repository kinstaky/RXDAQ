#ifndef __RXDAQ_H__
#define __RXDAQ_H__

// #include "main/Interactor.h"
// #include "main/Crate.h"
#include "Interactor.h"
#include "Crate.h"
#include "dstring.h"

namespace RXDAQ {


class RXDAQ {
public:
	RXDAQ();
	virtual ~RXDAQ();

	// public method
	virtual void AddInteractor(Interactor *interactor_);
	virtual void AddCrate(Crate *crate_);
	virtual void Run();
protected:
	// compoments of the DAQ
	Interactor *interactor;
	Crate *crate;
	dstringPtr ds;
};

}		// namespace RXDAQ

#endif