//Stub for custom BFS implementations

#include "common.h"
#include "aml.h"
#include "csr_reference.h"
#include "bitmap_reference.h"
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <stdint.h>

#ifdef DEBUGSTATS
extern int64_t nbytes_sent,nbytes_rcvd;
#endif
// two arrays holding visited VERTEX_LOCALs for current and next level
// we swap pointers each time
int *q1,*q2;
int qc,q2c; //pointer to first free element

//VISITED bitmap parameters
unsigned long *visited;
int64_t visited_size;

//global variables of CSR graph to be used inside of AM-handlers
int64_t *column;
int64_t *pred_glob;
unsigned int * rowstarts;

oned_csr_graph g;

MPI_Datatype mpi_message_type;
MPI_Request recv_req, send_req;
MPI_Status status;
int tag = 0;

typedef struct visitmsg {
	//both vertexes are VERTEX_LOCAL components as we know src and dest PEs to reconstruct VERTEX_GLOBAL
	int vloc;
	int vfrom;
	int ra;
} visitmsg;

visitmsg message;

void update(int64_t glob, int from){
	if (!TEST_VISITEDLOC(glob)) {
		SET_VISITEDLOC(glob);
		q2[q2c++] = glob;
		pred_glob[glob] = VERTEX_TO_GLOBAL(from, from);
	}
}

//user should provide this function which would be called once to do kernel 1: graph convert
void make_graph_data_structure(const tuple_graph* const tg) {
	//graph conversion, can be changed by user by replacing oned_csr.{c,h} with new graph format 
	convert_graph_to_oned_csr(tg, &g);

	column=g.column;

	//Added from reference too - Graph and Buffer Generation
	int i,j,k;
	rowstarts=g.rowstarts;

	visited_size = (g.nlocalverts + ulong_bits - 1) / ulong_bits;

	/*From the reference*/
	q1 = xmalloc(g.nlocalverts*sizeof(int)); //100% of vertexes
	q2 = xmalloc(g.nlocalverts*sizeof(int));
	for(i=0;i<g.nlocalverts;i++) q1[i]=0,q2[i]=0; //touch memory

	/* create a type for struct message */
	const int nitems=3;
	int          blocklengths[3] = {1,1,1};
	MPI_Datatype types[3] = {MPI_INT, MPI_INT, MPI_INT};
	MPI_Aint     offsets[3];

	offsets[0] = offsetof(visitmsg, vloc);
	offsets[1] = offsetof(visitmsg, vfrom);
	offsets[2] = offsetof(visitmsg, ra);

	MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpi_message_type);
	MPI_Type_commit(&mpi_message_type);
  	/*End struct initialization*/

	/*End*/

	visited = xmalloc(visited_size*sizeof(unsigned long));
	//user code to allocate other buffers for bfs
}

int check = 0;
int active_recv  = 0;
int done;

#define CHECK_REQS \
  do{ \
  	while (active_recv) { \
  		int flag; \
    	MPI_Status st; \
    	MPI_Test(&recv_req, &flag, &st); \
      	if (flag) { \
      		active_recv = 0; \
      		int count; \
        	MPI_Get_count(&st, mpi_message_type, &count); \
        	if(count==0){\
				done++;\
        	}\
        	else{\
		 		if (!TEST_VISITEDLOC(message.vloc)) { \
					SET_VISITEDLOC(message.vloc); \
					q2[q2c++] = message.vloc; \
					pred_glob[message.vloc] = VERTEX_TO_GLOBAL(message.ra, message.vfrom); \
				}\
			} \
			if (done < size) { \
		      MPI_Irecv(&message, 1, mpi_message_type, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &recv_req); \
		      active_recv = 1; \
		    } \
		} else break; \
	}\
    if(check) {MPI_Status st;int flag; MPI_Test(&send_req, &flag, &st); if(flag) check=0;} \
  }while(0)

//user should provide this function which would be called several times to do kernel 2: breadth first search
//pred[] should be root for root, -1 for unrechable vertices
//prior to calling run_bfs pred is set to -1 by calling clean_pred
void run_bfs(int64_t root, int64_t* pred) {
	int64_t nvisited;
	long sum, global_sum;
	unsigned int i,j,k,lvl=1;
	pred_glob=pred;
	int owner;

	//user code to do bfs
	
	//get my rank, then we have two queue (the current one and the next level one)

	CLEAN_VISITED();

	qc=0; sum=1; q2c=0; //qc and q2c tbd (?)
	nvisited=1;
	if(VERTEX_OWNER(root) == rank) {			//if the root belongs to the current rank
		pred[VERTEX_LOCAL(root)]=root;			//set the root visited
		SET_VISITED(root);			
		q1[0]=VERTEX_LOCAL(root);				//enqueue root (we have to send message to the neighbors)
		qc=1;									//qc cardinality of q1 
	} 

	while(1){
		done = 1;
		if (done < size){
			MPI_Irecv(&message, 1, mpi_message_type, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &recv_req);
			active_recv = 1;
		}
		for(i=0;i<qc;i++){											//for each vertex in the queue		
			CHECK_REQS;
			//check messages
			for(j=rowstarts[q1[i]]; j<rowstarts[q1[i]+1]; j++){		//for each neighbors in the vertex, based on the graph implementation 
				if(VERTEX_OWNER(COLUMN(j)) == rank){
					if (!TEST_VISITED(COLUMN(j))) {
			            SET_VISITED(COLUMN(j));
			            pred[VERTEX_LOCAL(COLUMN(j))] = VERTEX_TO_GLOBAL(rank, q1[i]);
			            q2[q2c++] = VERTEX_LOCAL(COLUMN(j));
          			}
				}
				else{
					//while(check) CHECK_REQS;
					visitmsg m = {VERTEX_LOCAL(COLUMN(j)), q1[i], rank};
					MPI_Isend(&m, 1, mpi_message_type, VERTEX_OWNER(COLUMN(j)), tag, MPI_COMM_WORLD, &send_req);
					check = 1;
					while (check) CHECK_REQS;
				}
				//send_visit(COLUMN(j),q1[i]);						
      		}
      	 }
      	visitmsg m;
      	for(int ra = 0; ra < size; ra++){
      		if(ra != rank){
	      		MPI_Isend(&m, 0, mpi_message_type, ra, tag, MPI_COMM_WORLD, &send_req);
	      		check = 1;
      		}
      		
      	}
      	
      	
      	while (done < size) CHECK_REQS;

		qc=q2c;int *tmp=q1;q1=q2;q2=tmp;
		sum=qc;
		global_sum = 0;
		MPI_Allreduce(&sum, &global_sum, 1, MPI_LONG_LONG, MPI_SUM, MPI_COMM_WORLD);
		if(global_sum == 0){
			/*if(1){
				printf("Rank %d, root %d\n", rank, root);
				for(int j=0; j<g.nlocalverts; j++) printf("%d ", pred_glob[j]);
					printf("\n");
			}*/
			
			//printf("Global sum = %li\n\n", global_sum);
			//printf("Done\n");

			//MPI_Barrier(MPI_COMM_WORLD);
			break;
		}
		
		nvisited+=sum;
		q2c=0;
		//synchronize
	}

}

//we need edge count to calculate teps. Validation will check if this count is correct
//user should change this function if another format (not standart CRS) used
void get_edge_count_for_teps(int64_t* edge_visit_count) {
	long i,j;
	long edge_count=0;
	for(i=0;i<g.nlocalverts;i++)
		if(pred_glob[i]!=-1) {
			for(j=g.rowstarts[i];j<g.rowstarts[i+1];j++)
				if(COLUMN(j)<=VERTEX_TO_GLOBAL(my_pe(),i))
					edge_count++;
		}
	aml_long_allsum(&edge_count);
	*edge_visit_count=edge_count;
}

//user provided function to initialize predecessor array to whatevere value user needs
void clean_pred(int64_t* pred) {
	int i;
	for(i=0;i<g.nlocalverts;i++) pred[i]=-1;
}

//user provided function to be called once graph is no longer needed
void free_graph_data_structure(void) {
	free_oned_csr_graph(&g);
	free(visited);
	free(q1); free(q2);
}

//user should change is function if distribution(and counts) of vertices is changed
size_t get_nlocalverts_for_pred(void) {
	return g.nlocalverts;
}
