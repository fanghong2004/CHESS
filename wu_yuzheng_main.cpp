//=======================================================================================================================
//  A brief introduction to the "R-RHESSys" model, which is short for rasterized RHESSys model::
//
//  The "R-RHESSys" is developed based on the Regional Hydroecological Simulation Systems (RHESSys, Tague and Band, 2004)
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
//  Running R-RHESSys, a potential alternative to the RHESSys, requires the following data:
//
//  First, DEM data and DEM-derived slope, aspect, streams, roads raster data in ArcInfor ASCII format;
//  Second, meteorological data: daily minimum and maximum temperature (in Celsius degree), and precipitation (in meter);
//  Third, annual time-series atmospheric CO2 concentration (in ppm);
//  Fourth, rasterized land-cover, land use or vegetation data to predefine land cover, land use or vegetation types;
//  Fifth, rasterized soil texture class data to predefine soil types for the study region;
//  Finally, a flow table file describes the spatial connectivity of water and nutrients movement among simulated patches.

//  Besides, four default files of patch, soil, vegetation and land-use are provided. These files include parameters
//  relevant to soil, vegetation and land-use that are required to parameterize before running R-RHESSys.

//  As its predecessor, R-RHESSys is able to simulate::
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
//  Users are welcome to employ R-RHESSys to regions of their own interests. Examples of model forcing data can be
//  provided for reference or learning. For other questions, please contact the author::

//  Dr. Guoping Tang
//  Division of Earth and Ecosystem Sciences
//  Desert Research Institute
//  Reno, NV 89512
//  Email: gtang@dri.edu or tangg2010@gmail.com
//  Phone: 775-673-7938 (o)

//  Date created: 08-31-2011
//=======================================================================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chess.h"
#include "Functions.h"
#include "Constants.h"
#include <iostream>
#include <time.h>
using namespace std;

//=======================================================================================================================
//Note:: Users should define the following global variables
//=======================================================================================================================
//spatial geographic information for the study region
int        maxc = 177,maxr = 146;
double    xll = 799600.00012;
double    yll = 2647174.38755;
double    cellsize = 200.;
float     NODATA_value = -9999.0;

// define the simulation year, month and day
const int start_year = 1960, end_year = 2013, start_month = 1, end_month = 12, start_day = 1, end_day = 31;
const int start_hour = 2, end_hour = 24;
//int       end_day; //The end of date in February varies between normal and leap year

// define the time period for outputting simulation results
// out_date_range outdate={first_year, last_year, first_month, last_month, first_day, last_day, first_hour, last_hour};
struct out_date_range out_date = {1961, 2013, 1, 12, 1, 31, 1, 24 };

// define the number of spin years required for vegetation and soil carbon to reach the stable state with long-term
// climatology. Spin interval is the period of input climate data used for spin-up simulations
const int spin_years =4, spin_interval = 10;
bool      spin_flag = true;

// define the input file prefix and paths for model forcing data
char  prefix[20] = "xf_ws";
char  inDefFile[120] = { "D://muzhen//wuyuzheng//xf_ws//defs//" };
char  inImgFile[120] = { "D://muzhen//wuyuzheng//xf_ws//flowtable//" };
char  inFlowFile[120] = { "D://muzhen//wuyuzheng//xf_ws//flowtable//" };
char  inClimPath[120] = { "D://muzhen//wuyuzheng//xf_ws//climate//" };
char  outPutPath[120] = { "D://muzhen//wuyuzheng//xf_ws//outdata//" };

char  FlowTableName[40] = "xf_ws_flow_table.dat";

//=======================================================================================================================
// The main interface to control model simulation
//=======================================================================================================================
int	main(int main_argc, char **main_argv)
{
	//struct objects
	struct  patch_object *patch;
	struct	command_line_object	*command_line;
	struct  date current_date = { 0, 0, 0, 0 };
	struct  daily_clim_object daily_clim = { 0 };
	struct  output_hydro_plant DM_outfiles = { 0 };
	struct  input_Clim_Files   inClimFiles = { 0 };
	struct  reservoir_object   reservoir = { 10072, 2284.5, 50, 338.6, 100, 1514.4, 2351.55, 0., 4. };

	//local variables
	int     num_patches, kk = 0;
	int     f_flag = 1, arc_flag = 1, CO2_flag = 1, out_flag = 0;
	int     i = 0, j = 0, endyear = 0, spin_yrs = 0;
	int     firstmonth, lastmonth, firstday, lastday;
	clock_t  t;

	//#######################################################################################################################
	//construct and assign command line arguments
	command_line = (struct command_line_object *) malloc(sizeof(struct command_line_object));
	construct_command_line(main_argc, main_argv, command_line);

	//Dynamically allocate patch objects according to input image size
	patch = (struct patch_object *) calloc(maxr*maxc, sizeof(struct patch_object));

	//reading GRASS- or ArcInfo-based input images such as DEM,slope,aspect....stream,roads
	read_images(patch, maxr, maxc, cellsize, xll, yll, inImgFile, prefix, f_flag, arc_flag);

	//Initialize the default values of patch fields/members
	construct_patch(patch, command_line, maxr, maxc, inDefFile, prefix);

	//constructing routing list topography
	num_patches = construct_routing_topology(patch, inFlowFile, FlowTableName, maxr, maxc);

	//open input climate files (daily precipitation, minimum temperature and maximum temperature)
	inClimFiles = open_Clim_Files(inClimPath, prefix);

	//starting spin-up + actual simulations
	printf("\n Starting simulation:: \n");
	do {
		t = clock();
		if (spin_yrs<spin_years) { //&& command_line->grow_flag>0
			endyear = start_year + spin_interval - 1;
			spin_flag = true;
			command_line->routing_flag = 0;
		}
		else {
			endyear = end_year;
			spin_flag = false;
			command_line->routing_flag = 1;
		}

		//contruct basin-level daily output files
		if (!spin_flag && out_flag == 0){
			construct_basin_output_files(outPutPath, &DM_outfiles, command_line);
			out_flag = 1;
		}

		//starting simulation by year, month and day
		for (current_date.year = start_year; current_date.year <= endyear; current_date.year++){

			if (current_date.year == start_year)
				firstmonth = start_month;
			else
				firstmonth = 1;

			if (current_date.year == end_year)
				lastmonth = end_month;
			else
				lastmonth = 12;

			for (current_date.month = firstmonth; current_date.month <= lastmonth; current_date.month++){

				//construct patch-level daily output files for a month
				if (!spin_flag && command_line->p != NULL){
					construct_patch_output_files(current_date, out_date, outPutPath, &DM_outfiles, command_line);
				}

				if (current_date.year == start_year && current_date.month == start_month)
					firstday = start_day;
				else
					firstday = 1;

				if (current_date.year == end_year && current_date.month == end_month)
					lastday = end_day;
				else
					lastday = end_day_norm_leap(current_date.year, current_date.month - 1);

				//determine if the year is leap year and thus the end day of February can be different
				//end_day=end_day_norm_leap(current_date.year,current_date.month-1);

				for (current_date.day = firstday; current_date.day <= lastday; current_date.day++){

					//reading input climate data for the simulated year, month and day
					daily_clim = construct_daily_clim(inClimFiles, current_date, start_year, start_month, start_day, &CO2_flag);

					init_routing_object(patch, num_patches);

					//starting cell-by-cell based simulation one by one
					for (i = 0; i<num_patches; i++){

						patch_daily_I(&patch[i], daily_clim, command_line, current_date);

						for (current_date.hour = start_hour; current_date.hour <= end_hour; current_date.hour++){
							patch_hourly(&patch[i], command_line, current_date);
						}

						patch_daily_F(&patch[i], command_line, current_date);

						compute_subsurface_routing_new(&patch[i], command_line, 24, current_date);

					}

					channel_flow_routing(patch, reservoir, command_line, current_date, num_patches);

					//output daily-step variables
					if (!spin_flag){
						if (command_line->b != NULL)
							out_basin_level_daily(num_patches, patch, current_date, out_date, &DM_outfiles, command_line);
						if (command_line->p != NULL)
							out_patch_level_daily(num_patches, patch, current_date, out_date, &DM_outfiles, command_line);
					}

				}//end of days

				//close patch_level_output_files
				if (!spin_flag)
					close_patch_output_files(&DM_outfiles);

			} //end of months

			CO2_flag = 1;
			if (!spin_flag)
				printf("year, month and day:: %d %d %d \n", current_date.year, current_date.month - 1, current_date.day - 1);

			spin_yrs += 1;
			if (spin_yrs == spin_years && spin_flag) break;
		} //end of years

		if (spin_flag){
			rewind_daily_clim(inClimFiles);
			printf("Spin-up to %d years takes %f minutes \n", spin_yrs, ((float)(clock() - t) / CLOCKS_PER_SEC / 60.));
		}

	} while (spin_flag);

	return (0);
} //end of main programs
//=======================================================================================================================