#ifndef _matrix_graph_h_
#define _matrix_graph_h_

typedef struct matrix_graph
{
	/* data */
	//int *diags[3];
	int num_graph; //le numero du graphe || de la sous matrice
	int w_x; //width
	int h_y;
	int ** diags;
	int * graph_depence; //autre graphe dont ce graphe depend
	int * graph_omnipotence; //autre graphe QUI dependent de ce graphe (possede une omnipotence sur)
	int is_ok_d; // si tout les dependance sont fini = 1 sinon 0
	int is_ok_o;// indique que le graphe a effectuer sont calcule et est pret a etre transmis

} matrix_graph_t;

void matrix_graph_init(matrix_frag_t *mg, int w , int h, int num_grag, int * graph_depence , int * graph_omnipotence);

void matrix_graph_is_ok




#endif