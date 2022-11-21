https://powcoder.com
代写代考加微信 powcoder
Assignment Project Exam Help
Add WeChat powcoder
// The MPI-parallelized version of the Game of Life.

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <stdarg.h>
#include <string.h>

#define TAG_PARAMS 0
#define TAG_DATA   1
#define TAG_TOP    2
#define TAG_BOTTOM 3
#define TAG_RESULT 4

#define DEBUG

void debug(const char* format, ...) {
#ifdef DEBUG
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
#endif
}

void error(const char* msg) {
	fprintf(stderr, "%s", msg);
	exit(-1);
}

struct Params_t {
	int w;
	int h;
	int iterations;
	int last_id;
};

void main_process(int argc, char** argv, int n_tasks);
void worker_process(int id);

int main(int argc, char** argv) {
	if (MPI_Init(&argc, &argv) != MPI_SUCCESS) {
		error("MPI initialization failed\n");
	}

	int n_tasks, id;
	MPI_Comm_size(MPI_COMM_WORLD, &n_tasks);
	MPI_Comm_rank(MPI_COMM_WORLD, &id);

	if (n_tasks < 2) {
		MPI_Finalize();
		error("You need at least 2 processors to run this program\n");
	}

	if (id == 0) {
		main_process(argc, argv, n_tasks);
	} else {
		worker_process(id);
	}

	MPI_Finalize();
}

void main_process(int argc, char** argv, int n_tasks) {
	debug("[Main] started\n");

	// parse arguments
	if (argc != 5) {
		error("life <input file name> <# of generations> <X_limit> <Y_limit>\n");
	}

	char* fname = argv[1];
	int iterations, w, h;
	if (sscanf(argv[2], "%d", &iterations) != 1 || sscanf(argv[3], "%d", &h) != 1 || sscanf(argv[4], "%d", &w) != 1
			|| iterations < 1 || w < 2 || h < 2) {
		error("Invalid input\n");
	}

	FILE* fd = fopen(fname, "r");
	if (fd == NULL) {
		error("Could not open data file\n");
	}

	// allocate board
	int board_size = (w + 2) * (h + 2) * sizeof(char);
	char* board = (char*) malloc(board_size);
	memset(board, 0, board_size);

#define BOARD(board, x, y) board[((y)+1) * (w+2) + (x)+1]

	// initialize from file
	int x, y;
	while (fscanf(fd, "%d %d", &y, &x) != EOF) {
		if (x < 0 || x >= w || y < 0 || y >= h) {
			error("Invalid data\n");
		}
		BOARD(board, x, y)= 1;
	}
	fclose(fd);

	//// start workers
	// limit the number of workers to number of tasks
	// each worker processes either (h / num_workers) or (h / num_workers + 1) rows
	int num_workers = h > (n_tasks - 1) ? (n_tasks - 1) : h;
	int* h_workers = (int*) malloc((n_tasks + 1) * sizeof(int));
	for (int id = 1; id <= n_tasks; ++id) {
		if (id > num_workers) {
			h_workers[id] = 0;
		} else {
			h_workers[id] = h / num_workers + ((id <= h % num_workers) ? 1 : 0);
		}
	}
	// send params
	for (int id = 1; id < n_tasks; ++id) {
		struct Params_t params;
		params.w = w;
		params.h = h_workers[id];
		params.iterations = iterations;
		params.last_id = num_workers;

		MPI_Send(&params, sizeof(params), MPI_UNSIGNED_CHAR, id, TAG_PARAMS, MPI_COMM_WORLD);

		debug("[Main] sent params to %d\n", id);
	}
	// send data
	int y_start = 0;
	for (int id = 1; id <= num_workers; ++id) {
		int h_worker = h_workers[id];
		MPI_Send(&BOARD(board, -1, y_start), h_worker * (w + 2), MPI_UNSIGNED_CHAR, id, TAG_DATA, MPI_COMM_WORLD);
		 // MPI_SEND(message, 12, MPI_CHARACTER, i, tag, MPI_COMM_WORLD, ierror) 
		y_start += h_worker;

		debug("[Main] sent data to %d\n", id);
	}

	//// receive results
	MPI_Status status;
	y_start = 0;
	for (int id = 1; id <= num_workers; ++id) {
		int h_worker = h_workers[id];
		MPI_Recv(&BOARD(board, -1, y_start), h_worker * (w + 2), MPI_UNSIGNED_CHAR, id, TAG_RESULT, MPI_COMM_WORLD,
				&status);
		y_start += h_worker;

		debug("[Main] received data from %d\n", id);
	}

	//// print
	for (y = 0; y < h; ++y) {
		for (x = 0; x < w; ++x) {
			if (BOARD(board, x, y)== 1) {
				printf("%d %d\n", y, x);
			}
		}
	}

	//// free memory
	free(board);
	free(h_workers);
}

void worker_process(int id) {
	debug("[%d] started\n", id);
	struct Params_t params;

	//// receive params
	// TODO your code here

	debug("[%d] received params from main\n", id);

	if (params.h == 0) {
		debug("[%d] exit (nothing to do)\n", id);
		return;
	}
	debug("[%d] has %d rows\n", id, params.h);

	//// allocate boards
	char* boards[2];
	// TODO your code here
	// HINT: take a look at life_seq.c

	//// receive board from main
	// TODO your code here
	// HINT: take a look at main_process to find out the amount of data it sends

	debug("[%d] received data from main\n", id);

	//// computation
	for (int iter = 0; iter < params.iterations; ++iter) {
		debug("[%d] iter %d\n", id, iter);

	// TODO your code here
	// HINT: send your top and bottom rows to appropriate workers,
	//       and also update your boundary rows from them
	// HINT: if you follow the idea in life_seq.c this part is straightforward

	}
	debug("[%d] finished computation\n", id);

	//// send board to main
	// TODO your code here
	debug("[%d] sent results to main\n", id);

	free(boards[0]);
	free(boards[1]);
}
