//
// Created by david on 28/12/15.
//

#ifndef LC_IOARGUMENTS_H
#define LC_IOARGUMENTS_H

#include "arguments.hh"

struct IOArguments {
		unsigned stage;
		unsigned block_size;
		std::string out_dir;
		Arguments n_args;
		std::ifstream reader;
		std::ofstream clusteredFile;

		IOArguments(const Arguments &args, unsigned s) :
				stage(s),
				block_size((unsigned) args.get_block_mb() * 1024 * 1024),
				out_dir(args.get_output_dir()),
				n_args(args) {
			reader.open(args.get_input().c_str());
			clusteredFile.open((out_dir + "/clustered.txt").c_str());
		}
};

#endif //LC_IOARGUMENTS_H
