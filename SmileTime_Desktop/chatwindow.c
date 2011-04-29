#include "chatwindow.h"

void println(char* username, int usernamesize, char* str, int size){
	GtkTextIter startIter;
	GtkTextIter endIter;
	gtk_text_buffer_get_iter_at_line(messagebuffer, &startIter, 0);
	gtk_text_buffer_get_iter_at_line(messagebuffer, &endIter, 1);
   gtk_text_buffer_delete(messagebuffer, &startIter, &endIter);

	gtk_text_buffer_get_iter_at_line(messagebuffer, &messageIter, MAX_LINES);

	char msg[usernamesize + size];
	strcpy(msg, username);
	strcat(msg, str);
   gtk_text_buffer_insert(messagebuffer, &messageIter, msg, size + usernamesize);
}

void button_event(GtkButton *button, gpointer   user_data)
{
	if(chatbox->text_length > 0){
		char str[chatbox->text_length+1];
		strncpy(str, chatbox->text, chatbox->text_length);
		str[chatbox->text_length] = '\n';
		println("Me: ", 4, str, chatbox->text_length+1);
		str[chatbox->text_length] = '\0';
		send_text_message(str);
		gtk_entry_set_text(chatbox, "");
	}
}

void * chatwindow_init()//int *argc, char ***argv
{    
    /* This is called in all GTK applications. Arguments are parsed
     * from the command line and are returned to the application. */
    gtk_init (0, NULL);
    
    /* create a new window */
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size (window, WINDOW_WIDTH, WINDOW_HEIGHT);
    
    /* Sets the border width of the window. */
    gtk_container_set_border_width (GTK_CONTAINER (window), 10);
    gtk_window_set_title(window, "smiletime! chat");
		//gtk_window_set_resizable(window, FALSE);
		gtk_window_move(window, 350, 0);
	container = gtk_vbox_new(FALSE ,10);

	button = gtk_button_new_with_label("Send");

	messagebuffer = gtk_text_buffer_new(NULL);
	int i = 0;
	for(; i < MAX_LINES; i++){
		gtk_text_buffer_get_iter_at_line(messagebuffer, &messageIter, MAX_LINES);
		gtk_text_buffer_insert(messagebuffer, &messageIter,"\n", 1);
	}
    messagebox = gtk_text_view_new_with_buffer(messagebuffer);
	messagebox->editable = FALSE;
    chatbox = gtk_entry_new_with_max_length(140);
    

    g_signal_connect (button, "clicked",	G_CALLBACK (button_event), NULL);

   
	 gtk_container_add (GTK_CONTAINER (container), messagebox);
    gtk_container_add (GTK_CONTAINER (container), chatbox);
	gtk_container_add (GTK_CONTAINER (container), button);
    gtk_container_add (GTK_CONTAINER (window), container);

    gtk_widget_show_all(window);    

    gtk_main ();
}
