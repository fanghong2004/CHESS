//===============================================================================================================================
//  A brief introduction to the "CHESS" model, which is short for rasterized RHESSys model::
//
//  The "CHESS" is developed based on the Regional Hydroecological Simulation Systems (RHESSys, Tague and Band, 2004)
//  but excluded basin, hillslope and zone related hierarchical framework/structure in its predecessor. It also excluded
//  the TOP-MODEL embedded in its predecessor. It is a strictly cell-by-cell-based model and uses spatially explicit
//  routing algorithm to simulate spatial connectivity and movement of water and dissolved nutrients among simulated cells.
//  Since vegetation growth is dynamically simulated, a spin-up simulation control for vegetation growth and associated
//  carbon dynamics was added. This enables users to model vegetation and cycling of C and N in terrestrial ecosystems.
//  In addition, land surface water balance in each cell will be checked before routing water and solutes. The model codes
//  were checked/debugged and greatly simplified compared to the original RHESSys model. In author's personal view, it is
//  easier to handle than its predecessor while most subroutines are kept identical as their counterparts in its
//  predecessor.
//
//  Running CHESS, a potential alternative to the RHESSys, requires the following data:
//
//  First, DEM data and DEM-derived slope, aspect, streams, roads raster data in ArcInfor ASCII format;
//  Second, meteorological data: daily minimum and maximum temperature (in Celsius degree), and precipitation (in meter);
//  Third, annual time-series atmospheric CO2 concentration (in ppm);
//  Fourth, rasterized land-cover, land use or vegetation data to predefine land cover, land use or vegetation types;
//  Fifth, rasterized soil texture class data to predefine soil types for the study region;
//  Finally, a flow table file describes the spatial connectivity of water and nutrients movement among simulated patches.

//  Besides, four default files of patch, soil, vegetation and land-use are provided. These files include parameters
//  relevant to soil, vegetation and land-use that are required to parameterize before running CHESS.

//  As its predecessor, CHESS is able to simulate::
//
//  (i) Land surface hydrology, including river stream flow, base flow, ET, plant transpiration, soil moisture, and etc.
//  (ii) Ecosystem dynamics: vegetation growth and productivity, cycling of carbon and nitrogen (e.g., soil autotrophic
//       and heterotrophic respiration) in terrestrial ecosystems, and etc.
//  (iii) Biogeochemical dynamics:: soil nitrification and denitrification, river nitrate, dissolved DOC and DON,
//       soil mineralization, and etc.
//
//  Note: For simulating land surface hydrology, a least of 20-year spin-up simulations are required for leaf area index
//  to reach stable state. For simulating soil and vegetation carbon, spin-up simulations more than 150 years are required.
//
//  Users are welcome to employ CHESS to regions of their own interests. Examples of model forcing data can be
//  provided for reference or learning. For other questions, please contact the author::

//  Dr. Guoping Tang
//  Division of Earth and Ecosystem Sciences
//  Desert Research Institute
//  Reno, NV 89512
//  Email: gtang@dri.edu or tangg2010@gmail.com
//  Phone: 775-673-7938 (o)

//  Date created: 08-31-2011
//===============================================================================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CHESS.h"
#include "Functions.h"
#include "Constants.h"
#include <iostream>
#include <time.h>

using namespace std;

int	main(int main_argc, char **main_argv)
{
	//===========================================================================================================================
	//Note:: Users need define the following global variables
	//===========================================================================================================================
	//1. define the simulation year, month and day
	//struct Input_Data_Range InRange = { start_year, end_year, start_month, end_month, start_day, end_day, start_hour, end_hour };
	struct Input_Data_Range InRange = { 1981, 2018, 1, 12, 1, 31, 1, 24 };

	//2. define the time period for output results
	// Out_Data_Range outdate={first_year, last_year, first_month, last_month, first_day, last_day, first_hour, last_hour};
	struct Out_Data_Range OutRange = { 1990, 2017, 1, 12, 1, 31, 1, 24 };

	//3. define the number of spin years required for vegetation and soil carbon to reach the equilibirium state with long-term
	// climatology. Spin interval is the period of input climate data used for spin-up simulations
	//struct Spin_Up Spin={spin_years,spin_interval, spin_flag};
	struct Spin_Up   Spin = { 20, 10, true };

	//4. define the input file prefix and paths for model forcing data
	struct In_File_Path Path{
		"twin",   //input file name prefix
		"D://GIS//newgb//twin//defs//", //input_default_file_directory
		"D://GIS//newgb//twin//image//", //input_image_file_directory
		"D://GIS//newgb//snotel//", //input_climate_file_directory
		"D://GIS//newgb//twin//simulated2//",    //output_file_directory
		"twin_flow_table.dat" //flow table name
	};

	//===========================================================================================================================
	//users should not modify the following varaibles
	//===========================================================================================================================
	struct  patch_object *patch = {};
	struct	command_line_object	*command_line;
	struct  date current_date = { 0, 0, 0, 0 };
	struct  daily_clim_object daily_clim = { 0 };
	struct  output_hydro_plant DM_outfiles = { 0 };
	struct  input_Clim_Files   inClimFiles = { 0 };
	struct  reservoir_object   reservoir = { 10072, 2284.5, 50, 338.6, 100, 1514.4, 2351.55, 0., 4. };
	struct  Input_Grid_Data  Grid = {};
	struct  Input_Grid_Data  *pGrid = { &Grid };

	//local variables
	int     f_flag = 1, arc_flag = 1, CO2_flag = 1, out_flag = 0;
	int     i = 0, j = 0, endyear = 0, spin_yrs = 0;
	int     firstmonth, lastmonth, firstday, lastday;
	clock_t  t;


	//===========================================================================================================================
	//construct and assign command line arguments
	command_line = (struct command_line_object *) malloc(sizeof(struct command_line_object));
	construct_command_line(main_argc, main_argv, command_line);

	//input header file
	input_header(pGrid, Path, arc_flag);

	//Dynamically allocate patch objects according to input image size
	patch = (struct patch_object *) calloc(Grid.num_patches, sizeof(struct patch_object));

	//reading GRASS- or ArcInfo-based input images such as DEM,slope,aspect....stream,roads
	read_images(patch, pGrid, Path, f_flag, arc_flag);

	//Initialize the default values of patch fields/members
	construct_patch(patch, command_line, pGrid->num_patches, Path.inDefFile, Path.prefix);

	//constructing routing list topography
	construct_routing_topology(patch, Path.inImgFile, Path.FlowTableName, pGrid->num_patches);

	//open input climate files (daily precipitation, minimum temperature and maximum temperature)
	inClimFiles = open_Clim_Files(Path.inClimPath, Path.prefix);

	//starting spin-up + actual simulations
	printf("\n Starting simulation:: \n");
	do {
		t = clock();
		if (spin_yrs<Spin.spin_years) { //&& command_line->grow_flag>0
			endyear = InRange.start_year + Spin.spin_interval - 1;
			Spin.spin_flag = true;
			command_line->routing_flag = 0;
		}
		else {
			endyear = InRange.end_year;
			Spin.spin_flag = false;
			command_line->routing_flag = 1;
		}

		//contruct basin-level daily output files
		if (!Spin.spin_flag && out_flag == 0){
			construct_basin_output_files(Path.outPutPath, &DM_outfiles, command_line);
			out_flag = 1;
		}

		//starting simulation by year, month and day
		for (current_date.year = InRange.start_year; current_date.year <= endyear; current_date.year++){

			if (current_date.year == InRange.start_year)
				firstmonth = InRange.start_month;
			else
				firstmonth = 1;

			if (current_date.year == InRange.end_year)
				lastmonth = InRange.end_month;
			else
				lastmonth = 12;

			init_phenology_object(patch, pGrid->num_patches);

			for (current_date.month = firstmonth; current_date.month <= lastmonth; current_date.month++){

				//construct patch-level daily output files for a month
				if (!Spin.spin_flag && command_line->p != NULL){
					construct_patch_output_files(current_date, OutRange, Path.outPutPath, &DM_outfiles, command_line);
				}

				if (current_date.year == InRange.start_year && current_date.month == InRange.start_month)
					firstday = InRange.start_day;
				else
					firstday = 1;

				if (current_date.year == InRange.end_year && current_date.month == InRange.end_month)
					lastday = InRange.end_day;
				else
					lastday = end_day_norm_leap(current_date.year, current_date.month - 1);

				//determine if the year is leap year and thus the end day of February can be different
				//end_day=end_day_norm_leap(current_date.year,current_date.month-1);

				for (current_date.day = firstday; current_date.day <= lastday; current_date.day++){

					//reading input climate data for the simulated year, month and day
					daily_clim = construct_daily_clim(inClimFiles, current_date, InRange.start_year, InRange.start_month, InRange.start_day, &CO2_flag);

					init_routing_object(patch, pGrid->num_patches);

					//starting cell-by-cell based simulation one by one
					for (i = 0; i<pGrid->num_patches; i++){

						patch_daily_I(&patch[i], daily_clim, command_line, current_date);

						for (current_date.hour = InRange.start_hour; current_date.hour <= InRange.end_hour; current_date.hour++){
							patch_hourly(&patch[i], command_line, current_date);
						}

						patch_daily_F(&patch[i], command_line, current_date);

						compute_subsurface_routing_new(&patch[i], command_line, 24, current_date);

					}

					channel_flow_routing(patch, reservoir, command_line, current_date, pGrid->num_patches);

					//output daily-step variables
					if (!Spin.spin_flag){
						if (command_line->b != NULL)
							out_basin_level_daily(pGrid->num_patches, patch, current_date, OutRange, &DM_outfiles, command_line);
						if (command_line->p != NULL)
							out_patch_level_daily(pGrid->num_patches, patch, current_date, OutRange, &DM_outfiles, command_line);
					}

				}//end of days

				//close patch_level_output_files
				if (!Spin.spin_flag)
					close_patch_output_files(&DM_outfiles);

			} //end of months

			CO2_flag = 1;
			if (!Spin.spin_flag)
				printf("year, month and day:: %d %d %d \n", current_date.year, current_date.month - 1, current_date.day - 1);

			spin_yrs += 1;
			if (spin_yrs == Spin.spin_years && Spin.spin_flag) break;
		} //end of years

		if (Spin.spin_flag){
			rewind_daily_clim(inClimFiles);
			printf("Spin-up to %d years takes %f minutes \n", spin_yrs, ((float)(clock() - t) / CLOCKS_PER_SEC / 60.));
		}

	} while (Spin.spin_flag);

	return (0);
} //end of main programs
//===============================================================================================================================