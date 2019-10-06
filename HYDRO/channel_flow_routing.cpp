#include <stdio.h>
#include <iostream>
#include <math.h>
#include "CHESS.h"
#include "Constants.h"

using std::cout;
using std::endl;

#define round(x) ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))

void  channel_flow_routing(patch_object *patch, struct reservoir_object reservoir, struct command_line_object *command_line,
						 struct	date current_date,int num_patches)
{
	double  compute_z_final(
	int,
	double,
	double,
	double,
	double,
	double);

	struct patch_object	*neigh; 
	int i,j,k=0,kk=0;
	static double route_to_streamflow=0,route_to_baseflow=0;
	double sumstreamflow=0,sumbaseflow=0;
	static double volume=0;
	static bool startDischarge=false;
	double Qout=0.,water=0.;
	double surface_Qout=0.,return_flow=0.;

	if(command_line->routing_flag ==1){
		for (i=0;i<num_patches;i++){
			if(patch[i].drainage_type==STREAM){

				//For river channel, soil should be mostly saturated. So, we assume one-tenth of
				//surface and subsurface flow are used to reduce soil saturation deficit in channel
                 			
				double partial_surface    = patch[i].surface_Qout*1./100.; //20
				double partial_subsurface = patch[i].subsurface_Qout*1./100.;//20

				if(patch[i].sat_deficit>0.){
					double min_surface=min(partial_surface,patch[i].sat_deficit);
					patch[i].sat_deficit -= min_surface;
					patch[i].surface_Qout-= min_surface;
				}

				if(patch[i].sat_deficit>0.){
					double min_subsurface=min(partial_subsurface,patch[i].sat_deficit);
					patch[i].sat_deficit    -=min_subsurface;
					patch[i].subsurface_Qout-=min_subsurface;
				}				
				/*
				patch[i].unsat_storage=0.;
				patch[i].rz_storage=0.;

				if(patch[i].sat_deficit <= 0.){
					patch[i].surface_Qout     -= patch[i].sat_deficit;
					patch[i].sat_deficit=0.;
				}
				else{
					if(patch[i].surface_Qout>=patch[i].sat_deficit) {
						patch[i].surface_Qout -= patch[i].sat_deficit;
						patch[i].sat_deficit=0.;
					}
					else {
						patch[i].sat_deficit-=patch[i].surface_Qout;
						patch[i].surface_Qout=0.;
									
						//if(patch[i].subsurface_Qout>=patch[i].sat_deficit) {
						//	patch[i].subsurface_Qout -= patch[i].sat_deficit;
						//	patch[i].sat_deficit =0.;
						//}
						//else{
						//	patch[i].sat_deficit -= patch[i].subsurface_Qout;
						//	patch[i].subsurface_Qout=0.;
						//}
					}
				}
				*/		
				patch[i].sat_deficit_z = compute_z_final(
					command_line->verbose_flag,
					patch[i].soil_defaults->porosity_0,
					patch[i].soil_defaults->porosity_decay,
					patch[i].soil_defaults->soil_depth,
					0.0,
					-1.0 * patch[i].sat_deficit);

				//routing the river flow
				for(j=0;j<patch[i].num_neighbours; j++){
					neigh = patch[i].neighbours[j].patch;
					if(neigh->drainage_type == STREAM ){ 
						neigh->surface_Qout     +=  patch[i].neighbours[j].gamma *patch[i].surface_Qout; //
						neigh->subsurface_Qout  +=  patch[i].neighbours[j].gamma *patch[i].subsurface_Qout; //
						neigh->gw.Qout          +=  patch[i].neighbours[j].gamma *patch[i].gw.Qout; //added for ground water
						neigh->DON_loss += patch[i].neighbours[j].gamma*patch[i].DON_loss;//we only route DOC and DON once
						neigh->DOC_loss += patch[i].neighbours[j].gamma*patch[i].DOC_loss;
						neigh->subsurface_leach_NH4_out += patch[i].neighbours[j].gamma*patch[i].subsurface_leach_NH4_out;
						neigh->surface_leach_NH4_out    += patch[i].neighbours[j].gamma*patch[i].surface_leach_NH4_out;
						neigh->subsurface_leach_NO3_out += patch[i].neighbours[j].gamma*patch[i].subsurface_leach_NO3_out;
						neigh->surface_leach_NO3_out    += patch[i].neighbours[j].gamma*patch[i].surface_leach_NO3_out;
	  				}												  
				}
			}
		}
	}

	/*
	if(command_line->routing_flag ==1){
		for (i=0;i<num_patches;i++){
			if(patch[i].ID ==15065) {//reservoir.ID
				kk=i;
				break;
			}
		}
		//printf("kk is %d \n",kk);

		//First，route stream water to the reservoir
		for (i=0;i<kk;i++){
			if(patch[i].drainage_type==STREAM){
				for(j=0;j<patch[i].num_neighbours; j++){
					neigh = patch[i].neighbours[j].patch;
					if(neigh->drainage_type == STREAM ){ 
						if(neigh->ID==reservoir.ID) {
							neigh->surface_Qout     += (patch[i].neighbours[j].gamma * patch[i].surface_Qout
														+ volume);
							neigh->subsurface_Qout  += patch[i].neighbours[j].gamma * patch[i].subsurface_Qout;
							volume=(neigh->surface_Qout+neigh->subsurface_Qout);
						}
						else{
							neigh->surface_Qout     += patch[i].neighbours[j].gamma * patch[i].surface_Qout;
							neigh->subsurface_Qout  += patch[i].neighbours[j].gamma * patch[i].subsurface_Qout;
						}
					}
				}
			}
		}

		// second, based on reservoir's volume condition to determine how to route water down the reservoir
		if(volume>=reservoir.max_volume) startDischarge=true;
		if(startDischarge) {
			Qout   = max(volume-reservoir.max_volume,0.); // water exceed the reservoir's maximum volumen
			volume-= Qout;                                // flow down the reservoir directly
			
			volume-= reservoir.discharge;                 // start human discharge
			Qout   = (Qout + reservoir.discharge);
			if(volume<=reservoir.min_volume) startDischarge=false; //close the dam if volume is less than 
			                                                       // minimum reservoir's volume
			for(i=kk;i<num_patches;i++){
				if(patch[i].drainage_type==STREAM){
					for(j=0;j<patch[i].num_neighbours; j++){
						neigh = patch[i].neighbours[j].patch;
						if(neigh->drainage_type == STREAM ){ 
							if(i==kk){
								neigh->surface_Qout     += patch[i].neighbours[j].gamma * Qout; //surface out
								neigh->subsurface_Qout  += patch[i].neighbours[j].gamma * 0.;   //no baseflow due to dam
							}
							else{
								neigh->surface_Qout     += patch[i].neighbours[j].gamma * patch[i].surface_Qout;
								neigh->subsurface_Qout  += patch[i].neighbours[j].gamma * patch[i].subsurface_Qout;
							}
						}
					}
				}
			}		
		}		
		else {
			for(i=kk+1;i<num_patches;i++){
				if(patch[i].drainage_type==STREAM){
					for(j=0;j<patch[i].num_neighbours; j++){
						neigh = patch[i].neighbours[j].patch;
						if(neigh->drainage_type == STREAM ){ 
							neigh->surface_Qout     += patch[i].neighbours[j].gamma * patch[i].surface_Qout;
							neigh->subsurface_Qout  += patch[i].neighbours[j].gamma * patch[i].subsurface_Qout;
						}
					}
				}
			}
		}
	}
	*/
return ;
}