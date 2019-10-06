#include <stdio.h>
#include <iostream>
#include "CHESS.h"
#include "Constants.h"

using std::cout;
using std::endl;

int construct_routing_topology(struct patch_object	*patch,
					  		  char *routing_filename,char *FlowTableName,
							  int num_patches)
													  
{
	/*--------------------------------------------------------------*/
	/*	Local function definition.									*/
	/*--------------------------------------------------------------*/
	struct patch_object *find_patch(int,struct patch_object *,  int);
	
	int assign_neighbours (struct neighbour_object *,
		int,
		struct patch_object *,
		FILE *, int);
	
	void *alloc(size_t, char *, char *);

	void match_patch (int,int,struct patch_object *, int);
	
	/*--------------------------------------------------------------*/
	/*	Local variable definition.									*/
	/*--------------------------------------------------------------*/
	int		i,j;
	int		total_patches, num_neighbours;
	int		patch_ID;
	int		drainage_type;
	double	x,y,z, area, gamma, width;
	FILE	*routing_file;
	struct	patch_object	*stream;
	
	/*--------------------------------------------------------------*/
	/*  Try to open the routing file in read mode.                    */
	/*--------------------------------------------------------------*/
   strcat(routing_filename,FlowTableName);
	if ( (routing_file = fopen(routing_filename,"r")) == NULL ){
		fprintf(stderr,"FATAL ERROR:  Cannot open routing file %s\n",
			routing_filename);
		exit(0);
	} /*end if*/
	fscanf(routing_file,"%d",&total_patches);
	
	if (total_patches != num_patches){
		cout << "the total number of patches does not match the input number of patches" << endl;
		return (0);
	}

	//the following is added to match input patch id with flow_table_patch id
	for (i=0; i< num_patches; ++i) {
		fscanf(routing_file,"%d %lf %lf %lf %lf %d %lf %d",
			&patch_ID,
			&x,&y,&z,
			&area,
			&drainage_type,
			&gamma,
			&num_neighbours);
		    
	    match_patch(i,patch_ID,patch,num_patches);

		for (j=0; j< num_neighbours; j++) 
			fscanf(routing_file,"%d %lf",
				&patch_ID,
				&gamma);
	}

	rewind(routing_file); //rewind file pointer to the first record
	fscanf(routing_file, "%d", &total_patches);

	/*--------------------------------------------------------------*/
	/*	Read in  each patch record and find it		.				*/
	/*	if it is a stream add it to the basin level routing list	*/
	/*	otherwise add it to the hillslope level routing list		*/
	/*--------------------------------------------------------------*/
	for (i=0; i< num_patches; ++i) {
		fscanf(routing_file,"%d %lf %lf %lf %lf %d %lf %d",
			&patch_ID,
			&x,&y,&z,
			&area,
			&drainage_type,
			&gamma,
			&num_neighbours);

		patch[i].num_innundation_depths=1;
		patch[i].num_neighbours = num_neighbours;
		patch[i].gamma = gamma;
		patch[i].acc_area = area;
		patch[i].critical_depth = NULLVAL;
		patch[i].drainage_type = drainage_type;
		if ((patch[i].drainage_type != STREAM) && (patch[i].gamma < ZERO))
			patch[i].drainage_type = STREAM;

		/*--------------------------------------------------------------*/
		/*  Allocate neighbour array									*/
		/*--------------------------------------------------------------*/
		patch[i].neighbours = (struct neighbour_object *)alloc(num_neighbours *
		sizeof(struct neighbour_object), "neighbours", "construct_routing_topology");
      	patch[i].num_neighbours = assign_neighbours(patch[i].neighbours, num_neighbours, patch, routing_file,num_patches);
		
			
		if (drainage_type == 2) {
			fscanf(routing_file,"%d %lf",
				&patch_ID,
				&width);
			patch[i].road_cut_depth = width * tan(patch[i].slope);
			stream = find_patch(patch_ID, patch, num_patches);
			patch[i].next_stream = stream;
		}
	}
	
	printf("\nFinishing construct_routing_topology.cpp  \n");
    return (total_patches);
} 

