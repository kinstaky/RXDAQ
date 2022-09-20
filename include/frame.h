#ifndef __FRAME_H__
#define __FRAME_H__

#include "include/parser.h"
#include "include/interactor.h"
#include "include/crate.h"

namespace rxdaq {

/// This class represents the whole frame of the rxdaq.
class Frame {
public:

	/// @brief constructor
	///
	Frame();


	/// @brief default destructor
	///
	~Frame() = default;


	/// @brief parse parameters and generate interactors
	///
	void Parse(int argc, char** argv);


	/// @brief set interactor directly
	///
	/// @param[in] interactor pointer to the interactor
	///
	inline void SetInteractor(std::unique_ptr<Interactor> &interacotr) {
		interactor_ = std::move(interacotr);
	}


	/// @brief set crate directly
	///
	/// @param[in] crate pointer to the interactor
	///
	inline void SetCrate(std::shared_ptr<Crate> &crate) {
		crate_ = crate;
	}


	/// @brief run the frame
	///
	void Run();

private:
	std::unique_ptr<Interactor> interactor_;
	std::shared_ptr<Crate> crate_;
};

}		// namespace rxdaq

#endif			// #ifndef __FRAME_H__