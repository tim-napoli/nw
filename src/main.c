#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "common.h"
#include "alignment.h"

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
		&nw
	},
	{
		"parallelized",
		"parallelized iterative implementation",
		&nw_omp
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
	       " -F, --Fingle 		sequences are from a single file (two lines)\n"
	       " -R, --Random <size>    generate random sequences of given size\n"
	       " -S, --Seed <seed>	use given seed for random numbers generation\n"
	       " -u			use hard drive memory\n"
	       " -a, --algorithm <algo>	use given algorithm for alignment\n"
	       " -t, --time		print algorithm run time\n"
	       " -c, --core 		specify the number of cores (iterative only)\n"
	       " -v, --validate <file>	validate computed alignment using `file`\n"
	       " -o, --output <file>	print alignment(s) to a file instead of stdout\n"
	       " -m, --max <max>	max alignments to print\n\n"

	       "algorithm list:\n"
	      );

	print_algo_list();
}

/* Load mode of sequences */
enum {
	LM_ARGUMENTS,
	LM_FILES,
	LM_SINGLE_FILE,
	LM_RANDOM,
};

int allocate_matrix(const algo_arg_t* args,
		    matrix_t* score_matrix,
		    matrix_t* move_matrix,
		    int use_file)
{
	if (matrix_init(score_matrix,
			args->len_a + 1, args->len_b + 1,
			sizeof(int), use_file))
	{
		printf("couldn't allocate score matrix\n");
		return 1;
	}
	if (matrix_init(move_matrix,
			args->len_a + 1, args->len_b + 1,
			sizeof(char), use_file))
	{
		matrix_wipe(score_matrix);
		printf("couldn't allocate move matrix\n");
		return 1;
	}
	return 0;
}


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
	int random_size = 0;
	int seed = 0;
	int use_file = 0;
	int bound = -1;
	algo_arg_t args;
	algo_res_t res;

	/* parsing options */
	char opt_c = 0;
	while ((opt_c = getopt(argc, argv, "hsfFR:S:tua:c:v:o:b:m:")) > 0) {
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

		    case 'F':
			load_mode = LM_SINGLE_FILE;
			break;

		    case 'R':
			load_mode = LM_RANDOM;
			if (sscanf(optarg, "%d", &random_size) != 1) {
				printf("invalid sequences size\n");
				return 1;
			}
			break;

		    case 'S':
			if (sscanf(optarg, "%d", &seed) != 1) {
				printf("invalid seed\n");
				return 1;
			}
			break;
		
		    case 'u':
		    	use_file = 1;
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
			
		    case 'm':
			if (sscanf(optarg, "%d", &bound) != 1) {
				printf("invalid max parameter\n");
				return 1;
			}
			break;
		}
	}

	/* Check sequences are given */
	if (load_mode == LM_SINGLE_FILE && argc - optind < 1) {
		printf("please specify single input sequences file\n");
		return 1;
	}
	else if (load_mode == LM_FILES && argc - optind < 2) {
		printf("please specify input sequence files\n");
		return 1;
	}
	else if (load_mode == LM_ARGUMENTS && argc - optind < 2) {
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
	else if (load_mode == LM_RANDOM) {
		srand(seed);
		args.seq_a = malloc(random_size + 1);
		args.seq_b = malloc(random_size + 1);
		args.seq_a[random_size] = '\0';
		args.seq_b[random_size] = '\0';
		for (int i = 0; i < random_size; i++) {
			args.seq_a[i] = 'A' + rand() % ('Z' - 'A'); 
			args.seq_b[i] = 'A' + rand() % ('Z' - 'A'); 
		}
	}
	args.len_a = strlen(args.seq_a);
	args.len_b = strlen(args.seq_b);

	/* Start algorithm */
	if (algorithms[algorithm].func == NULL) {
		printf("`%s` algorithm is not implemented\n",
		       algorithms[algorithm].name);
		return 1;
	}

	matrix_t score_matrix;
	matrix_t move_matrix;

	if (allocate_matrix(&args, &score_matrix, &move_matrix, use_file)) {
		return 1;
	}

	if (algorithms[algorithm].func(&args, &res,
				       &score_matrix, &move_matrix))
	{
		printf("algorithm failure\n");
		return 1;
	}

#if 0
	print_score_matrix(&args, &score_matrix);
	print_move_matrix(&args, &move_matrix);
#endif

	matrix_wipe(&score_matrix);

	if (bound != 0) {
		alignment_t* alignments = NULL;
		int nalignments = compute_alignments(&args, &move_matrix, &alignments,
						     bound);
		if (nalignments <= 0) {
			printf("Error during alignment creation\n");
			matrix_wipe(&move_matrix);
			return 1;
		}

		for (int i = 0; i < nalignments; i++) {
			printf("alignment %d:\n", i + 1);
			print_alignment(alignments + i);
			alignment_wipe(alignments + i);
		}
		free(alignments);
	}

	matrix_wipe(&move_matrix);

	return 0;
}

