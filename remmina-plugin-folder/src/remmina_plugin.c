/*
 *     Project: Remmina Plugin FOLDER
 * Description: Remmina protocol plugin to open a folder.
 *      Author: Fabio Castelli (Muflone) <muflone@muflone.com>
 *   Copyright: 2012-2019 Fabio Castelli (Muflone)
 *     License: GPL-2+
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of ERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "plugin_config.h"
#include <remmina/remmina_plugin.h>

// Define the launchers list
static gpointer launchers_list[] = {
  "xdg-open", "Automatically detected",
  "gnome-open", "Open for GNOME",
  "nautilus", "Nautilus",
  "pcmanfm", "PCManFM",
  "dolphin", "Dolphin Browser",
  NULL
};

typedef struct {
  GtkTextView *text_view;
  GtkTextBuffer *text_buffer;
} RemminaPluginData;

static RemminaPluginService *remmina_plugin_service = NULL;

/* Initialize plugin */
static void remmina_plugin_folder_init(RemminaProtocolWidget *gp) {
  TRACE_CALL(__func__);
  RemminaPluginData *gpdata;
  remmina_plugin_service->log_printf("[%s] Plugin init\n", PLUGIN_NAME);
  /* Instance log window widgets */
  gpdata = g_new0(RemminaPluginData, 1);
  gpdata->text_view = GTK_TEXT_VIEW(gtk_text_view_new());
  gtk_text_view_set_editable(gpdata->text_view, FALSE);
  gtk_container_add(GTK_CONTAINER(gp), GTK_WIDGET(gpdata->text_view));
  gpdata->text_buffer = gtk_text_view_get_buffer(gpdata->text_view);
  gtk_text_buffer_set_text(gpdata->text_buffer, PLUGIN_DESCRIPTION, -1);
  gtk_widget_show(GTK_WIDGET(gpdata->text_view));
  /* Save reference to plugin data */
  g_object_set_data_full(G_OBJECT(gp), "plugin-data", gpdata, g_free);
}

/* Open connection */
static gboolean remmina_plugin_folder_open_connection(RemminaProtocolWidget *gp) {
  TRACE_CALL(__func__);
  RemminaFile *remminafile;
  gboolean ret;
  GError *error = NULL;
  gchar *argv[50];
  gint argc;
  gint i;
  GPid pid;

  #define GET_PLUGIN_STRING(value) \
    g_strdup(remmina_plugin_service->file_get_string(remminafile, value))

  remmina_plugin_service->log_printf("[%s] Plugin open connection\n", PLUGIN_NAME);
  remminafile = remmina_plugin_service->protocol_plugin_get_file(gp);
  /* Define command line arguments */
  argc = 0;
  argv[argc++] = GET_PLUGIN_STRING("launcher");
  argv[argc++] = GET_PLUGIN_STRING("server");
  argv[argc++] = NULL;
  /* Spawn external process */
  ret = g_spawn_async(NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, &pid, &error);
  /* Free strings */
  for (i = 0; i < argc; i++) {
    g_free(argv[i]);
  }

  if (ret) {
    remmina_plugin_service->protocol_plugin_signal_connection_opened(gp);
  } else {
    remmina_plugin_service->protocol_plugin_set_error(gp, "%s", error->message);
  }
  return TRUE;
}

/* Close connection */
static gboolean remmina_plugin_folder_close_connection(RemminaProtocolWidget *gp) {
  TRACE_CALL(__func__);
  remmina_plugin_service->log_printf("[%s] Plugin close connection\n", PLUGIN_NAME);
  remmina_plugin_service->protocol_plugin_signal_connection_closed(gp);
  return FALSE;
}

/* Array of RemminaProtocolSetting for basic settings.
 * Each item is composed by:
 * a) RemminaProtocolSettingType for setting type
 * b) Setting name
 * c) Setting description
 * d) Compact disposition
 * e) Values for REMMINA_PROTOCOL_SETTING_TYPE_SELECT or REMMINA_PROTOCOL_SETTING_TYPE_COMBO
 * f) Setting tooltip
 */
static const RemminaProtocolSetting remmina_plugin_folder_basic_settings[] = {
  { REMMINA_PROTOCOL_SETTING_TYPE_TEXT, "server", N_("Startup program"), FALSE, NULL, NULL },
  { REMMINA_PROTOCOL_SETTING_TYPE_SELECT, "launcher", N_("Launcher"), FALSE, launchers_list, NULL },
  { REMMINA_PROTOCOL_SETTING_TYPE_END, NULL, NULL, FALSE, NULL, NULL }
};

/* Protocol plugin definition and features */
static RemminaProtocolPlugin remmina_plugin = {
  REMMINA_PLUGIN_TYPE_PROTOCOL,                 // Type
  PLUGIN_NAME,                                  // Name
  PLUGIN_DESCRIPTION,                           // Description
  GETTEXT_PACKAGE,                              // Translation domain
  PLUGIN_VERSION,                               // Version number
  PLUGIN_APPICON,                               // Icon for normal connection
  PLUGIN_APPICON,                               // Icon for SSH connection
  remmina_plugin_folder_basic_settings,         // Array for basic settings
  NULL,                                         // Array for advanced settings
  REMMINA_PROTOCOL_SSH_SETTING_NONE,            // SSH settings type
  NULL,                                         // Array for available features
  remmina_plugin_folder_init,                   // Plugin initialization
  remmina_plugin_folder_open_connection,        // Plugin open connection
  remmina_plugin_folder_close_connection,       // Plugin close connection
  NULL,                                         // Query for available features
  NULL,                                         // Call a feature
  NULL,                                         // Send a keystroke
  NULL                                          // Screenshot support
};

G_MODULE_EXPORT gboolean remmina_plugin_entry(RemminaPluginService *service) {
  TRACE_CALL(__func__);
  remmina_plugin_service = service;

  if (!service->register_plugin((RemminaPlugin *) &remmina_plugin)) {
    return FALSE;
  }
  return TRUE;
}
