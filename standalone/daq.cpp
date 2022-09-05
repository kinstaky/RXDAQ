#include "include/frame.h"

int main(int argc, char **argv) {
	rxdaq::Frame frame;
	frame.Parse(argc, argv);
	frame.Run();
	return 0;
}