#include "matrix_graph.h"

void matrix_graph_init(matrix_frag_t *mg, int w , int h, int num_graph , int * graph_depence , int * graph_omnipotence)
{
	mg->w_x = w;
	mg->h_y = h;
	mg->num_graph = num_graph;
	mg->graph_depence = graph_depence//pas sur je pense qu'il y a un malloc a faire et un for pour recopie
	mg->graph_omnipotence = graph_omnipotence //idem
	mg->is_ok_d = 0;
	mg_is_ok_o =0;
	//trouver le nombre de diagonals en fonction de w et h et trouver la taille des diagonale
	//mg->diag = malloc();
}

