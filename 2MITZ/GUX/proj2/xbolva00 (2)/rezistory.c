#include <gtk/gtk.h>
#include <math.h>

static GtkWidget *bar_count_combo_box;
static GtkWidget *first_bar_combo_box;
static GtkWidget *second_bar_combo_box;
static GtkWidget *third_bar_combo_box;
static GtkWidget *fourth_bar_combo_box;
static GtkWidget *fifth_bar_combo_box;
static GtkWidget *ohm_res_label;
static GtkWidget *tol_res_label;

// Implemention based on:
// http://www.eshop.elektrokomponenty.cz/novinky/barevne-znaceni-rezistoru-odporu/
static void recompute() {
  static const float tols[] = {-1.0, 1.0, 2.0, -1.0, -1.0, 0.5, 0.25, 0.1};

  /* Compute ohms and tolerancy */
  float ohm = 0.0;
  float tol = -1.0;
  switch (gtk_combo_box_get_active(bar_count_combo_box)) {
  case 0: {
    gint first = gtk_combo_box_get_active(first_bar_combo_box);
    gint second = gtk_combo_box_get_active(second_bar_combo_box);
    gint third = gtk_combo_box_get_active(third_bar_combo_box);
    ohm = (10 * first + second) * pow(10, third);
    tol = 20.0;
    break;
  }
  case 1: {
    gint first = gtk_combo_box_get_active(first_bar_combo_box);
    gint second = gtk_combo_box_get_active(second_bar_combo_box);
    gint third = gtk_combo_box_get_active(third_bar_combo_box);
    ohm = (10 * first + second) * pow(10, third);
    gint fourth = gtk_combo_box_get_active(fourth_bar_combo_box);
    tol = tols[fourth];
    break;
  }
  case 2: {
    gint first = gtk_combo_box_get_active(first_bar_combo_box);
    gint second = gtk_combo_box_get_active(second_bar_combo_box);
    gint third = gtk_combo_box_get_active(third_bar_combo_box);
    gint fourth = gtk_combo_box_get_active(fourth_bar_combo_box);
    ohm = (100 * first + 10 * second + third) * pow(10, fourth);
    gint fifth = gtk_combo_box_get_active(fifth_bar_combo_box);
    tol = tols[fifth];
    break;
  }
  }

  /* Set ohms */
  char *ohm_res_str = g_strdup_printf("<span font=\"14\" color=\"red\">"
                                      "<b>%g ohmov</b>"
                                      "</span>",
                                      ohm);
  gtk_label_set_markup(GTK_LABEL(ohm_res_label), ohm_res_str);
  g_free(ohm_res_str);

  /* Set tolerancy */
  char *tol_res_str = tol > 0.0
                          ? g_strdup_printf("<span font=\"14\" color=\"red\">"
                                            "<b>%g %%</b>"
                                            "</span>",
                                            tol)
                          : g_strdup("<span font=\"14\" color=\"red\">"
                                     "<b>-- %%</b>"
                                     "</span>");
  gtk_label_set_markup(GTK_LABEL(tol_res_label), tol_res_str);
  g_free(tol_res_str);
}

static void on_bar_count_changed(GtkComboBox *widget, gpointer user_data) {
  if (!fourth_bar_combo_box || !fifth_bar_combo_box)
    return;
  GtkComboBox *combo_box = widget;
  gint idx = gtk_combo_box_get_active(combo_box);
  /* Temporary make third and fifth combo box active */
  if (idx >= 0) {
    gtk_widget_set_sensitive(fourth_bar_combo_box, TRUE);
    gtk_widget_set_sensitive(fifth_bar_combo_box, TRUE);
  }

  /* Turn off unused combo boxes */
  switch (idx) {
  case 0:
    gtk_widget_set_sensitive(fourth_bar_combo_box, FALSE);
    gtk_widget_set_sensitive(fifth_bar_combo_box, FALSE);
    break;
  case 1:
    gtk_widget_set_sensitive(fifth_bar_combo_box, FALSE);
    break;
  }

  /* Recompute ohms and tolerancy */
  recompute();
}

static void on_bar_changed(GtkComboBox *widget, gpointer user_data) {
  /* Recompute ohms and tolerancy when color of bar is changed */
  recompute();
}

static gboolean on_delete_event(GtkWidget *window, gpointer user_data) {
  /* Show exit dialog */
  GtkWidget *exit_dialog = gtk_message_dialog_new(
      GTK_WINDOW(window), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION,
      GTK_BUTTONS_OK_CANCEL, "Are you sure to quit application?");
  gtk_window_set_title(GTK_WINDOW(exit_dialog), "Quit");
  gint ret = gtk_dialog_run(GTK_DIALOG(exit_dialog));
  gtk_widget_destroy(exit_dialog);
  return ret != GTK_RESPONSE_OK;
}
static void activate(GtkApplication *app, gpointer user_data) {
  /* Create a window with a title, border width, and a default size */
  GtkWidget *window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "Farebné značenie rezistorov");
  gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
  gtk_container_set_border_width(GTK_CONTAINER(window), 10);

  /* Create vertical box */
  GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
  gtk_container_add(GTK_CONTAINER(window), vbox);
  bar_count_combo_box = gtk_combo_box_text_new();

  g_signal_connect(bar_count_combo_box, "changed",
                   G_CALLBACK(on_bar_count_changed), NULL);

  /* Select how many bars a resistor has */
  GtkWidget *bar_counts_label = gtk_label_new("Počet pruhov rezistora");
  const char *bar_counts[] = {"3", "4", "5"};
  for (gint i = 0; i < G_N_ELEMENTS(bar_counts); i++) {
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(bar_count_combo_box),
                                   bar_counts[i]);
  }
  gtk_combo_box_set_active(GTK_COMBO_BOX(bar_count_combo_box), 2);

  /* Set color of bars */
  first_bar_combo_box = gtk_combo_box_text_new();
  GtkWidget *first_bar_label = gtk_label_new("Farba 1. pruhu");
  const char *first_bar[] = {"Čierna", "Hnedá",  "Červená", "Oranžová",
                             "Žltá",   "Zelená", "Modrá",   "Fialová"};
  for (gint i = 0; i < G_N_ELEMENTS(first_bar); i++) {
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(first_bar_combo_box),
                                   first_bar[i]);
  }
  gtk_combo_box_set_active(GTK_COMBO_BOX(first_bar_combo_box), 0);

  g_signal_connect(first_bar_combo_box, "changed", G_CALLBACK(on_bar_changed),
                   NULL);

  second_bar_combo_box = gtk_combo_box_text_new();
  GtkWidget *second_bar_label = gtk_label_new("Farba 2. pruhu");
  const char *second_bar[] = {"Čierna", "Hnedá",  "Červená", "Oranžová",
                              "Žltá",   "Zelená", "Modrá",   "Fialová"};
  for (gint i = 0; i < G_N_ELEMENTS(second_bar); i++) {
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(second_bar_combo_box),
                                   second_bar[i]);
  }
  gtk_combo_box_set_active(GTK_COMBO_BOX(second_bar_combo_box), 0);

  g_signal_connect(second_bar_combo_box, "changed", G_CALLBACK(on_bar_changed),
                   NULL);

  third_bar_combo_box = gtk_combo_box_text_new();
  GtkWidget *third_bar_label = gtk_label_new("Farba 3. pruhu");
  const char *third_bar[] = {"Čierna", "Hnedá",  "Červená", "Oranžová",
                             "Žltá",   "Zelená", "Modrá",   "Fialová"};
  for (gint i = 0; i < G_N_ELEMENTS(third_bar); i++) {
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(third_bar_combo_box),
                                   third_bar[i]);
  }
  gtk_combo_box_set_active(GTK_COMBO_BOX(third_bar_combo_box), 0);

  g_signal_connect(third_bar_combo_box, "changed", G_CALLBACK(on_bar_changed),
                   NULL);

  fourth_bar_combo_box = gtk_combo_box_text_new();
  GtkWidget *fourth_bar_label = gtk_label_new("Farba 4. pruhu");
  const char *fourth_bar[] = {"Čierna", "Hnedá",  "Červená", "Oranžová",
                              "Žltá",   "Zelená", "Modrá",   "Fialová"};
  for (gint i = 0; i < G_N_ELEMENTS(fourth_bar); i++) {
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(fourth_bar_combo_box),
                                   fourth_bar[i]);
  }
  gtk_combo_box_set_active(GTK_COMBO_BOX(fourth_bar_combo_box), 0);

  g_signal_connect(fourth_bar_combo_box, "changed", G_CALLBACK(on_bar_changed),
                   NULL);

  fifth_bar_combo_box = gtk_combo_box_text_new();
  GtkWidget *fifth_bar_label = gtk_label_new("Farba 5. pruhu");
  const char *fifth_bar[] = {"Čierna", "Hnedá",  "Červená", "Oranžová",
                             "Žltá",   "Zelená", "Modrá",   "Fialová"};
  for (gint i = 0; i < G_N_ELEMENTS(fifth_bar); i++) {
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(fifth_bar_combo_box),
                                   fifth_bar[i]);
  }
  gtk_combo_box_set_active(GTK_COMBO_BOX(fifth_bar_combo_box), 0);

  g_signal_connect(fifth_bar_combo_box, "changed", G_CALLBACK(on_bar_changed),
                   NULL);

  GtkWidget *ohm_label = gtk_label_new("Odpor rezistora");
  GtkWidget *tol_label = gtk_label_new("Tolerancia rezistora");

  /* Add items to vertical box */
  gtk_box_pack_start(GTK_BOX(vbox), bar_counts_label, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), bar_count_combo_box, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), first_bar_label, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), first_bar_combo_box, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), second_bar_label, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), second_bar_combo_box, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), third_bar_label, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), third_bar_combo_box, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), fourth_bar_label, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), fourth_bar_combo_box, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), fifth_bar_label, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), fifth_bar_combo_box, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), ohm_label, TRUE, TRUE, 0);

  char *ohm_res_str = g_strdup_printf("<span font=\"14\" color=\"red\">"
                                      "<b>%d ohmov</b>"
                                      "</span>",
                                      0);
  ohm_res_label = gtk_label_new("");
  gtk_label_set_markup(GTK_LABEL(ohm_res_label), ohm_res_str);
  gtk_box_pack_start(GTK_BOX(vbox), ohm_res_label, TRUE, TRUE, 0);
  g_free(ohm_res_str);

  gtk_box_pack_start(GTK_BOX(vbox), tol_label, TRUE, TRUE, 0);

  char *tol_res_str = g_strdup("<span font=\"14\" color=\"red\">"
                               "<b>-- %</b>"
                               "</span>");
  tol_res_label = gtk_label_new("");
  gtk_label_set_markup(GTK_LABEL(tol_res_label), tol_res_str);
  gtk_box_pack_start(GTK_BOX(vbox), tol_res_label, TRUE, TRUE, 0);
  g_free(tol_res_str);

  g_signal_connect(window, "delete-event", G_CALLBACK(on_delete_event), NULL);

  /* Show app window */
  gtk_widget_show_all(window);
}

int main(int argc, char **argv) {
  GtkApplication *app =
      gtk_application_new("org.gtk.rezistory", G_APPLICATION_FLAGS_NONE);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  gint status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);
  return status;
}