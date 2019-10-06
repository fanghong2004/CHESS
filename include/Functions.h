#define LEN_GRASSHEADER 12
#define LEN_ARCHEADER	12

struct  command_line_object	*construct_command_line(int main_argc,char **,struct command_line_object *);
void    read_CHESS_File(char *,struct Input_Grid_Data *,struct In_File_Path  *,struct Input_Data_Range  *,struct Out_Data_Range  *,struct Spin_Up *);
//void    read_R_RHESSys_File(char *);

void    input_header(struct Input_Grid_Data *Grid, struct In_File_Path Path, int arc_flag);
void	read_images(struct patch_object *, struct Input_Grid_Data *, struct In_File_Path, int, int);
void    construct_patch(struct patch_object *,struct command_line_object *, int,char *,char *);
int     construct_routing_topology(struct patch_object *,char *,char *,int);
struct  input_Clim_Files open_Clim_Files(char *,char *);
void    construct_output_files(int,int,struct date, char *,struct output_hydro_plant *,struct command_line_object *);

void    construct_basin_output_files(char *, struct output_hydro_plant *, struct command_line_object *);
void    construct_patch_output_files(struct date,struct Out_Data_Range,char *,struct output_hydro_plant *,struct command_line_object *);
void    close_patch_output_files(struct output_hydro_plant *);

int     end_day_norm_leap(int,int);
void    init_out_monthly(struct patch_object *,struct accumulate_patch_object *,int,int,struct command_line_object *);
void    init_routing_object(patch_object *,int);
struct  daily_clim_object construct_daily_clim(struct input_Clim_Files, struct date,int,int,int,int *); //windows
//struct  daily_clim_object construct_daily_clim(double, double, double, double); //unix


void    patch_daily_I(struct patch_object *,struct daily_clim_object ,struct command_line_object *, struct	date);
void    patch_hourly(struct patch_object *, struct command_line_object *,struct	date);
void	patch_daily_F(struct patch_object *,struct command_line_object *,struct date);
void    compute_subsurface_routing_new(patch_object * ,struct command_line_object *,int ,struct date);
void    compute_subsurface_routing (patch_object * ,struct command_line_object *,int ,struct date);
void    compute_subsurface_no_routing (patch_object * ,struct command_line_object *,int ,struct date);

void    out_daily(int,struct patch_object *,struct date,struct Out_Data_Range,struct output_hydro_plant *,struct command_line_object *);
void    out_basin_level_daily(int,struct patch_object *,struct date,struct Out_Data_Range,struct output_hydro_plant *,struct command_line_object *);
void    out_patch_level_daily(int,struct patch_object *,struct date,struct Out_Data_Range,struct output_hydro_plant *,struct command_line_object *);

void    *alloc(size_t, char *, char *);
void    rewind_daily_clim(struct input_Clim_Files);
void    read_daily_climate(struct input_Clim_Files,struct Input_Data_Range,double [][12][31] ,double [][12][31],double [][12][31],double []);
void    channel_flow_routing(struct patch_object *, struct reservoir_object, struct command_line_object *, struct	date, int);


int read_record(FILE *, char *);
void init_phenology_object(patch_object *, int);