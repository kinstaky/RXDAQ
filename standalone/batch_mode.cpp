// #include <unistd.h>
#include <string>
#include <iostream>

#include "pixie16/pixie16.h" 

#include "include/config.h"
#include "include/frame.h"


class BatchProcessor : public rxdaq::Interactor {
public:
	virtual void Run(std::shared_ptr<rxdaq::Crate> crate) override {
		crate->Initialize("config.json");
		crate->Boot(kModuleNum, false);
		crate->StartRun(kModuleNum, 1, 500);
	}
};

int main() {
	rxdaq::Frame frame;
	std::unique_ptr<rxdaq::Interactor> interactor =
		std::make_unique<BatchProcessor>();
	frame.SetInteractor(interactor);
	frame.Run();

	return 0;
}