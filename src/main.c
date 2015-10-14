#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "common.h"
#include "alignment.h"
#include "bench.h"
#include "validate.h"

int verbose = 0;

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
		    matrix_t* move_matrix,
		    int use_file)
{
	if (matrix_init(move_matrix,
			args->len_a + 1, args->len_b + 1,
			sizeof(char), use_file))
	{
		printf("couldn't allocate move matrix\n");
		return 1;
	}
	return 0;
}

static size_t __get_file_length(FILE* f) {
	long init_pos = ftell(f);
	fseek(f, 0, SEEK_END);
	long end_pos = ftell(f);
	fseek(f, init_pos, SEEK_SET);
	return end_pos;
}

int load_sequences_files(const char* path_a, const char* path_b,
			 algo_arg_t* args)
{
	FILE* f_a = fopen(path_a, "r");
	if (!f_a) {
		printf("couldn't open sequence file %s\n", path_a);
		goto error;
	}
	FILE* f_b = fopen(path_b, "r");
	if (!f_b) {
		printf("coundn't open sequence file %s\n", path_b);
		goto error_1;	
	}

	args->len_a = __get_file_length(f_a);
	args->len_b = __get_file_length(f_b);

	args->seq_a = malloc(args->len_a + 1);
	if (!args->seq_a) {
		printf("couldn't allocate sequence a\n");
		goto error_2; 
	}
	args->seq_b = malloc(args->len_b + 1);
	if (!args->seq_b) {
		printf("couldn't allocate sequence b\n");
		goto error_3;
	}

	if (fread(args->seq_a, args->len_a, 1, f_a) != 1) {
		printf("couldn't read file %s\n", path_a);
		goto error_4;
	}
	if (fread(args->seq_b, args->len_b, 1, f_b) != 1) {
		printf("couldn't read file %s\n", path_b);
		goto error_4;
	}

	args->seq_a[args->len_a] = '\0';
	args->seq_b[args->len_b] = '\0';

	return 0;

    error_4:
	free(args->seq_b);
    error_3:
	free(args->seq_a);
    error_2:
	fclose(f_b);
    error_1:
	fclose(f_a);
    error:
	return 1;
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
	int random_size = 0;
	int seed = 0;
	int use_file = 0;
	int bound = -1;
	algo_arg_t args;
	algo_res_t res;
	bench_t bench_algo;
	bench_t bench_align;

	/* parsing options */
	char opt_c = 0;
	while ((opt_c = getopt(argc, argv, "hsfFR:S:tua:c:v:o:b:m:V")) > 0) {
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

		    case 'V':
			verbose = 1;
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
		args.seq_a = argv[optind];
		args.seq_b = argv[optind + 1];
	}
	else if (load_mode == LM_FILES) {
		if (load_sequences_files(argv[optind], argv[optind + 1],
					 &args))
		{
			printf("couldn't load sequences\n");
			return 1;
		}
	}
	else if (load_mode == LM_SINGLE_FILE) {
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

	matrix_t move_matrix;

	if (allocate_matrix(&args, &move_matrix, use_file)) {
		return 1;
	}

	if (do_bench) {
		bench_start(&bench_algo, "algorithm runtime");
	}

	VERBOSE_FMT("start %s algorithm.\n", algorithms[algorithm].name);
	if (algorithms[algorithm].func(&args, &res,
				       &move_matrix))
	{
		printf("algorithm failure\n");
		return 1;
	}
	
	if (do_bench) {
		bench_end(&bench_algo);
	}

#if 0
	print_score_matrix(&args, &score_matrix);
	print_move_matrix(&args, &move_matrix);
#endif

	if (do_bench) {
		bench_start(&bench_align, "alignment runtime");
	}

	/* Alignment */
	if (bound != 0) {
		VERBOSE_FMT("retrieving alignments (max %d)\n", bound);
		alignment_t* alignments = NULL;
		int nalignments = compute_alignments(&args, &move_matrix, &alignments,
						     bound);
		if (nalignments <= 0) {
			printf("Error during alignment creation\n");
			matrix_wipe(&move_matrix);
			return 1;
		}
		if (do_validation)
		{
			if (validate(validation_file,alignments,nalignments)) {
				printf("you are a fucking genius !\n");
			}
			else {
				printf("you are a fucking retard !\n");
			}
		}
		
		for (int i = 0; i < nalignments; i++) {
			printf("alignment %d:\n", i + 1);

			if (i == 0)
			{
				int w = score_alignment(alignments + i);
				printf("alignment score: %d\n",w);
			}
			print_alignment(alignments + i);

			alignment_wipe(alignments + i);
		}
		free(alignments);
	}

	if (do_bench) {
		bench_end(&bench_align);
	}

	if (do_bench) {
		printf("algorithm runtime: %f\n", bench_diff_s(&bench_algo));
		printf("alignment runtime: %f\n", bench_diff_s(&bench_align));
	}

	matrix_wipe(&move_matrix);

	return 0;
}

