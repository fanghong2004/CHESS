#include <stdio.h>
#include "CHESS.h"

void init_routing_object(patch_object *patch,int num_patches){

	for (int i=0;i<num_patches;i++){
		
		patch[i].detention_store_preday    = patch[i].detention_store; 
		patch[i].rz_storage_preday         = patch[i].rz_storage;     
		patch[i].unsat_storage_preday      = patch[i].unsat_storage; 
		patch[i].sat_deficit_preday        = patch[i].sat_deficit;

		//patch[i].streamflow           = 0.0;
		//patch[i].base_flow            = 0.0;
		patch[i].subsurface_Qout      = 0.0;
		patch[i].subsurface_Qin       = 0.0;
		patch[i].surface_Qout         = 0.0;
		patch[i].surface_Qin          = 0.0;
		patch[i].gw_drainage          = 0.0;
		patch[i].gw.Qout              = 0.0;
		patch[i].gw.Nout              = 0.0;
		patch[i].infiltration_excess  = 0.0;

		patch[i].soil_ns.leach        = 0.0;

		patch[i].surface_NO3          = 0.0;
		patch[i].surface_NH4          = 0.0;
		patch[i].NO3_leached          = 0.0;
		/* kg/m2/day 	*/\
	
		patch[i].subsurface_leach_NO3_out=0.;   /* kg/m2/day 	*/
		patch[i].surface_leach_NO3_out=0.;      /* kg/m2/day 	*/
		patch[i].subsurface_leach_NH4_out=0.;   /* kg/m2/day 	*/
		patch[i].surface_leach_NH4_out=0.;      /* kg/m2/day 	*/
		patch[i].DON_loss = 0.0;
		patch[i].DOC_loss = 0.0;
		////////////////////////////////////////////
		patch[i].canopy_strata->cdf.net_psn = 0.0;
		patch[i].canopy_strata->cdf.cpool_deadcroot_gr=0.0;
		patch[i].canopy_strata->cdf.cpool_deadcroot_gr=0.0;
		patch[i].canopy_strata->cdf.cpool_froot_gr=0.0;
		patch[i].canopy_strata->cdf.cpool_leaf_gr=0.0;
		patch[i].canopy_strata->cdf.cpool_livecroot_gr=0.0;
		patch[i].canopy_strata->cdf.cpool_livestem_gr=0.0;
		patch[i].canopy_strata->cdf.total_gr=0.0;

		patch[i].canopy_strata->cdf.froot_mr=0.0;
		patch[i].canopy_strata->cdf.leaf_day_mr=0.0;
		patch[i].canopy_strata->cdf.leaf_night_mr=0.0;
		patch[i].canopy_strata->cdf.livecroot_mr=0.0;
		patch[i].canopy_strata->cdf.livestem_mr=0.0;
		patch[i].canopy_strata->cdf.total_mr=0.0;

		patch[i].canopy_strata->cdf.psn_to_cpool=0.0;
		patch[i].canopy_strata->cdf.potential_psn_to_cpool=0.0;

		patch[i].canopy_strata->cdf.transfer_deadcroot_gr=0.0;
		patch[i].canopy_strata->cdf.transfer_deadstem_gr=0.0;
		patch[i].canopy_strata->cdf.transfer_froot_gr=0.0;
		patch[i].canopy_strata->cdf.transfer_gr=0.0;
		patch[i].canopy_strata->cdf.transfer_leaf_gr=0.0;
		patch[i].canopy_strata->cdf.transfer_livecroot_gr=0.0;
		patch[i].canopy_strata->cdf.transfer_livestem_gr=0.0;

		//=================================================================
		//double check if we need initialize these variables
		patch[i].canopy_strata->cdf.cpool_to_leafc =0;
		patch[i].canopy_strata->cdf.cpool_to_frootc=0;
		patch[i].canopy_strata->cdf.leafc_transfer_to_leafc =0;
		patch[i].canopy_strata->cdf.frootc_transfer_to_frootc=0;

		patch[i].canopy_strata->cdf.cpool_to_livestemc = 0;
		patch[i].canopy_strata->cdf.cpool_to_deadstemc = 0;
		patch[i].canopy_strata->cdf.cpool_to_livecrootc =0;
		patch[i].canopy_strata->cdf.cpool_to_deadcrootc =0;
		patch[i].canopy_strata->cdf.livestemc_transfer_to_livestemc =0;
		patch[i].canopy_strata->cdf.deadstemc_transfer_to_deadstemc =0;
		patch[i].canopy_strata->cdf.livecrootc_transfer_to_livecrootc=0;
		patch[i].canopy_strata->cdf.deadcrootc_transfer_to_deadcrootc=0;


		patch[i].assim_shade = 0;
		patch[i].assim_sunlight = 0.;
		patch[i].canopy_strata->APAR_diffuse = 0.;
		patch[i].canopy_strata->APAR_direct = 0.;
		patch[i].canopy_strata->Kstar_diffuse=0.;
		patch[i].canopy_strata->Kstar_direct = 0.;
		patch[i].PAR_diffuse = 0.;
		patch[i].PAR_direct = 0.;
		patch[i].Kdown_diffuse = 0.;
		patch[i].Kdown_direct = 0.;
	}
	return;
}