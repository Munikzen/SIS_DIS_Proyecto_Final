#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <atmi.h>
#include <fml32.h>
#include "../inc/biblioBD.fml.h"

typedef struct streamingApp {
    int id;
    char titulo[20];
    char director[20];
    char estudio[20];
} streamingApp;

GtkTextBuffer *buffer;

void btnSubmitCallback_select(GtkWidget *widget, gpointer data);
void btnSubmitCallback_insert(GtkWidget *widget, gpointer data);
void btnSubmitCallback_update(GtkWidget *widget, gpointer data);
void btnSubmitCallback_delete(GtkWidget *widget, gpointer data);
void btnSubmitCallback_selectAll(GtkWidget *widget, gpointer data);
void handle_delete(int id);
void handle_insert(streamingApp *str);
void handle_update(streamingApp *str);
void handle_select(int id);
void handle_selectAll();
void append_to_text_view(const gchar *text);

void switch_window(GtkWidget *widget, gpointer data);

void create_agregar_window();
void create_modificar_window();
void create_eliminar_window();
void create_listar_window();

GtkWidget *entryId;
GtkWidget *entryTitulo;
GtkWidget *entryDirector;
GtkWidget *entryEstudio;

int main(int argc, char **argv) {
    GtkWidget *window;
    GtkWidget *mainBox;
    GtkWidget *headerBox;
    GtkWidget *buttonAgregar;
    GtkWidget *buttonModificar;
    GtkWidget *buttonEliminar;
    GtkWidget *buttonListar;
    GtkWidget *box;
    GtkWidget *label;
    GtkWidget *entry;
    GtkWidget *buttonBuscar;
    GtkWidget *textView;

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "StreamingApp");
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    int new_height = 400;
    int new_width = 700;
    gtk_widget_set_size_request(GTK_WIDGET(window), new_width, new_height);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    mainBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), mainBox);

    headerBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(mainBox), headerBox, FALSE, FALSE, 0);

    buttonAgregar = gtk_button_new_with_label("Agregar");
    g_signal_connect(buttonAgregar, "clicked", G_CALLBACK(switch_window), "Agregar");
    gtk_box_pack_start(GTK_BOX(headerBox), buttonAgregar, FALSE, FALSE, 0);

    buttonModificar = gtk_button_new_with_label("Modificar");
    g_signal_connect(buttonModificar, "clicked", G_CALLBACK(switch_window), "Modificar");
    gtk_box_pack_start(GTK_BOX(headerBox), buttonModificar, FALSE, FALSE, 0);

    buttonEliminar = gtk_button_new_with_label("Eliminar");
    g_signal_connect(buttonEliminar, "clicked", G_CALLBACK(switch_window), "Eliminar");
    gtk_box_pack_start(GTK_BOX(headerBox), buttonEliminar, FALSE, FALSE, 0);

    buttonListar = gtk_button_new_with_label("Listar");
    g_signal_connect(buttonListar, "clicked", G_CALLBACK(switch_window), "Listar");
    gtk_box_pack_start(GTK_BOX(headerBox), buttonListar, FALSE, FALSE, 0);

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(mainBox), box, FALSE, FALSE, 0);

    label = gtk_label_new("Id Pelicula a buscar:");
    gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);

    entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(box), entry, FALSE, FALSE, 0);

    buttonBuscar = gtk_button_new_with_label("Buscar");
    g_signal_connect(buttonBuscar, "clicked", G_CALLBACK(btnSubmitCallback_select), (gpointer)entry);
    gtk_box_pack_start(GTK_BOX(box), buttonBuscar, FALSE, FALSE, 0);

    textView = gtk_text_view_new();
    gtk_box_pack_start(GTK_BOX(box), textView, TRUE, TRUE, 0);
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textView));

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}

void btnSubmitCallback_select(GtkWidget *widget, gpointer data) {
    GtkWidget *entry = (GtkWidget *)data;

    int zaL_intId = atoi(gtk_entry_get_text(GTK_ENTRY(entry)));

    g_print("Id de la pelicula: %d\n", zaL_intId);

    handle_select(zaL_intId);
}

void handle_select(int id) {
    FBFR32 *fbfr;
    FBFR32 *recv;
    long ivL_tamLongt;
    int ivL_numOcurr;
    streamingApp *spL_strRegis;

    if (tpinit((TPINIT *)NULL) == -1) {
        g_print("Error en la conexión, tperrno = %d \n", tperrno);
        return;
    }

    if ((fbfr = (FBFR32 *)tpalloc("FML32", NULL, 1024)) == NULL) {
        g_print("Error reservando espacio para Buffer fbfrn");
        tpterm();
        return;
    }

    if ((recv = (FBFR32 *)tpalloc("FML32", NULL, 1024)) == NULL) {
        g_print("Error Reservando espacio para Buffer recv\n");
        tpterm();
        return;
    }

    if (Fadd32(fbfr, IDPELI, (char *)&id, 0) < 0) {
        g_print("\tError insertando campo FML (id)\n");
        tpfree((char *)fbfr);
        tpterm();
        return;
    }

    if (tpcall("select_Negocio_FML", (char *)fbfr, 0, (char **)&recv, &ivL_tamLongt, 0L) == -1) {
        g_print("Error en la llamada al servicio: tperrno = %d\n", tperrno);
        tpfree((char *)fbfr);
        tpfree((char *)recv);
        tpterm();
        return;
    }

    if ((ivL_numOcurr = Foccur32(recv, IDPELI)) < 0) {
        g_print("Error en Foccur32\n");
        tpfree((char *)fbfr);
        tpfree((char *)recv);
        tpterm();
        return;
    }

    spL_strRegis = (streamingApp *)malloc(sizeof(streamingApp) * ivL_numOcurr);

    gtk_text_buffer_set_text(buffer, "", -1);

    int i;
    for (i = 0; i < ivL_numOcurr; i++) {
        Fget32(recv, IDPELI, i, (char *)&spL_strRegis[i].id, 0);
        Fget32(recv, NOMBREPELI, i, (char *)spL_strRegis[i].titulo, 0);
        Fget32(recv, DIRECTORPELI, i, (char *)spL_strRegis[i].director, 0);
        Fget32(recv, ESTUDIODIR, i, (char *)spL_strRegis[i].estudio, 0);

        gchar *record_str = g_strdup_printf("Registro %d: ID Pelicula: %d, Titulo: %s, Director: %s, Estudio: %s\n",
                                            i + 1, spL_strRegis[i].id, spL_strRegis[i].titulo, spL_strRegis[i].director, spL_strRegis[i].estudio);
        append_to_text_view(record_str);
        g_free(record_str);
    }

    tpfree((char *)fbfr);
    tpfree((char *)recv);
    tpterm();
}

void handle_insert(streamingApp *str){
    FBFR32 *fbfr;
    FBFR32 *recv;
    FLDLEN32 flen;
    char msgbuf[64];
    long ivL_tamLongt;
    
    if(tpinit((TPINIT *) NULL) == -1) {
        g_print("Error en la conexión, tperrno = %d \n", tperrno);
        return;
    }    

    printf("Reservamos espacio para los buffers FML\n");
    if ((fbfr = (FBFR32 *) tpalloc("FML32", NULL, 1024)) == NULL){
        printf("Error reservando espacio para Buffer fbfrn");
        tpterm();
        return;
    }

    if ((recv = (FBFR32 *) tpalloc("FML32", NULL, 1024)) == NULL){
        printf("Error Reservando espacio para Buffer recv\n");
        tpterm();
        return;
    }

    printf ("\nInsertamos datos en buffer FML.");
    if(Fadd32 (fbfr, IDPELI, (char *)&(str->id), 0) < 0){
        printf ("\n\tError insertando campo FML (idDir)");
        tpfree((char*)fbfr);
        tpterm();
        return;
    }

    if(Fadd32 (fbfr, NOMBREPELI, str->titulo, 0) < 0){
        printf ("\n\tError insertando campo FML (nombreDir)");
        tpfree((char*)fbfr);
        tpterm();
        return;
    }

    if(Fadd32 (fbfr, DIRECTORPELI, str->director, 0) < 0){
        printf ("\n\tError insertando campo FML (nacionalidad)");
        tpfree((char*)fbfr);
        tpterm();
        return;
    }

    if(Fadd32 (fbfr, ESTUDIODIR, str->estudio, 0) < 0){
        printf ("\n\tError insertando campo FML (estudioDir)");
        tpfree((char*)fbfr);
        tpterm();
        return;
    }
   
    printf("Llamada al servicio 'insert_Negocio_FML'\n");
    if(tpcall("insert_Negocio_FML", (char *)fbfr, 0, (char **)&recv, &ivL_tamLongt, 0L) == -1){
        printf("\n Error en la llamada al servicio: tperrno = %d\n", tperrno);
        tpfree((char *)fbfr);
        tpfree((char *)recv);
        tpterm();
        return;
    }
    
    flen = sizeof(msgbuf);
    Fget32(recv, OUTPUT, 0, (char *)msgbuf, &flen);
    printf("Respuesta del servidor: %s\n", msgbuf);

    printf("Liberamos Buffer y desconectamos de la aplicacion\n");
    tpfree((char *)fbfr);
    tpfree((char *)recv);
    tpterm();
}

void btnSubmitCallback_insert(GtkWidget *widget, gpointer data){
    GtkWidget *windowAgregar = (GtkWidget *)data;
    const gchar *id = gtk_entry_get_text(GTK_ENTRY(entryId));
    const gchar *titulo = gtk_entry_get_text(GTK_ENTRY(entryTitulo));
    const gchar *director = gtk_entry_get_text(GTK_ENTRY(entryDirector));
    const gchar *estudio = gtk_entry_get_text(GTK_ENTRY(entryEstudio));

    g_print("ID de pelicula: %s\n", id);
    g_print("Titulo: %s\n", titulo);
    g_print("Director: %s\n", director);
    g_print("Estudio: %s\n", estudio);

    streamingApp str;
    str.id = atoi(id);
    strncpy(str.titulo, titulo, sizeof(str.titulo));
    strncpy(str.director, director, sizeof(str.director));
    strncpy(str.estudio, estudio, sizeof(str.estudio));

    handle_insert(&str);

    gtk_widget_destroy(windowAgregar);
}

void handle_update(streamingApp *str){
    FBFR32 *fbfr;
    FBFR32 *recv;
    FLDLEN32 flen;
    char msgbuf[64];
    long ivL_tamLongt;
    
    if(tpinit((TPINIT *) NULL) == -1) {
        g_print("Error en la conexión, tperrno = %d \n", tperrno);
        return;
    }    

    printf("Reservamos espacio para los buffers FML\n");
    if ((fbfr = (FBFR32 *) tpalloc("FML32", NULL, 1024)) == NULL){
        printf("Error reservando espacio para Buffer fbfrn");
        tpterm();
        return;
    }

    if ((recv = (FBFR32 *) tpalloc("FML32", NULL, 1024)) == NULL){
        printf("Error Reservando espacio para Buffer recv\n");
        tpterm();
        return;
    }

    printf ("\nInsertamos datos en buffer FML.");
    if(Fadd32 (fbfr, IDPELI, (char *)&(str->id), 0) < 0){
        printf ("\n\tError insertando campo FML (id)");
        tpfree((char*)fbfr);
        tpterm();
        return;
    }

    if(Fadd32 (fbfr, NOMBREPELI, str->titulo, 0) < 0){
        printf ("\n\tError actualizando campo FML (titulo)");
        tpfree((char*)fbfr);
        tpterm();
        return;
    }

    if(Fadd32 (fbfr, DIRECTORPELI, str->director, 0) < 0){
        printf ("\n\tError actualizando campo FML (director)");
        tpfree((char*)fbfr);
        tpterm();
        return;
    }

    if(Fadd32 (fbfr, ESTUDIODIR, str->estudio, 0) < 0){
        printf ("\n\tError actualizando campo FML (estudio)");
        tpfree((char*)fbfr);
        tpterm();
        return;
    }
   
    printf("Llamada al servicio 'update_Negocio_FML'\n");
    if(tpcall("update_Negocio_FML", (char *)fbfr, 0, (char **)&recv, &ivL_tamLongt, 0L) == -1){
        printf("\n Error en la llamada al servicio: tperrno = %d\n", tperrno);
        tpfree((char *)fbfr);
        tpfree((char *)recv);
        tpterm();
        return;
    }
    
    flen = sizeof(msgbuf);
    Fget32(recv, OUTPUT, 0, (char *)msgbuf, &flen);
    printf("Respuesta del servidor: %s\n", msgbuf);

    printf("Liberamos Buffer y desconectamos de la aplicacion\n");
    tpfree((char *)fbfr);
    tpfree((char *)recv);
    tpterm();
}

void btnSubmitCallback_update(GtkWidget *widget, gpointer data){
    GtkWidget *windowModificar = (GtkWidget *)data;
    const gchar *id = gtk_entry_get_text(GTK_ENTRY(entryId));
    const gchar *titulo = gtk_entry_get_text(GTK_ENTRY(entryTitulo));
    const gchar *director = gtk_entry_get_text(GTK_ENTRY(entryDirector));
    const gchar *estudio = gtk_entry_get_text(GTK_ENTRY(entryEstudio));

    g_print("ID de pelicula: %s\n", id);
    g_print("Titulo: %s\n", titulo);
    g_print("Director: %s\n", director);
    g_print("Estudio: %s\n", estudio);

    streamingApp str;
    str.id = atoi(id);
    strncpy(str.titulo, titulo, sizeof(str.titulo));
    strncpy(str.director, director, sizeof(str.director));
    strncpy(str.estudio, estudio, sizeof(str.estudio));

    handle_update(&str);

    gtk_widget_destroy(windowModificar);
}

void btnSubmitCallback_delete(GtkWidget *widget, gpointer data){
    GtkWidget *windowEliminar = (GtkWidget *)data;
    const gchar *id = gtk_entry_get_text(GTK_ENTRY(entryId));

    int zaL_intId = atoi(id);

    g_print("Id de la pelicula: %d\n", zaL_intId);

    handle_delete(zaL_intId);

    gtk_widget_destroy(windowEliminar);
}

void handle_delete(int id){
    FBFR32 *fbfr;
    FBFR32 *recv;
    long ivL_tamLongt;
    int ivL_numOcurr;
    // streamingApp *spL_strRegis;

    if (tpinit((TPINIT *)NULL) == -1) {
        g_print("Error en la conexión, tperrno = %d \n", tperrno);
        return;
    }

    if ((fbfr = (FBFR32 *)tpalloc("FML32", NULL, 1024)) == NULL) {
        g_print("Error reservando espacio para Buffer fbfrn");
        tpterm();
        return;
    }

    if ((recv = (FBFR32 *)tpalloc("FML32", NULL, 1024)) == NULL) {
        g_print("Error Reservando espacio para Buffer recv\n");
        tpterm();
        return;
    }

    if (Fadd32(fbfr, IDPELI, (char *)&id, 0) < 0) {
        g_print("\tError borrando campo FML (id)\n");
        tpfree((char *)fbfr);
        tpterm();
        return;
    }

    if (tpcall("delete_Negocio_FML", (char *)fbfr, 0, (char **)&recv, &ivL_tamLongt, 0L) == -1) {
        g_print("Error en la llamada al servicio: tperrno = %d\n", tperrno);
        tpfree((char *)fbfr);
        tpfree((char *)recv);
        tpterm();
        return;
    }

    if ((ivL_numOcurr = Foccur32(recv, IDPELI)) < 0) {
        g_print("Error en Foccur32\n");
        tpfree((char *)fbfr);
        tpfree((char *)recv);
        tpterm();
        return;
    }

    tpfree((char *)fbfr);
    tpfree((char *)recv);
    tpterm();
}

void append_to_text_view(const gchar *text) {
    GtkTextIter iter;

    gtk_text_buffer_get_end_iter(buffer, &iter);

    gtk_text_buffer_insert(buffer, &iter, text, -1);
}

void switch_window(GtkWidget *widget, gpointer data) {
    gchar *window_name = (gchar *)data;

    if (g_strcmp0(window_name, "Agregar") == 0) {
        create_agregar_window();
    } else if (g_strcmp0(window_name, "Modificar") == 0) {
        create_modificar_window();
    } else if (g_strcmp0(window_name, "Eliminar") == 0) {
        create_eliminar_window();
    } else if (g_strcmp0(window_name, "Listar") == 0) {
        create_listar_window();
    }
}

void create_agregar_window() {
    GtkWidget *windowAgregar;
    GtkWidget *boxAgregar;
    GtkWidget *labelId;
    GtkWidget *labelTitulo;
    GtkWidget *labelDirector;
    GtkWidget *labelEstudio;
    GtkWidget *button;

    windowAgregar = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(windowAgregar), "Agregar");
    gtk_container_set_border_width(GTK_CONTAINER(windowAgregar), 10);
    int new_height = 400;
    int new_width = 700;
    gtk_widget_set_size_request(GTK_WIDGET(windowAgregar), new_width, new_height);

    boxAgregar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(windowAgregar), boxAgregar);

    labelId = gtk_label_new("ID de la pelicula:");
    gtk_box_pack_start(GTK_BOX(boxAgregar), labelId, FALSE, FALSE, 0);
    entryId = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(boxAgregar), entryId, FALSE, FALSE, 0);

    labelTitulo = gtk_label_new("Titulo:");
    gtk_box_pack_start(GTK_BOX(boxAgregar), labelTitulo, FALSE, FALSE, 0);
    entryTitulo = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(boxAgregar), entryTitulo, FALSE, FALSE, 0);

    labelDirector = gtk_label_new("Director:");
    gtk_box_pack_start(GTK_BOX(boxAgregar), labelDirector, FALSE, FALSE, 0);
    entryDirector = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(boxAgregar), entryDirector, FALSE, FALSE, 0);

    labelEstudio = gtk_label_new("Estudio:");
    gtk_box_pack_start(GTK_BOX(boxAgregar), labelEstudio, FALSE, FALSE, 0);
    entryEstudio = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(boxAgregar), entryEstudio, FALSE, FALSE, 0);

    button = gtk_button_new_with_label("Insertar");
    g_signal_connect(button, "clicked", G_CALLBACK(btnSubmitCallback_insert), 
                     (gpointer)windowAgregar);
    gtk_box_pack_start(GTK_BOX(boxAgregar), button, FALSE, FALSE, 0);

    gtk_widget_show_all(windowAgregar);
}

void create_modificar_window() {
    GtkWidget *windowModificar;
    GtkWidget *boxModificar;
    GtkWidget *labelId;
    GtkWidget *labelTitulo;
    GtkWidget *labelDirector;
    GtkWidget *labelEstudio;
    GtkWidget *button;

    windowModificar = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(windowModificar), "Modificar");
    gtk_container_set_border_width(GTK_CONTAINER(windowModificar), 10);
    int new_height = 400;
    int new_width = 700;
    gtk_widget_set_size_request(GTK_WIDGET(windowModificar), new_width, new_height);

    boxModificar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(windowModificar), boxModificar);

    labelId = gtk_label_new("ID de la pelicula:");
    gtk_box_pack_start(GTK_BOX(boxModificar), labelId, FALSE, FALSE, 0);
    entryId = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(boxModificar), entryId, FALSE, FALSE, 0);

    labelTitulo = gtk_label_new("Titulo:");
    gtk_box_pack_start(GTK_BOX(boxModificar), labelTitulo, FALSE, FALSE, 0);
    entryTitulo = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(boxModificar), entryTitulo, FALSE, FALSE, 0);

    labelDirector = gtk_label_new("Director:");
    gtk_box_pack_start(GTK_BOX(boxModificar), labelDirector, FALSE, FALSE, 0);
    entryDirector = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(boxModificar), entryDirector, FALSE, FALSE, 0);

    labelEstudio = gtk_label_new("Estudio:");
    gtk_box_pack_start(GTK_BOX(boxModificar), labelEstudio, FALSE, FALSE, 0);
    entryEstudio = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(boxModificar), entryEstudio, FALSE, FALSE, 0);

    button = gtk_button_new_with_label("Actualiza");
    g_signal_connect(button, "clicked", G_CALLBACK(btnSubmitCallback_update), 
                     (gpointer)windowModificar);
    gtk_box_pack_start(GTK_BOX(boxModificar), button, FALSE, FALSE, 0);

    gtk_widget_show_all(windowModificar);
}

void create_eliminar_window() {
    GtkWidget *windowEliminar;
    GtkWidget *boxEliminar;
    GtkWidget *labelId;
    GtkWidget *labelTitulo;
    GtkWidget *labelDirector;
    GtkWidget *labelEstudio;
    GtkWidget *button;

    windowEliminar = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(windowEliminar), "Eliminar");
    gtk_container_set_border_width(GTK_CONTAINER(windowEliminar), 10);
    int new_height = 400;
    int new_width = 700;
    gtk_widget_set_size_request(GTK_WIDGET(windowEliminar), new_width, new_height);

    boxEliminar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(windowEliminar), boxEliminar);

    labelId = gtk_label_new("ID de la pelicula:");
    gtk_box_pack_start(GTK_BOX(boxEliminar), labelId, FALSE, FALSE, 0);
    entryId = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(boxEliminar), entryId, FALSE, FALSE, 0);

    button = gtk_button_new_with_label("Elimina");
    g_signal_connect(button, "clicked", G_CALLBACK(btnSubmitCallback_delete), 
                     (gpointer)windowEliminar);
    gtk_box_pack_start(GTK_BOX(boxEliminar), button, FALSE, FALSE, 0);


    gtk_widget_show_all(windowEliminar);
}

void create_listar_window() {
    GtkWidget *windowListar;
    GtkWidget *boxListar;
    GtkWidget *button;
    GtkWidget *textView;

    windowListar = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(windowListar), "Listar");
    gtk_container_set_border_width(GTK_CONTAINER(windowListar), 10);
    int new_height = 400;
    int new_width = 700;
    gtk_widget_set_size_request(GTK_WIDGET(windowListar), new_width, new_height);

    boxListar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(windowListar), boxListar);

    button = gtk_button_new_with_label("Muestra");
    g_signal_connect(button, "clicked", G_CALLBACK(btnSubmitCallback_selectAll), 
                     (gpointer)windowListar);

    gtk_box_pack_start(GTK_BOX(boxListar), button, FALSE, FALSE, 0);

    textView = gtk_text_view_new();
    gtk_box_pack_start(GTK_BOX(boxListar), textView, TRUE, TRUE, 0);
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textView));

    gtk_widget_show_all(windowListar);
}

void btnSubmitCallback_selectAll(GtkWidget *widget, gpointer data){
    handle_selectAll();
}

void handle_selectAll(){
    if (tpinit((TPINIT *)NULL) == -1) {
        g_print("Error en la conexión, tperrno = %d \n", tperrno);
        return;
    }

    FBFR32 *fbfr;
    FBFR32 *recv;
    long ivL_tamLongt;
    int ivL_numOcurr;
    streamingApp *spL_strRegis;

    if ((fbfr = (FBFR32 *)tpalloc("FML32", NULL, 1024)) == NULL) {
        g_print("Error reservando espacio para Buffer fbfrn");
        tpterm();
        return;
    }

    if ((recv = (FBFR32 *)tpalloc("FML32", NULL, 1024)) == NULL) {
        g_print("Error Reservando espacio para Buffer recv\n");
        tpterm();
        return;
    }

    if (tpcall("selectAll_Negocio_FML", (char *)fbfr, 0, (char **)&recv, &ivL_tamLongt, 0L) == -1) {
        g_print("Error en la llamada al servicio: tperrno = %d\n", tperrno);
        tpfree((char *)fbfr);
        tpfree((char *)recv);
        tpterm();
        return;
    }

    if ((ivL_numOcurr = Foccur32(recv, IDPELI)) < 0) {
        g_print("Error en Foccur32\n");
        tpfree((char *)fbfr);
        tpfree((char *)recv);
        tpterm();
        return;
    }

    spL_strRegis = (streamingApp *)malloc(sizeof(streamingApp) * ivL_numOcurr);

    gtk_text_buffer_set_text(buffer, "", -1);

    int i;
    for (i = 0; i < ivL_numOcurr; i++) {
        Fget32(recv, IDPELI, i, (char *)&spL_strRegis[i].id, 0);
        Fget32(recv, NOMBREPELI, i, (char *)spL_strRegis[i].titulo, 0);
        Fget32(recv, DIRECTORPELI, i, (char *)spL_strRegis[i].director, 0);
        Fget32(recv, ESTUDIODIR, i, (char *)spL_strRegis[i].estudio, 0);

        gchar *record_str = g_strdup_printf("Registro %d: ID Pelicula: %d, Titulo: %s, Director: %s, Estudio: %s\n",
                                            i + 1, spL_strRegis[i].id, spL_strRegis[i].titulo, spL_strRegis[i].director, spL_strRegis[i].estudio);
        append_to_text_view(record_str);
        g_free(record_str);
    }

    tpfree((char *)fbfr);
    tpfree((char *)recv);
    tpterm();
}
