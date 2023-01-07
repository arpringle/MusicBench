/*
interface.c

This source file includes the basic code for constructing
the main user interface of MusicBench. It contains 3 main
functions which do this, the first of which is the 
ConstructUI function. ConstructUI is called by main() when
the user interface needs to be constructed. ConstructUI
further calls the other 2 user interface functions:

1) The ConstructWindow function, which builds the bulk
of the user interface components.

2) The ConstructTitleBar functions, which handles the
custom window decorations, which include the media controls
and the tempo/time signature selectors.

There are other various callback functions, explained in
more detail later on.

The only public function is the ConstructUI function.

Check the header file for the "includes."
*/

#include "interface.h" 

/*
The following function is called by main(). It initializes GTK,
and does other basic functionality, like setting up the functionality
of the "X" button.
----
Function type: Public
Parameters: The command line args, so we can pass them to GTK.
Returns: An int. Should return zero if everything goes smoothly.
*/

int ConstructUI(int argc, char *argv[])
{
  //Initialize GTK with command-line args
  gtk_init(&argc, &argv);
  
  //Construct the window and set *MainWindow equal to it
  GtkWidget *MainWindow=ConstructWindow();
  
  //Construct the "about" dialog box
  GtkWidget *AboutDialog=ConstructAbout();

  //Construct the window titlebar and set *TitleBar equal to it
  GtkWidget *TitleBar=ConstructTitleBar();

  // Set the titlebar as the titlebar for the window
  gtk_window_set_titlebar(GTK_WINDOW(MainWindow), TitleBar);

  //Set the callback function for the "X" button
  g_signal_connect(MainWindow, "destroy", G_CALLBACK(WindowClosed), NULL);

  // Show the window and run the GTK main loop
  gtk_widget_show_all(MainWindow);
  gtk_main(); 
  return 0;
}

/*
The following function constructs the bulk of the
user interface layout of the program.
----
Function type: Private
Parameters: None
Output: GtkWidget ptr pointing to the assembled GtkWindow.
*/
static GtkWidget *ConstructWindow()
{
  //To start, we will create the main window:
  GtkWidget *Window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  //Create vertical box to separate the  top bar,
  //(the zoom options and timeline ruler) from the scrollable window:
  GtkWidget *MainBox=gtk_box_new(GTK_ORIENTATION_VERTICAL,0);

  //Create the scrollable window:
  GtkWidget *ScrolledWindow=gtk_scrolled_window_new(NULL,NULL);
  
  //Create the main paned layout, which will be scrollable:
  GtkWidget *MainLayout=gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);

  //Now, we want the user to be able to scale the left pane, and logically,
  //the zoom box appears to be a part of the left pane, so it should scale too.
  //However, we always want the zoom controls and the timeline ruler on screen,
  //no matter where the user has scrolled. This would not be the case if the
  //zoom controls and rulers were part of the MainLayout Pane,
  //as that one can scroll. Therefore, the zoom controls are actually
  //in a separate GtkPaned, which syncs its position with the MainLayout.

  //Create the paned layout to hold the zoom controls and the timeline ruler.
  GtkWidget *TopPanes=gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);

  //These two lines of code connect to the callback function which syncs the 2 GtkPaned layouts.
  g_signal_connect(MainLayout, "notify::position", G_CALLBACK(on_paned_position_changed), TopPanes);
  g_signal_connect(TopPanes, "notify::position", G_CALLBACK(on_paned_position_changed), MainLayout);

  //Attach the main paned layout (which should scroll) to the scrolled window:
  gtk_container_add(GTK_CONTAINER(ScrolledWindow),MainLayout);

  //Attach the scrolled window to the main box:
  gtk_box_pack_end(GTK_BOX(MainBox),ScrolledWindow,TRUE,TRUE,0);
  //Attach the top panes to the main box above the scrolled ones:
  gtk_box_pack_end(GTK_BOX(MainBox),TopPanes,FALSE,FALSE,0);

  //Attach the main box to the window:
  gtk_container_add(GTK_CONTAINER(Window),MainBox);

  //Here we create a box and a frame for the track pane (the left pane)'s contents:
  GtkWidget *TrackPaneBox=gtk_box_new (GTK_ORIENTATION_VERTICAL,0);
  //Set its size:
  gtk_widget_set_size_request(TrackPaneBox, 350, 80);
  //Add the frame (which contains the box) to the left pane:
  gtk_paned_add1(GTK_PANED(MainLayout),TrackPaneBox);

  //Now, we will create 2 boxes inside the TrackPaneBox--

  //Create the box which will hold the track list:
  GtkWidget *TrackListBox=gtk_box_new(GTK_ORIENTATION_VERTICAL,0);

  //This next box holds only one widget, the "Add track" button. It is named accordingly.
  GtkWidget *AddTrackButtonBox=gtk_box_new(GTK_ORIENTATION_VERTICAL,0);

  //These boxes then are both added to the Track Pane:
  gtk_box_pack_start(GTK_BOX(TrackPaneBox),TrackListBox, FALSE  , FALSE, 0);
  gtk_box_pack_start(GTK_BOX(TrackPaneBox),AddTrackButtonBox, FALSE  , FALSE, 0);
  
  //Now, coming back to the TopPanes from earlier, here we will
  //create the left side of that GtkPaned. It contains the timeline view controls.

  //Start out by creating the frame and box that will hold the controls.
  //Also request the proper size:
  GtkWidget *ZoomControlsBoxFrame=gtk_frame_new(NULL);
  GtkWidget *ZoomControlsBox=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
  gtk_container_add(GTK_CONTAINER(ZoomControlsBoxFrame),ZoomControlsBox);
  gtk_widget_set_size_request(ZoomControlsBox, 350, 60);

  //We want a zoom slider, but we want to first label it with an image and label.
  
  //Use the GNOME zoom icon:  
  GtkWidget *ZoomIcon=gtk_image_new_from_icon_name("zoom-fit-best-symbolic",GTK_ICON_SIZE_MENU);
  //Create a label "Zoom":
  GtkWidget *ZoomLabel=gtk_label_new("Zoom");

  //Here is the zoom slider, which goes between 0% zoom and 100% zoom.
  GtkWidget *ZoomSlider=gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 1);

  //We don't want to show the current value of the slider. It will be apparent
  //from the change to the timeline ruler. This line turns it off:
  gtk_scale_set_draw_value (GTK_SCALE(ZoomSlider),FALSE);

  //Set the default value at 50% zoom:
  gtk_range_set_value(GTK_RANGE(ZoomSlider),50);

  //The next element in the zoom control box is a radio button selection.
  //The option given is to choose whether the timeline ruler displays
  //time (minutes/seconds) or measures.
  
  //The radio buttons are in a vertical box:
  GtkWidget *TimeOrMeasuresBox= gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
  //The radio buttons must be in a list so they are aware of each other's state:
  GSList *TimeOrMeasuresButtons = NULL;
  //Create the first of the two radio buttons, the "time" option:
  GtkWidget *TimeRadioButton = gtk_radio_button_new_with_label(NULL, "Time");
  //Set the value of the list according to the first one created:
  TimeOrMeasuresButtons = gtk_radio_button_get_group(GTK_RADIO_BUTTON(TimeRadioButton));
  //Create the second of the two radio buttons, the "measures" option:
  GtkWidget *MeasuresRadioButton = gtk_radio_button_new_with_label(TimeOrMeasuresButtons, "Measures");
  //Pack those radio buttons into the radio button box:
  gtk_box_pack_start(GTK_BOX(TimeOrMeasuresBox),TimeRadioButton, TRUE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(TimeOrMeasuresBox), MeasuresRadioButton, TRUE, FALSE, 0);

  //Pack all of the components of the zoom control into the zoom controls box:
  gtk_box_pack_start(GTK_BOX(ZoomControlsBox),ZoomIcon, FALSE,FALSE ,2);
  gtk_box_pack_start(GTK_BOX(ZoomControlsBox),ZoomLabel,FALSE,FALSE,0);
  gtk_box_pack_start(GTK_BOX(ZoomControlsBox),ZoomSlider,TRUE,TRUE,4);
  gtk_box_pack_start(GTK_BOX(ZoomControlsBox),TimeOrMeasuresBox,FALSE,FALSE,0);

  //Now we add the ZoomControlsBox into the left pane of the TopPanes:
  gtk_paned_add1(GTK_PANED(TopPanes),ZoomControlsBoxFrame);

  //PlaceholderSpinner
  GtkWidget *TestSpinner=gtk_spinner_new();
  gtk_spinner_start(GTK_SPINNER(TestSpinner));
  gtk_paned_add2(GTK_PANED(TopPanes),TestSpinner);
  //PlaceHolderSpinner end

  //Create the button that creates a new track, and set its padding:
  GtkWidget *AddTrackButton=gtk_button_new_with_label("Add track");
  gtk_widget_set_margin_start(AddTrackButton,10);
  gtk_widget_set_margin_end(AddTrackButton,10);
  gtk_widget_set_margin_top(AddTrackButton,2);
  gtk_widget_set_margin_bottom(AddTrackButton,2);

  //Pack the button into its box:
  gtk_box_pack_end (GTK_BOX(AddTrackButtonBox),AddTrackButton,TRUE,FALSE,5);
  
  //Placeholder spinner
  GtkWidget *spinner2=gtk_spinner_new ();

  gtk_paned_add2 (GTK_PANED(MainLayout),spinner2);

  gtk_spinner_start (GTK_SPINNER(spinner2));
  //Placeholder spinner end

  //And we end by returning the newly created interface.
  return Window;
}

/*
This function constructs the "About" Dialog.
It uses the standard GtkAboutDialog, and is triggered by
the "about" button in the PrimaryMenu.
----
Function type: Private
Parameters: None
Returns: GtkWidget ptr pointing to the assembled GtkAboutDialog
*/
static GtkWidget *ConstructAbout()
{
  GtkWidget *AboutMenu = gtk_about_dialog_new();
  gtk_about_dialog_set_program_name (GTK_ABOUT_DIALOG(AboutMenu),"MusicBench");
  gtk_about_dialog_set_version (GTK_ABOUT_DIALOG(AboutMenu),"PRE-ALPHA UI TEST");
  gtk_about_dialog_set_copyright (GTK_ABOUT_DIALOG(AboutMenu),"©2023 Austin Pringle");
  gtk_about_dialog_set_comments (GTK_ABOUT_DIALOG(AboutMenu), "An easy-to-use, open source, cross-platform DAW, designed\n to meet the GNOME Human Interface Guidelines");
  gtk_about_dialog_set_license_type (GTK_ABOUT_DIALOG(AboutMenu), GTK_LICENSE_GPL_3_0);
  const gchar *authors[] = {"Austin Pringle", NULL};
  gtk_about_dialog_set_authors (GTK_ABOUT_DIALOG(AboutMenu), authors);
  gtk_about_dialog_set_logo_icon_name (GTK_ABOUT_DIALOG(AboutMenu), "emblem-music-symbolic");
  return AboutMenu;
}

/*
The following function constructs the titlebar of the
program, using a GtkHeaderBar. The title bar for MusicBench
contains many controls, such as the tempo selector and the
media controls, in a bid to save screen space.
----
Function type: Private
Parameters: None
Output: GtkWidget ptr pointing to the assembled GtkHeaderBar.
*/
static GtkWidget *ConstructTitleBar()
{
  // Create the container widget for the custom titlebar, a GtkHeader:
  GtkWidget *titlebar = gtk_header_bar_new();
  
  //Show window controls:
  gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(titlebar),TRUE);
  
  //Set window title:
  gtk_header_bar_set_title (GTK_HEADER_BAR(titlebar), "MusicBench");

  //Create the media control buttons
  GtkWidget *PauseButton=gtk_button_new_from_icon_name("media-playback-pause-symbolic",GTK_ICON_SIZE_LARGE_TOOLBAR);
  GtkWidget *PlayButton=gtk_button_new_from_icon_name("media-playback-start-symbolic",GTK_ICON_SIZE_LARGE_TOOLBAR);
  GtkWidget *StopButton=gtk_button_new_from_icon_name("media-playback-stop-symbolic",GTK_ICON_SIZE_LARGE_TOOLBAR);
  GtkWidget *RecordButton=gtk_button_new_from_icon_name ("media-record-symbolic", GTK_ICON_SIZE_LARGE_TOOLBAR);
  
  //The loop button is a "toggle" so it is created differently.

  //First, the button is created:
  GtkWidget *LoopButton=gtk_toggle_button_new();
  //Then, we create the image label:
  GtkWidget *LoopSymbol=gtk_image_new() ;
  //Set the image label to the proper icon:
  gtk_image_set_from_icon_name (GTK_IMAGE(LoopSymbol), "view-refresh-symbolic",GTK_ICON_SIZE_LARGE_TOOLBAR);
  //And pack the image into the button:
  gtk_container_add (GTK_CONTAINER(LoopButton),LoopSymbol);
  //Space out the loop button slightly:
  gtk_widget_set_margin_start(LoopButton,20);

  //Create a menu button:
  GtkWidget *MenuButton=gtk_menu_button_new ();
  //Create the image for the menu button:
  GtkWidget *MenuSymbol=gtk_image_new();
  //Set the image to the proper icon:
  gtk_image_set_from_icon_name (GTK_IMAGE(MenuSymbol), "open-menu-symbolic",GTK_ICON_SIZE_SMALL_TOOLBAR);
  //And pack the image into the button:
  gtk_container_add (GTK_CONTAINER(MenuButton),MenuSymbol);

  //Create the associated menu:
  GtkWidget *PrimaryMenu=gtk_popover_menu_new();
  GtkWidget *PrimaryMenuBox=gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
  
  gtk_container_add(GTK_CONTAINER(PrimaryMenu),PrimaryMenuBox);
  
  //Create menu items
  GtkWidget *SaveMenuItem=gtk_menu_item_new_with_label ("Save");
  GtkWidget *SaveAsMenuItem=gtk_menu_item_new_with_label ("Save As...");
  GtkWidget *ExportProjectMenuItem=gtk_menu_item_new_with_label ("Export project");
  GtkWidget *PreferencesMenuItem=gtk_menu_item_new_with_label ("Preferences");
  GtkWidget *AboutMenuItem=gtk_menu_item_new_with_label ("About");

  gtk_box_pack_start(GTK_BOX(PrimaryMenuBox), SaveMenuItem, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(PrimaryMenuBox), SaveAsMenuItem, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(PrimaryMenuBox), ExportProjectMenuItem, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(PrimaryMenuBox), PreferencesMenuItem, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(PrimaryMenuBox), AboutMenuItem, TRUE, TRUE, 0);

  gtk_widget_show_all(PrimaryMenuBox);

  gtk_menu_button_set_popover (GTK_MENU_BUTTON (MenuButton), PrimaryMenu);

  //Create a spin button for the tempo of the project:
  GtkWidget *TempoSelector=gtk_spin_button_new_with_range (0, 400, 1);
  //Set spin button's default value:
  gtk_spin_button_set_value (GTK_SPIN_BUTTON(TempoSelector), 128);
  //Place a label beside it indicating that the adjustment is for BPM:
  GtkWidget *BPMLabel=gtk_label_new("BPM:");
  gtk_widget_set_margin_end(TempoSelector,20);
  
  GtkWidget* TimeSignatureSelector=gtk_combo_box_text_new();

  // Add items to the combo box:
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(TimeSignatureSelector), "4/4");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(TimeSignatureSelector), "6/8");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(TimeSignatureSelector), "3/4");
  gtk_combo_box_set_active(GTK_COMBO_BOX(TimeSignatureSelector), 0);
  gtk_widget_set_margin_end(TimeSignatureSelector,20);
  
  //Pack the media controls and loop button on the left side of the header:
  gtk_header_bar_pack_start(GTK_HEADER_BAR(titlebar), PauseButton);
  gtk_header_bar_pack_start(GTK_HEADER_BAR(titlebar), PlayButton);
  gtk_header_bar_pack_start(GTK_HEADER_BAR(titlebar), StopButton);
  gtk_header_bar_pack_start(GTK_HEADER_BAR(titlebar), RecordButton);
  gtk_header_bar_pack_start(GTK_HEADER_BAR(titlebar), LoopButton);
  
  //Pack the other objects to the right:
  gtk_header_bar_pack_end(GTK_HEADER_BAR(titlebar), MenuButton);
  gtk_header_bar_pack_end(GTK_HEADER_BAR(titlebar), TempoSelector);
  gtk_header_bar_pack_end(GTK_HEADER_BAR(titlebar), BPMLabel);
  gtk_header_bar_pack_end(GTK_HEADER_BAR(titlebar), TimeSignatureSelector);

  return titlebar;
}

//Callback function for clicking the "X" button
static void WindowClosed(GtkWidget *widget, gpointer data)
{
  gtk_main_quit();
}

void on_paned_position_changed(GObject* object, GParamSpec* pspec, gpointer user_data)
{
  GtkPaned* paned = GTK_PANED(object);
  GtkPaned* other_paned = GTK_PANED(user_data);

  // Get the current position of the paned widget
  int position = gtk_paned_get_position(paned);

  // Set the position of the other paned widget to match
  gtk_paned_set_position(other_paned, position);
}