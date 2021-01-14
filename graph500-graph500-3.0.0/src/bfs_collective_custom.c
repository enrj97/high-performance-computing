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
	int ra; // send rank
} visitmsg;

visitmsg message;

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

int *send_counts; // number of neighbors that will be sent to each proc
//mpi_message_type* send_buf; // contains the data that need to be sent to each proc
int *send_disps; // start index of the data that has to be sent to each proc
visitmsg* send_buf;
int *recv_counts; // number of neighbors to be received from each proc
//mpi_message_type* recv_buf; // to store the data to be received from each proc
int *recv_disps; // start index of storage for data received from each proc

long send_size;
long recv_size; 

void initialize_list(){

	// count the total number of vertices that need to be sent to each proc
    // store the number of vertices that need to be sent to each proc
    send_counts = (int *)malloc(sizeof(int) * size);
    send_disps = (int *)malloc(sizeof(int) * size);

    // inform each porcessor, how many vertices to expect in the next communication
    recv_counts = (int *)malloc(sizeof(int) * size);
    recv_disps = (int *)malloc(sizeof(int) * size);

	for(int i=0; i<size; ++i){
		recv_counts[i] = 0;
    	recv_disps[i] = 0;
    	send_counts[i] = 0;
    	send_disps[i] = 0;
    }

    recv_size = 0;
    send_size = 0;

    return;
} 


//user should provide this function which would be called several times to do kernel 2: breadth first search
//pred[] should be root for root, -1 for unrechable vertices
//prior to calling run_bfs pred is set to -1 by calling clean_pred
void run_bfs(int64_t root, int64_t* pred) {
	int64_t nvisited;
	long sum, global_sum;
	unsigned int i,j,k,lvl=1;
	pred_glob=pred;
	int owner;

	
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
		MPI_Barrier(MPI_COMM_WORLD);
		
		initialize_list(); //init lists

		for(i=0;i<qc;i++)
			for(j=rowstarts[q1[i]];j<rowstarts[q1[i]+1];j++){
				send_counts[VERTEX_OWNER(COLUMN(j))]++; //fill the send count size;
				send_size++;
      	}

      	//create a buff of send_size so that we can fill it with the messages
        send_buf = (visitmsg*)malloc(sizeof(visitmsg) * send_size);

        initialize_list(); //init lists

		for(i=0;i<qc;i++)
			for(j=rowstarts[q1[i]];j<rowstarts[q1[i]+1];j++){
				send_counts[VERTEX_OWNER(COLUMN(j))]++; //fill the send count size;
				send_size++;
      	}


        //what we will expect to receive from each processors?
      	MPI_Alltoall(send_counts, 1, MPI_INT, recv_counts, 1, MPI_INT, MPI_COMM_WORLD);
      	
      	printf("Rank: %d dim: %ld\n", rank, send_size);
      	for(i=0; i<size; ++i) printf("%d ", recv_counts[i]);
      	printf("\n");

      	MPI_Barrier(MPI_COMM_WORLD);
      	sleep(1);
		break;

		qc=q2c;int *tmp=q1;q1=q2;q2=tmp;
		sum=qc;
		global_sum = 0;
		MPI_Allreduce(&sum, &global_sum, 1, MPI_LONG_LONG, MPI_SUM, MPI_COMM_WORLD);

		nvisited+=sum;
		q2c=0;
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
