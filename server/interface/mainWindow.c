//gcc mainWindow.c -o mainWindow $(pkg-config --libs --cflags gtk+-2.0)
#include <gtk/gtk.h>

enum
{
	COL_CLIENT = 0,
	NUM_COLS
};

void view_popup_menu_onDoSomething (GtkWidget *menuitem, gpointer userdata)
{
    /* we passed the view as userdata when we connected the signal */
    GtkTreeView *treeview = GTK_TREE_VIEW(userdata);

    g_print ("Do something!\n");
}

void view_popup_menu (GtkWidget *treeview, GdkEventButton *event, gpointer userdata)
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
	g_signal_connect(moreInfo, "activate", 
			(GCallback) view_popup_menu_onDoSomething, treeview);

	gtk_menu_shell_append(GTK_MENU_SHELL(menu), start);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), stop);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), moreInfo);
	
	gtk_widget_show_all(menu);
	
	gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
                   (event != NULL) ? event->button : 0,
                   gdk_event_get_time((GdkEvent*)event));
}

gboolean view_onButtonPressed (GtkWidget *treeview, GdkEventButton *event, gpointer userdata)
 {
	GtkTreeModel *model;
	GtkTreeIter iter;

    /* single click with the right mouse button? */
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
		gchar *data;
		gtk_tree_model_get(model, &iter, COL_CLIENT, &data, -1);
		g_print("Row selected : %s\n", data);
	}

      }

      view_popup_menu(treeview, event, userdata);

      return TRUE;
    }

    return FALSE;
}

gboolean view_onPopupMenu (GtkWidget *treeview, gpointer userdata)
{
    view_popup_menu(treeview, NULL, userdata);

    return TRUE;
}

static GtkTreeModel *create_fill_list_clients(void)
{
	GtkListStore *store;
	GtkTreeIter iter;

	store = gtk_list_store_new (NUM_COLS, G_TYPE_STRING);

	//Ajout et remplissage d'une ligne 
	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter, COL_CLIENT, "Celine", -1);

	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter, COL_CLIENT, "Emma", -1);
	
	// A supprimer plus tard 
	int i = 0;
	for (i = 0; i < 100; i++)
	{
		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter, COL_CLIENT, "Kevin", -1);
	}

	return GTK_TREE_MODEL(store); 
}

static GtkWidget *create_view_model (void)
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

void infoClientWindow (void)
{
	GtkWidget *mainWindow;
       	
	// Creation de la fenetre principale
        mainWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_default_size(GTK_WINDOW(mainWindow), 1000, 700);
        gtk_window_set_position(GTK_WINDOW(mainWindow), GTK_WIN_POS_CENTER);
       	
	g_signal_connect(G_OBJECT(mainWindow), "delete_event",
                G_CALLBACK(gtk_main_quit), NULL);
        gtk_widget_show_all(mainWindow);

        gtk_main();

}


void on_changed(GtkWidget *widget)
{
        GtkTreeIter iter;
        GtkTreeModel *model;

        if (gtk_tree_selection_get_selected(GTK_TREE_SELECTION(widget), &model, &iter)) {
                infoClientWindow();
        }
}

int main (int argc, char **argv)
{
	GtkTreeIter iter;
	GtkWidget *listClients;
	GtkWidget *mainWindow;
	GtkTreeModel *model;
	GtkWidget *scrollbar;
	GtkTreeSelection *selection;
	GtkWidget *table;

	
	gtk_init(&argc, &argv);

	// Creation de la fenetre principale
	mainWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(mainWindow), 1000, 700);
	gtk_window_set_position(GTK_WINDOW(mainWindow), GTK_WIN_POS_CENTER);
	
	// Creation de la liste des clients	
	listClients = create_view_model();
	scrollbar = gtk_scrolled_window_new(NULL, NULL);
	
	// Creation de la table
	table = gtk_table_new(10, 7, TRUE);

	// Gestion des detection des clics dans la liste des clients
	selection  = gtk_tree_view_get_selection(GTK_TREE_VIEW(listClients));
	//g_signal_connect(selection, "changed", G_CALLBACK(on_changed), NULL);	
    	g_signal_connect(listClients, "button-press-event", (GCallback) 
		view_onButtonPressed, NULL);
    	g_signal_connect(listClients, "popup-menu", (GCallback) 
		view_onPopupMenu, NULL);
	// Insertion des widgets
	gtk_container_add(GTK_CONTAINER(mainWindow), GTK_WIDGET(table));
	gtk_container_add(GTK_CONTAINER(scrollbar), listClients);
	gtk_table_attach_defaults(GTK_TABLE(table), scrollbar, 0, 3, 0, 7);


	// Affichage et boucle evenementielle
	g_signal_connect(G_OBJECT(mainWindow), "delete_event", 
		G_CALLBACK(gtk_main_quit), NULL);
	gtk_widget_show_all(mainWindow);

	gtk_main();

	return 0;
}
