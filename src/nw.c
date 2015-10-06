#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "common.h"

/* Algorithms enumeration */
enum {
	ALGO_UNKNOWN = -1,
	ALGO_RECURSIVE = 0,
	ALGO_ITERATIVE,
	ALGO_PARALLELIZED,
	ALGO_CLUSTERIZED,
};

algo_t algorithms[] = {
	{
		"recursive",
		"recusrive implementation",
		NULL
	},
	{
		"iterative",
		"iterative implementation",
		NULL
	},
	{
		"parallelized",
		"parallelized iterative implementation",
		NULL
	},
	{
		"clusterized",
		"clusterized parallelized implementation",
		NULL
	},
};

void print_algo_list(void) {
	for (int i = 0; i < countof(algorithms); i++) {
		printf(" -%s\t\t%s %s\n",
		       algorithms[i].name,
		       algorithms[i].desc,
		       (!algorithms[i].func) ? "(not implemented)" : "");
	}
}

int find_algo_id(const char* name) {
	for (int i = 0; i < countof(algorithms); i++) {
		if (!strcmp(name, algorithms[i].name)) {
			return i;
		}
	}
	return ALGO_UNKNOWN;
}

void help() {
	printf("usage: nw [options] sequence1 sequence2\n\n"

	       "use needleman-wunsch algorithm to do global"
	       " alignment of sequences.\n\n"

	       "options are:\n"
	       " -h, --help		print this help\n"
	       " -s, --string		(default) sequences are program arguments\n"
	       " -f, --file		sequences are read from files\n"
	       " -S, --Single 		sequences are from one file\n"
	       " -a, --algorithm <algo>	use given algorithm for alignment\n"
	       " -t, --time		print algorithm run time\n"
	       " -c, --core 		specify the number of cores (iterative only)\n"
	       " -v, --validate <file>	validate computed alignment using `file`\n"
	       " -o, --output <file>	print alignment(s) to a file instead of stdout\n\n"

	       "algorithm list:\n"
	      );

	print_algo_list();
}

/* Load mode of sequences */
enum {
	LM_ARGUMENTS,
	LM_FILES,
	LM_SINGLE_FILE,
};


int main(int argc, char** argv) {
	if (argc < 2) {
		help();
		return 1;
	}

	/* program parameters */
	int load_mode = LM_ARGUMENTS;
	int algorithm = ALGO_ITERATIVE;
	int do_bench  = 0;
	int core_number = 1;
	int do_validation = 0;
	char validation_file[512] = "";
	int file_output = 0;
	char output_path[512] = "";
	char seq_a_path[512], seq_b_path[512];
	algo_arg_t args;
	algo_res_t res;

	/* parsing options */
	char opt_c = 0;
	while ((opt_c = getopt(argc, argv, "hsfSta:c:v:o:b:")) > 0) {
		switch (opt_c) {
		    case '?':
		    case ':':
			help();
			return 1;

		    case 'h':
			help();
			return 0;

		    case 's':
			load_mode = LM_ARGUMENTS;
			break;

		    case 'f':
			load_mode = LM_FILES;
			break;

		    case 'S':
			load_mode = LM_SINGLE_FILE;
			break;
		
		    case 't':
			do_bench = 1;
			break;

		    case 'a':
			algorithm = find_algo_id(optarg);
			if (algorithm == ALGO_UNKNOWN) {
				printf("algorithms are:\n");
				print_algo_list();
				return 1;
			}
			break;

		    case 'c':
			if (sscanf(optarg, "%d", &core_number) != 1) {
				printf("invalid number of cores\n");
				return 1;
			}
			break;

		    case 'v':
			do_validation = 1;
			strcpy(validation_file, optarg);
			break;
			
		    case 'o':
			file_output = 1;
			strcpy(output_path, optarg);
			break;
			
			
		}
	}

	/* Check sequences are given */
	if (load_mode == LM_SINGLE_FILE && argc - optind < 1) {
		printf("please specify single input sequences file\n");
		return 1;
	}
	else if (load_mode != LM_SINGLE_FILE && argc - optind < 2) {
		printf("please specify input sequences\n");
		return 1;
	}

	/* Load sequences */
	if (load_mode == LM_ARGUMENTS) {
		if (argc - optind < 2) {
			printf("please specify input sequences\n");
			return 1;
		}
		args.seq_a = argv[optind];
		args.seq_b = argv[optind + 1];
	}
	else if (load_mode == LM_FILES) {
		if (argc - optind < 2) {
			printf("please specify input sequence files\n");
			return 1;
		}
		/* TODO load from file */
		return 1;
	}
	else if (load_mode == LM_SINGLE_FILE) {
		if (argc - optind < 1) {
			printf("please specify input sequence file\n");
			return 1;
		}
		/* TODO load from file */
		return 1;
	}
	args.len_a = strlen(args.seq_a);
	args.len_b = strlen(args.seq_b);

	/* Start algorithm */
	if (algorithms[algorithm].func == NULL) {
		printf("`%s` algorithm is not implemented\n",
		       algorithms[algorithm].name);
		return 1;
	}
	algorithms[algorithm].func(&args, &res);

	return 0;
}

