/*-------------------------------------------------------------------------------------------------------------------------------
 NAME:                    FILE_IO.CPP

		contains all of the functions for handling file input and output. 
 ------------------------------------------------------------------------------------------------------------------------------*/

#include <malloc.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include "CHESS.h"
#include "Functions.h"
#include "constants.h"
#include <iostream>
#include <fstream>

using namespace std;

//===============================================================================================================================
//creating x and y coordinate 
void create_x_coordinates(double *indata, int mr,int mc, double cellsize, double xll){
	int i,j,r;
	for(i=0;i<mr;i++){
		for(j=0;j<mc;j++){
			r=i*mc+j;
			indata[r]=xll+(double)j*cellsize;
			//printf("\n longitude %f \n",indata[r]);

		}
	}
	//getchar();
	return;
}

void create_y_coordinates(double *indata,int mr,int mc, double cellsize, double yll){
	int i,j,r;
	for(i=0;i<mr;i++){
		for(j=0;j<mc;j++){
			r=i*mc+j;
			indata[r]=yll+(double)(mr-i-1)*cellsize;
		}
	}
	return;
}


//===============================================================================================================================
//        input_ascii_int() - input an ascii image into an interger array using the (row, col) coordinates maxr and maxc.
//===============================================================================================================================
void input_ascii_int(int *indata, char *filename, int mc, int mr, int arc_flag){
	FILE *in1;
	int  r;
    int max; 
	void    pause();
	
	max = 0;
    if ( (in1 = fopen(filename, "r")) == NULL){
        printf("cannot open file %s\n",filename);
		pause();
    }
    else {
		/* skip header */
		if (arc_flag == 0)
			for (r=0; r < LEN_GRASSHEADER; r++)
				fscanf(in1,"%*s");
		else
			for (r=0; r < LEN_ARCHEADER; r++)
				fscanf(in1,"%*s");

		for (r=0; r < mr*mc; r++) {
			  fscanf(in1,"%d",&indata[r]);
			  //printf("\n patchID is:: %d \n",indata[r]);
			  if (indata[r] > max) 
						max = indata[r];
		}
			 
		printf("\nMax for %s is %d",filename,max);
			 fclose(in1);
    }
    return;
}
 

//===============================================================================================================================
//        input_ascii_float() - input an ascii image into an float array using the (row, col) coordinates maxr and maxc.
//===============================================================================================================================
void input_ascii_float(float *indata, char *filename,int mc,int mr, int arc_flag)
{
   FILE *in1;
   int  r;
   float max; 
   void    pause();

    max = 0;
    if ( (in1 = fopen(filename, "r")) == NULL){
        printf("cannot open file %s\n",filename);
		pause();
    }
    else {
		/* skip header */
		if (arc_flag == 0)
			for (r=0; r < LEN_GRASSHEADER; r++)
				fscanf(in1,"%*s");
		else
			for (r=0; r < LEN_ARCHEADER; r++)
				fscanf(in1,"%*s");

		for (r=0; r < mr*mc; r++) {
			  fscanf(in1,"%f",&indata[r]);

			  indata[r] = (float)(indata[r]);
			  //printf("dem is %f \n",indata[r]);
			  //getchar();
			  if (indata[r] > max) 
				max = indata[r];
	    }
			
		 printf("\nMax for %s is %f",filename, max);
		 fclose(in1);
    }
    return;
}

//===============================================================================================================================
//        input_header() - input information (row, col) from [root].header
//===============================================================================================================================
void input_header(struct Input_Grid_Data *Grid, struct In_File_Path Path, int arc_flag){
    ifstream  in1,in2;
	char tmp[100];
	int i;
	char  fndem[MAXS], fn_flowtable[MAXS];

	//File pointer
	strcpy(fn_flowtable, Path.inImgFile);
	strcat(Path.inImgFile, Path.prefix);
	strcpy(fndem, Path.inImgFile);

	// append '.' extensions to each filename (these should be generalized) */
	strcat(fndem, ".dem");
	strcat(fn_flowtable, Path.FlowTableName);

	in1.open(fndem, ios::in);
	if ( !in1 ){
        cerr<<"cannot open file \n"<<fndem<<endl;
		return ;
    }
    else {
		//cerr << "sucessfully opened DEM file \n" << fndem << endl;
		if (arc_flag == 0) {
			for (i=0; i< 9; i++)
				in1>>tmp;
			in1 >> Grid->maxr >> Grid->maxc;
		}
		else{
			in1 >> tmp >> Grid->maxc;
			in1 >> tmp >> Grid->maxr;
			in1 >> tmp >> Grid->xll;
			in1 >> tmp >> Grid->yll;
			in1 >> tmp >> Grid->cellsize;
			in1 >> tmp >> Grid->NODATA_value;
			cout << "cols are " << Grid->maxc << endl;
			cout << "rows are " << Grid->maxr << endl;
		}
		in1.close();
	}

	//reading flow table to determine the number of patches to be simulated
	in2.open(fn_flowtable, ios::in);
	if (!in2){
		cerr << "cannot open file !" << fn_flowtable << endl;
		return ;
	}
	else {
		//cerr << "sucessfully opened flowtable file \n" << fndem << endl;
		in2 >> Grid->num_patches;
		cout << "The number of patches are " << Grid->num_patches << endl;
		//getchar();
	}
	in2.close();

	return;
}


//===============================================================================================================================
//        input_prompt() - input root filename, create full filenames
//===============================================================================================================================
void	read_images(struct patch_object *patch, struct Input_Grid_Data *Grid,struct In_File_Path Path, int f_flag, int arc_flag){

	// filenames for each image and file
	char  fnpatch[MAXS],fndem[MAXS],fnslope[MAXS], fnaspect[MAXS], fneast_horizon[MAXS],fnwest_horizon[MAXS];
	char  fnsoil[MAXS],fnveg[MAXS],fnstream[MAXS],fn_flowtable[MAXS];


    //File pointer
	int          *ppatch;
	float        *pdem;
	float        *pslope;
	float        *paspect;
	float        *peast_horizon;
	float        *pwest_horizon;
	int          *psoil;
	int          *pveg;
	int		     *pstream;
	double       *plon;
	double       *plat;          

    //local functions
	void    header_help(int ,int , char* );
	void	input_ascii_int(int *, char *, int, int, int);
	void	input_ascii_float(float *, char *, int, int, int);
	void    create_x_coordinates(double *,int,int , double , double );
	void    create_y_coordinates(double *,int,int , double , double );
	void    pause();

    // copy the root filename into the specific filenames
	strcpy(fn_flowtable, Path.inImgFile);
	strcat(Path.inImgFile,Path.prefix);
	strcpy(fnpatch, Path.inImgFile);
	strcpy(fndem, Path.inImgFile);
	strcpy(fnslope, Path.inImgFile);
	strcpy(fnaspect, Path.inImgFile);
	strcpy(fneast_horizon, Path.inImgFile);
	strcpy(fnwest_horizon, Path.inImgFile);
	strcpy(fnsoil, Path.inImgFile);
	strcpy(fnveg, Path.inImgFile);
	strcpy(fnstream, Path.inImgFile);

    // append '.' extensions to each filename (these should be generalized) */
    strcat(fnpatch,".patch"); 
    strcat(fndem,".dem");
    strcat(fnslope,".slope");
    strcat(fnaspect,".aspect");
    strcat(fneast_horizon,".east");
    strcat(fnwest_horizon,".west");
    strcat(fnsoil,".soil");
    strcat(fnveg,".veg");
    strcat(fnstream,".stream");
	strcat(fn_flowtable,Path.FlowTableName);

	// allocate and read input map images
	ppatch = new  int[Grid->maxr*Grid->maxc];      
	input_ascii_int(ppatch, fnpatch, Grid->maxr, Grid->maxc, arc_flag);

	plon = new double[Grid->maxr*Grid->maxc];
	create_x_coordinates(plon, Grid->maxr, Grid->maxc, Grid->cellsize, Grid->xll);

	plat = new double[Grid->maxr*Grid->maxc];
	create_y_coordinates(plat, Grid->maxr, Grid->maxc, Grid->cellsize, Grid->yll);

	pdem = new float[Grid->maxr*Grid->maxc];
	input_ascii_float(pdem, fndem, Grid->maxr, Grid->maxc, arc_flag);

	pslope = new float[Grid->maxr*Grid->maxc];
	input_ascii_float(pslope, fnslope, Grid->maxr, Grid->maxc, arc_flag);

	paspect = new float[Grid->maxr*Grid->maxc];
	input_ascii_float(paspect, fnaspect, Grid->maxr, Grid->maxc, arc_flag);

	peast_horizon = new float[Grid->maxr*Grid->maxc];
	input_ascii_float(peast_horizon, fneast_horizon, Grid->maxr, Grid->maxc, arc_flag);

	pwest_horizon = new float[Grid->maxr*Grid->maxc];
	input_ascii_float(pwest_horizon, fnwest_horizon, Grid->maxr, Grid->maxc, arc_flag);

	psoil = new  int[Grid->maxr*Grid->maxc];
	input_ascii_int(psoil, fnsoil, Grid->maxr, Grid->maxc, arc_flag);

	pveg = new  int[Grid->maxr*Grid->maxc];
	input_ascii_int(pveg, fnveg, Grid->maxr, Grid->maxc, arc_flag);

	pstream = new  int[Grid->maxr*Grid->maxc];
	input_ascii_int(pstream, fnstream, Grid->maxr, Grid->maxc, arc_flag);

	int count = 0;
	for (int i = 0; i<Grid->maxr*Grid->maxc; i++){
		if (ppatch[i] != Grid->NODATA_value ){//
			patch[count].ID = ppatch[i];
			patch[count].x = plon[i];
			patch[count].y = plat[i];
			patch[count].z = pdem[i];
			patch[count].slope = pslope[i];
			patch[count].aspect = paspect[i];
			patch[count].e_horizon = peast_horizon[i];
			patch[count].w_horizon = pwest_horizon[i];
			patch[count].stream = pstream[i];
			patch[count].vegtype = pveg[i];
			patch[count].soiltype = psoil[i];
			count++;
			//cout << count <<" "<< patch[count].ID<< endl;
			//getchar();
		}
	}

	delete [] ppatch;
	delete[] pdem;
	delete[] pslope;
	delete[] paspect;
	delete[] peast_horizon;
	delete[] pwest_horizon;
	delete [] psoil;
	delete [] pveg;
	delete [] pstream;
	delete [] plon;
	delete [] plat;   

	return;
}


//===============================================================================================================================
//        pause() - stop program until user presses enter (return)
//===============================================================================================================================
void pause()
    {  
    /* clear buffer first */

    while (getchar() != '\n')
        ;
    printf("\nPress enter to continue...");
    getchar();
}


void read_CHESS_File(char FileName[200], struct Input_Grid_Data *Grid, struct In_File_Path *Path, struct Input_Data_Range *InRange, struct Out_Data_Range *OutRange, struct Spin_Up *Spin)
{

	FILE *in1;
	char  record[MAXSTR];
	char  str[200];

	if ((in1 = fopen(FileName, "r")) == NULL){
		printf("Warning:: cannot open the CHESS_info.txt file \n");
		printf("Make sure it is in the same directory as the exe file \n");
		return;
	}
	else
	{
		while (!feof(in1))
		{
			read_record(in1, record);
			read_record(in1, record);
			read_record(in1, record);
			read_record(in1, record);
			read_record(in1, record);
			//cout<<record<<endl;

			fscanf(in1, "%s %d", &str, &InRange->start_year);
			read_record(in1, record);
			fscanf(in1, "%s %d", &str, &InRange->end_year);
			read_record(in1, record);
			fscanf(in1, "%s %d", &str, &InRange->start_month);
			read_record(in1, record);
			fscanf(in1, "%s %d", &str, &InRange->end_month);
			read_record(in1, record);
			fscanf(in1, "%s %d", &str, &InRange->start_day);
			read_record(in1, record);
			fscanf(in1, "%s %d", &str, &InRange->end_day);
			read_record(in1, record);
			fscanf(in1, "%s %d", &str, &InRange->start_hour);
			read_record(in1, record);
			fscanf(in1, "%s %d", &str, &InRange->end_hour);
			read_record(in1, record);
			//cout<<"In end hour is "<<InRange->end_hour<<endl;

			read_record(in1, record);
			read_record(in1, record);
			//cout<<record<<endl;

			fscanf(in1, "%s %d", &str, &OutRange->first_year);
			read_record(in1, record);
			fscanf(in1, "%s %d", &str, &OutRange->last_year);
			read_record(in1, record);
			fscanf(in1, "%s %d", &str, &OutRange->first_month);
			read_record(in1, record);
			fscanf(in1, "%s %d", &str, &OutRange->last_month);
			read_record(in1, record);
			fscanf(in1, "%s %d", &str, &OutRange->first_day);
			read_record(in1, record);
			fscanf(in1, "%s %d", &str, &OutRange->last_day);
			read_record(in1, record);
			fscanf(in1, "%s %d", &str, &OutRange->first_hour);
			read_record(in1, record);
			fscanf(in1, "%s %d", &str, &OutRange->last_hour);
			read_record(in1, record);
			//cout<<"out data last hour "<<OutRange->last_hour<<endl;

			read_record(in1, record);
			read_record(in1, record);
			read_record(in1, record);
			//cout<<record<<endl;

			fscanf(in1, "%s %d", &str, &Spin->spin_years);
			read_record(in1, record);
			fscanf(in1, "%s %d", &str, &Spin->spin_interval);
			read_record(in1, record);
			fscanf(in1, "%s %d", &str, &Spin->spin_flag);
			read_record(in1, record);
			//cout<<"sping flag is "<<Spin->spin_flag<<endl;
			//getchar();

			read_record(in1, record);
			read_record(in1, record);

			fscanf(in1, "%s %s", &str, &Path->prefix);
			read_record(in1, record);
			fscanf(in1, "%s %s", &str, &Path->inDefFile);
			read_record(in1, record);
			fscanf(in1, "%s %s", &str, &Path->inImgFile);
			read_record(in1, record);
			fscanf(in1, "%s %s", &str, &Path->inClimPath);
			read_record(in1, record);
			fscanf(in1, "%s %s", &str, &Path->outPutPath);
			read_record(in1, record);
			fscanf(in1, "%s %s", &str, &Path->FlowTableName);
			read_record(in1, record);

			//cout<<Path->FlowTableName<<endl;
			//getchar();
			break;
		}
	}
	return;

}



