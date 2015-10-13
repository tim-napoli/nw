#include "validate.h"

int validate(char * validation_file, alignment_t* alignments,int nalignments)
{
	FILE * fichier = NULL;
	fichier = fopen(validation_file,"r");
	if (fichier == NULL)
	{
		printf("error open validation_file \n");
		return 0;
	}
	int taille = 1;
	while ('\n' != fgetc(fichier))
		taille++;
	fseek(fichier, 0, SEEK_SET);
	char ligne1[taille]; //faire malloc si trop grand
	char ligne2[taille];

	fgets(ligne1,taille+1,fichier);
	fgets(ligne2,taille+1,fichier);
	ligne1[taille - 1] = '\0';
	ligne2[taille - 1] = '\0';
	fclose(fichier);
	//printf("%s\n",ligne1);
	//printf("%s\n",ligne2);
	for (int i=0 ; i < nalignments ; ++i)
	{
		if ( (strcmp(ligne1,alignments[i].up)==0) && (strcmp(ligne2,alignments[i].down) == 0) )
			return 1;
	}
	return 0;

	
}