#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include "../inc/stringutils.h"
#include "../inc/linkedlist.h"
#include "../inc/IVsweep.h"
#include "../inc/gpib_io.h"

#define WIDTH 850
#define HEIGHT 500

#define BWIDTH 120
#define BHEIGHT 45

#define X1 15
#define X2 155
#define X3 295
#define X4 435
#define X5 575
#define X6 715

int gpibADDR;
int gpibHANDLE;

// The application
typedef struct{

  int MODE;
  int NUMBER;

  // The application 
  GtkApplication *app;

  // Initialize and Measure
  GtkWidget *initBUTTON; 
  GtkWidget *measBUTTON;
  GtkWidget *gpibBUTTON;

  // the main window
  GtkWidget *window;
  GtkWidget *fixed;

    // SMU control
  GtkWidget *smuBUTTON; 
  GtkWidget *disBUTTON;  
  GtkWidget *smuLABEL1;
  GtkWidget *smuLABEL2;
  GtkWidget *smuLABEL3;
  GtkWidget *smuLABEL4;
  GtkWidget *smuLABEL5;
  GtkWidget *smuLABEL6;
  GtkWidget **SMU;

  // VAR control
  GtkWidget *varBUTTON;
  GtkWidget *varLABEL1;
  GtkWidget *varLABEL2;
  GtkWidget *varLABEL3;
  GtkWidget *varLABEL4;
  GtkWidget *varLABEL5;
  GtkWidget **VAR;

  // List Control
  GtkWidget *listENTRY; 
  GtkWidget *listCOMBO; 
  GtkWidget *listADD; 
  GtkWidget *listREM;
  GtkWidget *listALL;
  node_t *comboVARS;
  node_t *listVARS;
  char* listSTR;

  // Save Data 
  GtkWidget *saveWINDOW;
  GtkWidget *saveBUTTON;
  GtkWidget *saveENTRY;
  GtkWidget *saveDATA;
  GtkWidget *saveINC; 
  GtkWidget *saveLABEL;

  char* filename;
  int increment; 

}GTKwrapper;


//////////////////////////////////////////
// INITIALIZATION AND MEASURE CALLBACKS //
//////////////////////////////////////////
static void SETGPIB(GtkWidget *gpibBUTTON, gpointer data)
{ 
  gpibADDR = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(gpibBUTTON));

}
static int INITIALIZE_GPIB(GtkWidget *initBUTTON)
{  
  gpibHANDLE = _initialize(gpibADDR);
  disableSMU(gpibHANDLE,"SMU1");
  disableSMU(gpibHANDLE,"SMU2");
  disableSMU(gpibHANDLE,"SMU3");
  disableSMU(gpibHANDLE,"SMU4");
  disableSMU(gpibHANDLE,"SMU5");
}
static void MEASURE(GtkWidget *measBUTTON)
{
  measure(gpibHANDLE);
}

///////////////////////////////////////////
// INITIALIZATION AND MEASURE GENERATION //
///////////////////////////////////////////
static void generateINIT(GTKwrapper* state){
   /* Initialize GPIB button */
  state->initBUTTON = gtk_button_new_with_label("Initialize GPIB");
  //g_signal_connect(state->initBUTTON,"clicked", G_CALLBACK(INITIALIZE_GPIB),NULL);
  gtk_fixed_put(GTK_FIXED(state->fixed), state->initBUTTON, X1, 15);
  gtk_widget_set_size_request(state->initBUTTON, BWIDTH, BHEIGHT);
  
  /* GPIB address selector */
  GtkWidget* adj = (GtkWidget*)gtk_adjustment_new(0,1,30,1,1,0);
  state->gpibBUTTON = gtk_spin_button_new(GTK_ADJUSTMENT(adj),1,2);
  //g_signal_connect(state->gpibBUTTON,"value-changed", G_CALLBACK(SETGPIB), NULL);
  gtk_spin_button_set_digits(GTK_SPIN_BUTTON (state->gpibBUTTON),0);
  gtk_fixed_put(GTK_FIXED(state->fixed), state->gpibBUTTON, 150, 15);
  gtk_widget_set_size_request(state->gpibBUTTON, 100 , BHEIGHT);
    
  /* Measure Button */
  state->measBUTTON = gtk_button_new_with_label("Measure");
  gtk_fixed_put(GTK_FIXED(state->fixed), state->measBUTTON, X6, 15);
  //g_signal_connect(state->measBUTTON,"clicked", G_CALLBACK(MEASURE),NULL);
  gtk_widget_set_size_request(state->measBUTTON, BWIDTH, BHEIGHT);
}

///////////////////////////////////////////////////////////////////////////////////////
//                      SWEEP MODE FUNCTIONS AND CALLBACKS                           // 
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////
// SMU CONTROL CALLBACKS //
///////////////////////////
static int SETSMU(GtkWidget *smuBUTTON,  GTKwrapper *state)
{
  // read everything
  char* _smu    = (char*)gtk_combo_box_text_get_active_text((GtkComboBoxText*)state->SMU[0]);
  char* vtmp    = (char*)gtk_entry_get_text((GtkEntry*)state->SMU[1]); 
  char* itmp    = (char*)gtk_entry_get_text((GtkEntry*)state->SMU[2]); 
  char* mode    = (char*)gtk_combo_box_text_get_active_text((GtkComboBoxText*)state->SMU[3]); 
  char* _var    = (char*)gtk_combo_box_text_get_active_text((GtkComboBoxText*)state->SMU[4]);
  char* cons    = (char*)gtk_entry_get_text((GtkEntry*)state->SMU[5]);
  char* comp    = (char*)gtk_entry_get_text((GtkEntry*)state->SMU[6]);      
 
  gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT (state->listCOMBO));   
  add_to_list_unique(state->comboVARS,strdup(vtmp));
  add_to_list_unique(state->comboVARS,strdup(itmp));
  while (state->comboVARS->next != NULL){
    gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (state->listCOMBO),NULL,state->comboVARS->data);
    state->comboVARS = state->comboVARS->next;
    gtk_combo_box_set_active (GTK_COMBO_BOX (state->listCOMBO), 0);
  }
  // rewind the pointer
  state->comboVARS = state->comboVARS->head;

  // format vname and iname for the gpib
  char* vname;
  char* iname;

  vname = stringify(vtmp);
  iname = stringify(itmp);

  if (! ( (int)strcmp(_var,"CONS") )){
    // Black magick bitwise operations for mode selection
    if (! ((int)strcmp("V", mode) & (int)strcmp(mode,"I"))){
      const char *data[7] = {_smu, vname, iname, mode, _var, cons, comp};
      setSMU(gpibHANDLE, data);
    }
    else {
      const char *data[7] = {_smu, vname, iname, mode, _var, NULL, NULL};
      setSMU(gpibHANDLE, data);
    }
  }
  else {
    const char *data[7] = {_smu, vname, iname, mode, _var, NULL, NULL};
    setSMU(gpibHANDLE, data);
  }
}
static void DISSMU(GtkWidget *disBUTTON,  GTKwrapper *state)
{
  gtk_entry_set_text((GtkEntry*)state->SMU[1],"");
  gtk_entry_set_text((GtkEntry*)state->SMU[2],"");
  char* SMUX = (char*)gtk_combo_box_text_get_active_text((GtkComboBoxText*)state->SMU[0]);
  disableSMU(gpibHANDLE, SMUX);
}


static void smuChanger(GtkWidget *widget, GTKwrapper* state){
  char *tmp1;
  char *tmp2;
  tmp1 = (char*)gtk_combo_box_text_get_active_text((GtkComboBoxText*)state->SMU[3]);
  tmp2 = (char*)gtk_combo_box_text_get_active_text((GtkComboBoxText*)state->SMU[4]);
  
  // If in common constant mode ... turn off everything
  if ((strcmp(tmp1,"COMM") == 0) & (strcmp(tmp2,"CONS") == 0)) {
    gtk_entry_set_text(GTK_ENTRY(state->SMU[5]), "------------------");
    gtk_entry_set_text(GTK_ENTRY(state->SMU[6]), "------------------");
    gtk_editable_set_editable(GTK_EDITABLE(state->SMU[5]), FALSE);
    gtk_editable_set_editable(GTK_EDITABLE(state->SMU[6]), FALSE);
  }

  else if(strcmp(tmp2,"CONS") == 0  ){
    gtk_entry_set_text(GTK_ENTRY(state->SMU[5]), "");
    gtk_editable_set_editable(GTK_EDITABLE(state->SMU[5]), TRUE);
    gtk_entry_set_text(GTK_ENTRY(state->SMU[6]), "");
    gtk_editable_set_editable(GTK_EDITABLE(state->SMU[6]), TRUE);
  }
  else {
    gtk_entry_set_text(GTK_ENTRY(state->SMU[5]), "------------------");
    gtk_entry_set_text(GTK_ENTRY(state->SMU[6]), "------------------");
    gtk_editable_set_editable(GTK_EDITABLE(state->SMU[5]), FALSE);
    gtk_editable_set_editable(GTK_EDITABLE(state->SMU[6]), FALSE);
  }
}

///////////////////////////
// SMU CONTROL GENRATION //
///////////////////////////
static void generateSMU(GTKwrapper* state){
  
  /* set SMU control */
  state->smuBUTTON = gtk_button_new_with_label("OK");
  g_signal_connect(state->smuBUTTON,"clicked", G_CALLBACK(SETSMU), state);
  gtk_fixed_put(GTK_FIXED(state->fixed), state->smuBUTTON, X6, 110);
  gtk_widget_set_size_request(state->smuBUTTON, BWIDTH, BHEIGHT);

  state->disBUTTON = gtk_button_new_with_label("!DISABLE!");
  g_signal_connect(state->disBUTTON,"clicked", G_CALLBACK(DISSMU), state);
  gtk_fixed_put(GTK_FIXED(state->fixed), state->disBUTTON, X6, 180);
  gtk_widget_set_size_request(state->disBUTTON, BWIDTH, BHEIGHT);

  state->SMU = g_new(GtkWidget*, 8);
      
  //GtkListStore *SMULIST = gtk_list_store_new(5, G_TYPE_STRING,G_TYPE_INT,G_TYPE_BOOLEAN);
  state->SMU[0] = gtk_combo_box_text_new();
  gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT(state->SMU[0]),NULL, "SMU1");
  gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT(state->SMU[0]),NULL, "SMU2");
  gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT(state->SMU[0]),NULL, "SMU3");
  gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT(state->SMU[0]),NULL, "SMU4");
  gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT(state->SMU[0]),NULL, "SMU5");
  gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT(state->SMU[0]),NULL, "GNDU");
  gtk_combo_box_set_active(GTK_COMBO_BOX(state->SMU[0]),0);
  gtk_widget_set_size_request(state->SMU[0], BWIDTH, BHEIGHT);
  gtk_fixed_put(GTK_FIXED(state->fixed), state->SMU[0], X1, 110);
 
  // VNAME 
  state->SMU[1] = gtk_entry_new();
  gtk_editable_set_editable(GTK_EDITABLE(state->SMU[1]), TRUE);
  gtk_entry_set_width_chars((GtkEntry*)state->SMU[1],14);
  gtk_widget_set_size_request(state->SMU[1], BWIDTH, BHEIGHT);
  gtk_fixed_put(GTK_FIXED(state->fixed), state->SMU[1], X2, 110);
  state->smuLABEL1 = gtk_label_new("(V) name");
  gtk_fixed_put(GTK_FIXED(state->fixed), state->smuLABEL1, X2, 90);
    
  // INAME
  state->SMU[2] = gtk_entry_new();
  gtk_editable_set_editable(GTK_EDITABLE(state->SMU[2]), TRUE);
  gtk_entry_set_width_chars((GtkEntry*)state->SMU[2],14);
  gtk_widget_set_size_request(state->SMU[2], BWIDTH, BHEIGHT);
  gtk_fixed_put(GTK_FIXED(state->fixed), state->SMU[2], X3, 110);
  state->smuLABEL2 = gtk_label_new("(I) name");
  gtk_fixed_put(GTK_FIXED(state->fixed), state->smuLABEL2, X3, 90);

  // MODE
  state->SMU[3] = gtk_combo_box_text_new ();
  gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (state->SMU[3]),NULL, "V");
  gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (state->SMU[3]),NULL, "I");
  gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (state->SMU[3]),NULL, "COMM");
  gtk_combo_box_set_active (GTK_COMBO_BOX (state->SMU[3]), 0);
  gtk_widget_set_size_request(state->SMU[3], BWIDTH, BHEIGHT);
  gtk_fixed_put(GTK_FIXED(state->fixed), state->SMU[3], X4, 110);
  state->smuLABEL3 = gtk_label_new("Mode");
  gtk_fixed_put(GTK_FIXED(state->fixed), state->smuLABEL3, X4, 90);
  g_signal_connect(state->SMU[3],"changed", G_CALLBACK(smuChanger), state);

  // CONSTANT OR VARIABLE
  state->SMU[4] = gtk_combo_box_text_new ();
  gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (state->SMU[4]),NULL, "CONS");
  gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (state->SMU[4]),NULL, "VAR1");
  gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (state->SMU[4]),NULL, "VAR2");
  gtk_combo_box_set_active (GTK_COMBO_BOX (state->SMU[4]), 0);
  gtk_widget_set_size_request(state->SMU[4], BWIDTH, BHEIGHT);
  gtk_fixed_put(GTK_FIXED(state->fixed), state->SMU[4], X5, 110);
  state->smuLABEL4 = gtk_label_new("VAR Select");
  gtk_fixed_put(GTK_FIXED(state->fixed), state->smuLABEL4, X5, 90);
  g_signal_connect(state->SMU[4],"changed", G_CALLBACK(smuChanger), state);
 

  // CONST VALUE
  state->SMU[5] = gtk_entry_new();
  gtk_editable_set_editable(GTK_EDITABLE(state->SMU[5]), TRUE);
  gtk_entry_set_width_chars((GtkEntry*)state->SMU[5],14);
  gtk_widget_set_size_request(state->SMU[5], BWIDTH, BHEIGHT);
  gtk_fixed_put(GTK_FIXED(state->fixed), state->SMU[5], X4, 180);
  state->smuLABEL5 = gtk_label_new("Const Value");
  gtk_fixed_put(GTK_FIXED(state->fixed), state->smuLABEL5, X4, 160);

  // CONST COMP
  state->SMU[6] = gtk_entry_new();
  gtk_editable_set_editable(GTK_EDITABLE(state->SMU[6]), TRUE);
  gtk_entry_set_width_chars((GtkEntry*)state->SMU[6],14);
  gtk_widget_set_size_request(state->SMU[6], BWIDTH, BHEIGHT);
  gtk_fixed_put(GTK_FIXED(state->fixed), state->SMU[6], X5, 180);
  state->smuLABEL6 = gtk_label_new("Const Compliance");
  gtk_fixed_put(GTK_FIXED(state->fixed), state->smuLABEL6, X5, 160);
}



///////////////////////////////////////////////////////////////////////////////////////
//                            SWEEP MODE CONTROL                                     // 
///////////////////////////////////////////////////////////////////////////////////////
static void destroySWEEPMODE(GTKwrapper* state){
   // DESTROY GPIB
  if ( (state->initBUTTON!=NULL) && (GTK_IS_WIDGET(state->initBUTTON)))
    gtk_widget_destroy (state->initBUTTON);
  if ( (state->measBUTTON!=NULL) && (GTK_IS_WIDGET(state->measBUTTON)))
    gtk_widget_destroy (state->measBUTTON);
  if ( (state->gpibBUTTON!=NULL) && (GTK_IS_WIDGET(state->gpibBUTTON)))
    gtk_widget_destroy (state->gpibBUTTON);
  // DESTROY SMU
  if ( (state->smuBUTTON!=NULL) && (GTK_IS_WIDGET(state->smuBUTTON)))
    gtk_widget_destroy (state->smuBUTTON);
  if ( (state->disBUTTON!=NULL) && (GTK_IS_WIDGET(state->disBUTTON)))
    gtk_widget_destroy (state->disBUTTON);
  if ( (state->initBUTTON!=NULL) && (GTK_IS_WIDGET(state->smuLABEL1)))
    gtk_widget_destroy (state->smuLABEL1);
  if ( (state->initBUTTON!=NULL) && (GTK_IS_WIDGET(state->smuLABEL2)))
    gtk_widget_destroy (state->smuLABEL2);
  if ( (state->initBUTTON!=NULL) && (GTK_IS_WIDGET(state->smuLABEL3)))
    gtk_widget_destroy (state->smuLABEL3);
  if ( (state->initBUTTON!=NULL) && (GTK_IS_WIDGET(state->smuLABEL4)))
    gtk_widget_destroy (state->smuLABEL4);
  if ( (state->initBUTTON!=NULL) && (GTK_IS_WIDGET(state->smuLABEL5)))
    gtk_widget_destroy (state->smuLABEL5);
  if ( (state->initBUTTON!=NULL) && (GTK_IS_WIDGET(state->smuLABEL6)))
    gtk_widget_destroy (state->smuLABEL6);
  int i = 0;
  for (i=0; i<7; i++){
    if ( (state->initBUTTON!=NULL) && (GTK_IS_WIDGET(state->SMU[i])))
      gtk_widget_destroy (state->SMU[i]);
  }
}

static void generateSWEEPMODE (GSimpleAction *action, GVariant*parameter,  void* gui_state)
{  
  GTKwrapper* _state = (GTKwrapper*)malloc(sizeof(GTKwrapper*));
  _state = gui_state; 

  if (_state->MODE != 1){
    generateINIT(_state);
    generateSMU(_state);
  }
  _state->MODE = 1;
  gtk_widget_show_all(_state->window); 
}

///////////////////////////////////////////////////////////////////////////////////////
//                         SAMPLING MODE CONTROL                                     // 
///////////////////////////////////////////////////////////////////////////////////////
static void generateSAMPLINGMODE (GSimpleAction *action,GVariant*parameter, void* gui_state)
{
  GTKwrapper* _state = (GTKwrapper*)malloc(sizeof(GTKwrapper*));
  _state = gui_state; 
  destroySWEEPMODE(_state);
  _state->MODE = 2;
  gtk_widget_show_all(_state->window); 
}

static void quit (GSimpleAction *action, GVariant *parameter, void* gui_state)
{
  GTKwrapper* _state = (GTKwrapper*)malloc(sizeof(GTKwrapper));
  _state = gui_state; 
  g_application_quit((GApplication*)_state->app);
}

static void startup (GtkApplication* app, GTKwrapper* state)
{
  static const GActionEntry actions[] = {
    {"sweepmode",  generateSWEEPMODE},
    {"samplingmode", generateSAMPLINGMODE},
    {"quit", quit}
  };
 
  GMenu *menu;
  GMenu *measMENU;
  menu = g_menu_new ();
  measMENU = g_menu_new ();
  g_menu_append (measMENU, "Sweep", "app.sweepmode");
  g_menu_append (measMENU, "Sampling", "app.samplingmode");
  g_menu_append_submenu (menu,"Measure",G_MENU_MODEL(measMENU));
  g_menu_append (menu, "Quit", "app.quit");

  g_action_map_add_action_entries (G_ACTION_MAP(app), actions, G_N_ELEMENTS(actions), state);
  gtk_application_set_menubar (app, G_MENU_MODEL (menu));
  g_object_unref (menu);
}

static void activate (GtkApplication *app, GTKwrapper* state)
{
  state->NUMBER = 1;
  state->window = gtk_application_window_new (app);
  gtk_window_set_application (GTK_WINDOW (state->window), GTK_APPLICATION (app));
  gtk_window_set_default_size(GTK_WINDOW(state->window), WIDTH, HEIGHT);
  gtk_window_set_title (GTK_WINDOW (state->window), "Hello GNOME");
  gtk_widget_show_all (GTK_WIDGET (state->window));

  state->fixed = gtk_fixed_new();
  gtk_container_add(GTK_CONTAINER(state->window), state->fixed); 
  state->MODE = 0;
}

int main (int argc, char **argv)
{
  GTKwrapper* state = (GTKwrapper*)malloc(sizeof(GTKwrapper));
  state->app = gtk_application_new ("org.gtk.example",G_APPLICATION_FLAGS_NONE);
  g_signal_connect (state->app, "startup", G_CALLBACK (startup), state);
  g_signal_connect (state->app, "activate", G_CALLBACK (activate), state);
  g_application_run (G_APPLICATION (state->app), argc, argv);
  g_object_unref (state->app);
  return 0;
}
