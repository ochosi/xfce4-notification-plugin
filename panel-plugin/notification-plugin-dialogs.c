/*  xfce4-notification-plugin
 *
 *  Copyright (C) 2017 Simon Steinbeiß <simon@xfce.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <gtk/gtk.h>

#include <xfconf/xfconf.h>
#include <libxfce4ui/libxfce4ui.h>
#include <libxfce4panel/xfce-panel-plugin.h>

#include "notification-plugin.h"
#include "notification-plugin-log.h"
#include "notification-plugin-dialogs.h"

/* the website url */
#define PLUGIN_WEBSITE "https://docs.xfce.org/apps/notifyd/start"



static void
notification_plugin_configure_response (GtkWidget    *dialog,
                           gint          response,
                           NotificationPlugin *notification_plugin)
{
  gboolean result;

  if (response == GTK_RESPONSE_HELP)
    {
      /* show help */
      result = g_spawn_command_line_async ("exo-open --launch WebBrowser " PLUGIN_WEBSITE, NULL);

      if (G_UNLIKELY (result == FALSE))
        g_warning (_("Unable to open the following url: %s"), PLUGIN_WEBSITE);
    }
  else
    {
      /* remove the dialog data from the plugin */
      g_object_set_data (G_OBJECT (notification_plugin->plugin), "dialog", NULL);

      /* unlock the panel menu */
      xfce_panel_plugin_unblock_menu (notification_plugin->plugin);

      /* destroy the properties dialog */
      gtk_widget_destroy (dialog);
    }
}



void
notification_plugin_configure (XfcePanelPlugin      *plugin,
                               NotificationPlugin   *notification_plugin)
{
  GtkWidget *dialog;
  GtkWidget *box, *spin, *label;
  GtkAdjustment *adjustment;
  gdouble log_display_limit;

  /* block the plugin menu */
  xfce_panel_plugin_block_menu (plugin);

  /* create the dialog */
  dialog = xfce_titled_dialog_new_with_buttons (_("Notification Plugin"),
                                                GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (plugin))),
                                                GTK_DIALOG_DESTROY_WITH_PARENT,
                                                "gtk-help", GTK_RESPONSE_HELP,
                                                "gtk-close", GTK_RESPONSE_OK,
                                                NULL);
  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
  gtk_window_set_icon_name (GTK_WINDOW (dialog), ICON_NAME);
  gtk_widget_show (dialog);

  log_display_limit = xfconf_channel_get_int (notification_plugin->channel,
                                              SETTING_LOG_DISPLAY_LIMIT, DEFAULT_LOG_DISPLAY_LIMIT);
  adjustment = gtk_adjustment_new (log_display_limit, 0.0, 100.0, 1.0, 5.0, 0.0);
  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_container_add_with_properties (GTK_CONTAINER (gtk_dialog_get_content_area (GTK_DIALOG (dialog))),
						                         box, "expand", TRUE, "fill", TRUE, NULL);
  label = gtk_label_new (_("Number of notifications to show: "));
  gtk_box_pack_start (GTK_BOX (box), GTK_WIDGET (label), TRUE, FALSE, 0);
  spin = gtk_spin_button_new (adjustment, 1.0, 0);
  gtk_box_pack_start (GTK_BOX (box), GTK_WIDGET (spin), FALSE, FALSE, 0);
  xfconf_g_property_bind (notification_plugin->channel, "/plugin/log-display-limit", G_TYPE_INT,
                          G_OBJECT (spin), "value");

  /* link the dialog to the plugin, so we can destroy it when the plugin
   * is closed, but the dialog is still open */
  g_object_set_data (G_OBJECT (plugin), "dialog", dialog);

  /* connect the reponse signal to the dialog */
  g_signal_connect (G_OBJECT (dialog), "response",
                    G_CALLBACK(notification_plugin_configure_response), notification_plugin);
  gtk_widget_show_all (box);
}



void
notification_plugin_about (XfcePanelPlugin *plugin)
{
  const gchar *auth[] =
    {
      "Simon Steinbeiss <simon@xfce.org>",
      NULL
    };
  gtk_show_about_dialog (NULL,
                         "logo-icon-name",         ICON_NAME,
                         "license",                xfce_get_license_text (XFCE_LICENSE_TEXT_GPL),
                         "version",                PACKAGE_VERSION,
                         "program-name",           PACKAGE_NAME,
                         "comments",               _("This is the notification plugin"),
                         "website",                PLUGIN_WEBSITE,
                         "copyright",              _("Copyright \xc2\xa9 2017 Simon Steinbeiß\n"),
                         "authors",                auth,
                         NULL);
}
