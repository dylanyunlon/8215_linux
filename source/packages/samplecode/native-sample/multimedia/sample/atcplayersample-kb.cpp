/* GStreamer command line playback testing utility - keyboard handling helpers
 *
 * Copyright (C) 2013 Tim-Philipp Müller <tim centricular net>
 * Copyright (C) 2013 Centricular Ltd
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "atcplayersample-kb.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef G_OS_UNIX
#include <unistd.h>
#include <termios.h>
#endif

#include <gst/gst.h>

/* This is all not thread-safe, but doesn't have to be really */

#ifdef G_OS_UNIX

static struct termios term_settings;
static gboolean term_settings_saved = FALSE;
static atcPlayKbFunc kb_callback = NULL;
static gpointer kb_callback_data = NULL;
static gulong io_watch_id = 0;
static GIOChannel *watch_channel = NULL;
static GSource *watch_source = NULL;

static gboolean
atc_play_kb_io_cb (GIOChannel * ioc, GIOCondition cond, gpointer user_data)
{
  GIOStatus status;
  g_print ("Atc_play_kb_io_cb -- cond: 0x%x\r\n", cond);

  if (cond & G_IO_IN) {
    gchar buf[16] = { 0, };
    gsize read;

    status = g_io_channel_read_chars (ioc, buf, sizeof (buf) - 1, &read, NULL);
    g_print ("Atc_play_kb_io_cb -- g_io_channel_read_chars return status = %d\r\n", status);
    if (status == G_IO_STATUS_ERROR) {
      g_print ("Atc_play_kb_io_cb -- g_io_channel_read_chars return status = G_IO_STATUS_ERROR\r\n");
      return FALSE;
    }
    if (status == G_IO_STATUS_NORMAL) {
      g_print ("Atc_play_kb_io_cb -- g_io_channel_read_chars return status = G_IO_STATUS_NORMAL\r\n");
      if (kb_callback) {
        g_print ("Atc_play_kb_io_cb -- kb_callback (buf, kb_callback_data)\r\n");
        kb_callback (buf, kb_callback_data);
      } else {
        g_print ("Atc_play_kb_io_cb -- kb_callback = NULL\r\n");
      }
    }
  } else {
    g_print ("Atc_play_kb_io_cb -- cond(0x%x) & G_IO_IN == 0\r\n", cond);
  }

  return TRUE;                  /* call us again */
}

gboolean
atc_play_kb_set_key_handler (atcPlayKbFunc kb_func, gpointer user_data, gpointer main_context)
{
  int flags;

  if (!isatty (fileno (stdin))) {
    g_print ("Atc_play_kb_set_key_handler fail stdin is not connected to a terminal\r\n");
    return FALSE;
  }

  if (io_watch_id > 0) {
    GSource *source;
    source = g_main_context_find_source_by_id ((GMainContext *)main_context, io_watch_id);
    if (source) {
      g_print ("Atc_play_kb_set_key_handler -- source: %p, watch_source: %px\r\n", source, watch_source);
      g_source_destroy (source);
    }
    else
      g_critical ("Atc_play_kb_set_key_handler --> Source ID %u was not found when attempting to remove it\r\n",
        io_watch_id);
    
    watch_source = NULL;
  }
  
  if (watch_channel != NULL) {
    g_io_channel_unref (watch_channel);
    watch_channel = NULL;
  }

  if (kb_func == NULL && term_settings_saved) {
    /* restore terminal settings */
    if (tcsetattr (fileno (stdin), TCSAFLUSH, &term_settings) == 0)
      term_settings_saved = FALSE;
    else
      g_print ("could not restore terminal attributes");

    setvbuf (stdin, NULL, _IOLBF, 0);
  }

  if (kb_func != NULL) {
    struct termios new_settings;

    if (!term_settings_saved) {
      if (tcgetattr (fileno (stdin), &term_settings) != 0) {
        g_print ("could not save terminal attributes\r\n");
        g_print ("Atc_play_kb_set_key_handler fail for Could not save terminal attributes\r\n");
        return FALSE;
      }
      term_settings_saved = TRUE;

      /* Echo off, canonical mode off, extended input processing off  */
      new_settings = term_settings;
      new_settings.c_lflag &= ~(ECHO | ICANON | IEXTEN);
      
      if (tcsetattr (fileno (stdin), TCSAFLUSH, &new_settings) != 0) {
        g_print ("Could not set terminal state\r\n");
        g_print ("Atc_play_kb_set_key_handler fail for Could not set terminal state\r\n");
        return FALSE;
      }
      
      setvbuf (stdin, NULL, _IONBF, 0);
    }
  }

  watch_channel = g_io_channel_unix_new (fileno (stdin));

  /* make non-blocking */
  flags = g_io_channel_get_flags (watch_channel);
  g_io_channel_set_flags (watch_channel, (GIOFlags)(flags | G_IO_FLAG_NONBLOCK), NULL);

  g_print ("Atc_play_kb_set_key_handler --> g_io_add_watch_full");

  watch_source = g_io_create_watch (watch_channel, G_IO_IN);

  g_source_set_callback (watch_source, (GSourceFunc)atc_play_kb_io_cb, user_data, NULL);

  io_watch_id = g_source_attach (watch_source, (GMainContext *)main_context);
    
  if (io_watch_id <= 0) {
    g_print ("Atc_play_kb_set_key_handler fail in g_io_add_watch_full\r\n");
  }

  kb_callback = kb_func;
  kb_callback_data = user_data;

  g_print ("Atc_play_kb_set_key_handler success\r\n");
  return TRUE;
}

#else /* !G_OS_UNIX */

gboolean
atc_play_kb_set_key_handler (atcPlayKbFunc key_func, gpointer user_data, gpointer main_context)
{
  GST_FIXME ("Keyboard handling for this OS needs to be implemented");
  return FALSE;
}

#endif /* !G_OS_UNIX */


