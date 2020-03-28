#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>//to estimate the runing time

#define NLINKS 100000000 //maximum number of edges for memory allocation, will increase if needed

typedef struct {
	unsigned long s;
	unsigned long t;
} edge;

//edge list structure:
typedef struct {
	unsigned long n;//number of nodes
	unsigned long e;//number of edges
	edge *edges;//list of edges
	unsigned long *cd;//cumulative degree cd[0]=0 length=n+1
	unsigned long *adj;//concatenated lists of neighbors of all nodes
} adjlist;

//compute the maximum of three unsigned long
unsigned long max3(unsigned long a,unsigned long b,unsigned long c){
	a=(a>b) ? a : b;
	return (a>c) ? a : c;
}

//reading the edgelist from file
adjlist* readedgelist(char* input){
	unsigned long e1=NLINKS;
	FILE *file=fopen(input,"r");
	adjlist *g=malloc(sizeof(adjlist));
	g->n=0;
	g->e=0;
	g->edges=malloc(e1*sizeof(edge));//allocate some RAM to store edges

	while (fscanf(file,"%lu %lu", &(g->edges[g->e].s), &(g->edges[g->e].t))==2) {
		g->n=max3(g->n,g->edges[g->e].s,g->edges[g->e].t);
		if (++(g->e)==e1) {//increase allocated RAM if needed
			e1+=NLINKS;
			g->edges=realloc(g->edges,e1*sizeof(edge));
		}
	}
	fclose(file);
	g->n++;
	g->edges=realloc(g->edges,g->e*sizeof(edge));
	return g;
}

bool estDouble(unsigned long noeud, unsigned long voisin, unsigned long* adjlist, unsigned long* cd2){
	unsigned long tNoeuds = cd2[noeud+1]-cd2[noeud];
	for(unsigned long j=0; j< tNoeuds; j++){
		if(adjlist[cd2[noeud]+j] == voisin){
			return true;
		}
	}
	if (voisin <= noeud) {
		unsigned long tVoisins = cd2[voisin+1]-cd2[voisin];
		for(unsigned long j=0; j< tVoisins; j++){
			if(adjlist[cd2[voisin]+j] == noeud){
				return true;
			}
		}
	}
	return false;
}

bool contient(unsigned long* visited, unsigned long noeud, unsigned long taille){
	for(unsigned long i = 0; i < taille; i++){
		if(visited[i] == noeud)
		return true;
	}
	return false;
}

//nettoyage des doublons, selfloops et bidirections
void* cleanEdges(adjlist* g){  
	unsigned long iter = 1; //itérateur sur les noeuds
	unsigned long nbVoisins = 0; //nombre de voisins du noeud iter
	unsigned long nbDoublons = 0;
	unsigned long * adjlist = calloc(NLINKS,sizeof(unsigned long)); //copie de la liste d'adjacence
	unsigned long * cd2 = calloc(NLINKS,sizeof(unsigned long));
	cd2[0] = 0;
	unsigned long comptAdj = 0; //compteur de la nouvelle liste d'adjacence
	while(iter < g->n){
		nbVoisins = g->cd[iter+1] - g->cd[iter];
		cd2[iter+1] = cd2[iter];
		for(unsigned long i = 0; i<nbVoisins; i++){
			unsigned long target = g->adj[g->cd[iter]+i]; 
			if (iter == target) { // cas selfloop
				nbDoublons++;
			}
			else if(estDouble(iter, target, adjlist, cd2)){ //cas bidirection
				nbDoublons++;
			}
			else{ //ajout dans la nouvelle adjlist et cd2
				adjlist[comptAdj] = target;
				comptAdj++;
				cd2[iter+1]++;
			}
		}
		iter++;
	}
	g->e = comptAdj;
	g->adj = adjlist;
	g->cd = cd2;
	printf("clean bidirections fini\n");
}

//building the adjacency matrix
void mkadjlist(adjlist* g){
	unsigned long i,u,v;
	unsigned long *d=calloc(g->n,sizeof(unsigned long));

	for (i=0; i <g->e; i++) {
		d[g->edges[i].s]++;
		d[g->edges[i].t]++;
	}
	g->cd=malloc((g->n+1)*sizeof(unsigned long));
	g->cd[0]=0;
	for (i=1; i < g->n+1; i++) {
		g->cd[i]=g->cd[i-1]+d[i-1];
		d[i-1]=0;
	}

	g->adj=malloc(2*g->e*sizeof(unsigned long));

	for (i=0; i < g->e; i++) {
		u=g->edges[i].s;
		v=g->edges[i].t;
		g->adj[g->cd[u] + d[u]++]=v;
		g->adj[g->cd[v] + d[v]++]=u;
	}
	free(d);
	free(g->edges);
}

//freeing memory
void free_adjlist(adjlist *g){
	free(g->cd);
	free(g->adj);
	free(g);
}

/*unsigned long bfs(adjlist* g, unsigned long* visited, unsigned long compteur, unsigned long noeud){ //ne fonctionne pas
	unsigned long nbVoisins = g->cd[noeud+1] - g->cd[noeud];

	if(!contient(visited, noeud, compteur)){
		visited[compteur] = noeud;
		compteur++;
	}

	for(unsigned long i = 0; i< nbVoisins; i++){
		unsigned long voisin = g->adj[g->cd[noeud]+i];
		if(!contient(visited, voisin, compteur)){
			visited[compteur] = voisin;
			compteur++;
		}
	}

	for(unsigned long j=0; j< nbVoisins; j++){
		unsigned long voisin = g->adj[g->cd[noeud]+j];
		printf(" j : %lu voisin : %lu\n", j, voisin);
		if(!contient(visited, voisin, compteur)){
			printf("bite\n");
			compteur = bfs(g, visited, compteur, voisin);
		}
	}
	return compteur;
}*/

int main(int argc,char** argv){
	adjlist* g;
	time_t t1,t2;

	t1=time(NULL);

	printf("Reading edgelist from file %s\n",argv[1]);
	
	g=readedgelist(argv[1]);

	printf("Number of nodes: %lu\n",g->n);
	printf("Number of edges: %lu\n",g->e);

	mkadjlist(g); //creation de la liste d'adjacence et des degrés cumulatifs
	cleanEdges(g); //nettoyage des doublons, selfloops et bidirections
	printf("New number of edges: %lu\n",g->e);
	unsigned long * visited = calloc(NLINKS, sizeof(unsigned long));
	unsigned long compteur = 0;
	//compteur = bfs(g, visited, compteur, 1);

	for(unsigned long i =0; i< compteur; i++)
	{
		printf( " visited[%d] : %lu\n", i , visited[i]);
	}
	free_adjlist(g); 
	t2=time(NULL);
	printf("- Overall time = %ldh%ldm%lds\n",(t2-t1)/3600,((t2-t1)%3600)/60,((t2-t1)%60));
	return 0;
}
