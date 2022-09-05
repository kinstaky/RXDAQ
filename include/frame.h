#ifndef __FRAME_H__
#define __FRAME_H__

#include "include/parser.h"
#include "include/interactor.h"

namespace rxdaq {


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
	/// @param interactor pointer to the interactor
	void SetInteractor(std::unique_ptr<Interactor> &interacotr);


	/// @brief run the frame
	///
	void Run() const;

private:
	std::unique_ptr<Interactor> interactor_;
};

}		// namespace rxdaq

#endif			// #ifndef __FRAME_H__