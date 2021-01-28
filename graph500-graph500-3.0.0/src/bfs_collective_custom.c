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

struct visitmsg* send_buf;
struct visitmsg* recv_buf;
long send_size = 0;
long recv_size = 0;
int alloc_size;

inline void set_zero(int *send_counts_p, int *send_counts_2_p, int *send_disps_p, int *recv_counts_p, int *recv_disps_p) {
    memset(recv_counts_p, 0, alloc_size);
    memset(recv_disps_p, 0, alloc_size);
    memset(send_counts_p, 0, alloc_size);
    memset(send_disps_p, 0, alloc_size);
    memset(send_counts_2_p, 0, alloc_size);
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
    alloc_size = sizeof(int) * size;
    int send_counts[alloc_size];
    int send_disps[alloc_size];
    // inform each porcessor, how many vertices to expect in the next communication
    int recv_counts[alloc_size];
    int recv_disps[alloc_size];
    int send_counts_2[alloc_size];

    set_zero(send_counts, send_counts_2, send_disps, recv_counts, recv_disps);
	
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

		for(i=0;i<qc;i++){
			for(j=rowstarts[q1[i]];j<rowstarts[q1[i]+1];j++){
				send_counts_2[VERTEX_OWNER(COLUMN(j))]++; //fill the send count size;
			}
            send_size += rowstarts[q1[i]+1] - rowstarts[q1[i]];
      	}

      	//create a buff of send_size so that we can fill it with the messages
        send_buf = (struct visitmsg*)calloc(send_size, sizeof(struct visitmsg));

		for(i=0;i<qc;i++){
			for(j=rowstarts[q1[i]];j<rowstarts[q1[i]+1];j++){
				int offset = 0; int owner = VERTEX_OWNER(COLUMN(j));
				for(int l=0; l<owner; l++){
					offset+=send_counts_2[l]; //calculate the offset in which we set the message
				}
				visitmsg m = {VERTEX_LOCAL(COLUMN(j)), q1[i], rank};
				send_buf[(send_counts[owner]+offset)] = m;
				send_counts[owner]++; //fill the send count size;
			}

      	}

        //what we will expect to receive from each processors?
      	MPI_Alltoall(send_counts, 1, MPI_INT, recv_counts, 1, MPI_INT, MPI_COMM_WORLD);

        recv_disps[0] = 0;
        send_disps[0] = 0;
        recv_size += recv_counts[0];
        for(i=1;i<size;++i){
            recv_disps[i] = recv_disps[i-1]+recv_counts[i-1];
            send_disps[i] = send_disps[i-1]+send_counts[i-1];
            recv_size += recv_counts[i];
        }
        
        recv_buf = (struct visitmsg*)calloc(recv_size, sizeof(struct visitmsg));
		
        MPI_Alltoallv(send_buf, send_counts, send_disps, mpi_message_type, recv_buf, recv_counts, recv_disps, mpi_message_type, MPI_COMM_WORLD);

      	//now we face with local vertex only, thus ...
      	for(i=0; i<recv_size; ++i){
      	    int vloc = recv_buf[i].vloc;
  			if (!TEST_VISITEDLOC(vloc)) {
					SET_VISITEDLOC(vloc);
					q2[q2c++] = vloc;
					pred_glob[vloc] = VERTEX_TO_GLOBAL(recv_buf[i].ra, recv_buf[i].vfrom);
				}
      	}

        free(recv_buf);
        free(send_buf);
        recv_size = 0;

      	MPI_Barrier(MPI_COMM_WORLD);

		qc=q2c;int *tmp=q1;q1=q2;q2=tmp;
		sum=qc;
		global_sum = 0;
		MPI_Allreduce(&sum, &global_sum, 1, MPI_LONG_LONG, MPI_SUM, MPI_COMM_WORLD);

		if(global_sum==0){
			break;
		}

        set_zero(send_counts, send_counts_2, send_disps, recv_counts, recv_disps);
        send_size = 0;
		nvisited+=sum;
		q2c=0;
	}
//    free_memory();
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
