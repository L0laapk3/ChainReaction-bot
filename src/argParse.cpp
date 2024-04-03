#include "argParse.h"
#include "lib/args.hpp"


SearchStopCriteria parseArgs(int argc, char** argv) {
    args::ArgumentParser parser("Epic ChainReaction bot.", "Welcome robbie");
    args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
    args::ValueFlag<float> timeTarget(parser, "timeTarget", "Search time target [s]", {'t', "time"});
    args::ValueFlag<int> depthLimit(parser, "depthLimit", "Search depth limit in plies", {'d', "depth"});
    try {
        parser.ParseCLI(argc, argv);
    } catch (args::Help) {
        std::cout << parser;
        std::exit(0);
    } catch (args::ParseError e) {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        std::exit(1);
    } catch (args::ValidationError e) {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        std::exit(1);
    }
	SearchStopCriteria stop;
	if (timeTarget) stop.time = (float)(args::get(timeTarget) * 1000);
	if (depthLimit) stop.depth = args::get(depthLimit);
	if (!timeTarget && !depthLimit) stop.time = 500;
	if (timeTarget && depthLimit)
		std::cout << "Search stop criteria: " << stop.time << "ms OR " << stop.depth << " plies" << std::endl;
	else if (timeTarget)
		std::cout << "Search stop criteria: " << stop.time << "ms" << std::endl;
	else if (depthLimit)
		std::cout << "Search stop criteria: " << stop.depth << " plies" << std::endl;

	return stop;
}