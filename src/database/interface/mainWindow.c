#include "mainWindow.h"

enum
{
	COL_CLIENT = 0,
	NUM_COLS
};

void view_popup_menu_onDoSomething ()
{
  g_print ("Do something!\n");
}

void view_popup_menu (GtkWidget *treeview, GdkEventButton *event, gchar *data)
{
	GtkWidget *menu;
	GtkWidget *moreInfo;
	GtkWidget *start;
	GtkWidget *stop;

	menu = gtk_menu_new();

	start = gtk_menu_item_new_with_label("Start");
	stop = gtk_menu_item_new_with_label("Stop");
	moreInfo = gtk_menu_item_new_with_label("More info...");

	g_signal_connect(start, "activate",
			(GCallback) view_popup_menu_onDoSomething, treeview);
	g_signal_connect(stop, "activate",
			(GCallback) view_popup_menu_onDoSomething, treeview);
	g_signal_connect_swapped(moreInfo, "activate",
			(GCallback) infoClientWindow, (gpointer)data);

	gtk_menu_shell_append(GTK_MENU_SHELL(menu), start);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), stop);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), moreInfo);

	gtk_widget_show_all(menu);

	gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
      			(event != NULL) ? event->button : 0,
      	gdk_event_get_time((GdkEvent*)event));
}

gboolean view_onButtonPressed (GtkWidget *treeview, GdkEventButton *event)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	gchar *data;
	/* un seul clic droit */
	if (event->type == GDK_BUTTON_PRESS  &&  event->button == 3)
	{
		g_print ("Single right click on the tree view.\n");

		if (1)
		{
			GtkTreeSelection *selection;
			// recupere la ligne selectionnee
			selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
 			if (gtk_tree_selection_get_selected(selection, &model, &iter))
			{
				gtk_tree_model_get(model, &iter, COL_CLIENT, &data, -1);
				g_print("Row selected : %s\n", data);
			}

		}

		view_popup_menu(treeview, event, data);
		return TRUE;
  }

  	return FALSE;
}

gboolean view_onPopupMenu (GtkWidget *treeview, gchar *data)
{
	view_popup_menu(treeview, NULL, data);
	return TRUE;
}

GtkTreeModel *create_fill_list_clients(void)
{
	GtkListStore *store;
	GtkTreeIter iter;

	store = gtk_list_store_new (NUM_COLS, G_TYPE_STRING);

	//Ajout et remplissage d'une ligne
	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter, COL_CLIENT, "Celine", -1);

	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter, COL_CLIENT, "Emma", -1);

	int i = 0;
	for (i = 0; i < 100; i++)
	{
		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter, COL_CLIENT, "Kevin", -1);
	}

	return GTK_TREE_MODEL(store);
}

GtkWidget *create_view_model (void)
{
	GtkCellRenderer *renderer;
	GtkTreeModel *model;
	GtkWidget *view;

	view = gtk_tree_view_new();

	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view), -1, "Client",
						renderer, "text", COL_CLIENT, NULL);

	model = create_fill_list_clients();

	gtk_tree_view_set_model(GTK_TREE_VIEW(view), model);

	g_object_unref(model);

	return view;
}


void *mainWindow (void *arg)
{
	GtkWidget *boxV, *boxH;
	GtkWidget *btnGetAll;
	GtkWidget *listClients;
	GtkWidget *mainWindow;
	GtkWidget *text;
	GtkWidget *research;
	GtkWidget *scrollbar;
	
	struct data *d = (struct data*) arg;
	
	gtk_init(0, NULL);

	// Creation de la fenetre principale
	mainWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
 	gtk_window_set_title(GTK_WINDOW(mainWindow), "Keylogger");
	gtk_window_set_default_size(GTK_WINDOW(mainWindow), 1000, 700);
	gtk_window_set_position(GTK_WINDOW(mainWindow), GTK_WIN_POS_CENTER);
	
	// Creation de la liste des clients	
	listClients = create_view_model();
	scrollbar = gtk_scrolled_window_new(NULL, NULL);

	//Recherche d'un client (texte + zone de saisie)
	text = gtk_label_new("Research client :");
	research = gtk_entry_new();
	
	// Creation de la box
	boxV = gtk_vbox_new(FALSE, 0);
	boxH = gtk_hbox_new(TRUE, 0);

        //Creation des boutons Start All et Stop All
	btnGetAll = gtk_button_new_with_label("Get all client");

	// Insertion des widgets
	gtk_container_add(GTK_CONTAINER(mainWindow), boxV);
	gtk_container_add(GTK_CONTAINER(scrollbar), listClients);
	
	gtk_box_pack_start(GTK_BOX(boxV), scrollbar, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(boxV), boxH, FALSE, TRUE, 20);
	gtk_box_pack_start(GTK_BOX(boxH), btnGetAll, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(boxH), text, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(boxH), research, FALSE, FALSE, 0);

	// Gestion des detection des clics dans la liste des clients
  	g_signal_connect(listClients, "button-press-event", (GCallback) 
		  view_onButtonPressed, NULL);
 	g_signal_connect(listClients, "popup-menu", (GCallback) 
		  view_onPopupMenu, NULL);

	
	// Affichage et boucle evenementielle
	if (g_signal_connect(G_OBJECT(mainWindow), "delete_event", 
		  G_CALLBACK(gtk_main_quit), NULL) > 0)
		printf("delete_event OK\n");
	g_signal_connect(G_OBJECT(research), "activate", G_CALLBACK(research_client), NULL);
	if (g_signal_connect(G_OBJECT(btnGetAll), "clicked", G_CALLBACK(get_all_clients), d) > 0)
		printf("get_all_clients interface\n");
	else
		printf("get_all_clients interface failed");
	gtk_widget_show_all(mainWindow);


	gtk_main();
	pthread_exit(NULL);
	return NULL;
}
