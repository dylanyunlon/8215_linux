/* GStreamer command line playback testing utility
 *
 * Copyright (C) 2013-2014 Tim-Philipp Müller <tim centricular net>
 * Copyright (C) 2013 Collabora Ltd.
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

#include <locale.h>

#include <gst/gst.h>
#include <gst/gstversion.h>
#include <gst/audio/audio.h>
#include <gst/pbutils/pbutils.h>
#include <gst/math-compat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "gst/playback/gstplay-enum.h"
#include "atcmediaplayer.h"

#include "atcplayersample-kb.h"
#include "gettext.h"
#include "glib-compat-private.h"
#include "gst-i18n-plugin.h"
#include "atcmediautils.h"
#include "atcsurface.h"
#include <gstatcconvert.h>
#include <atcsubtitlerender.h>
#include "PrivLog.h"
#include "osalpub.h"

#include "client/linux/handler/exception_handler.h"

#define LOCALEDIR "/usr/share/locale"

#define ATC_PLAYERSAMPLE_NAME        "atcplayersample"	
/* /> main version */
#define ATC_PLAYERSAMPLE_VER_MAIN        00
/* /> minor version, Large feature tens add 1 and clear unit, */
/* /> other feature and add new function need add 1. */
#define ATC_PLAYERSAMPLE_VER_MIN         00
/* /> version number, you should add 1 when check in aud */
#define ATC_PLAYERSAMPLE_VER_REV         01

typedef struct _custom_subtitle_render {
    IAtcSubtitleRender   subtitle_render;
    void  *data;
} custom_subtitle_render_t;

class AtcPlay : public AtcMediaListener
{
public:
  
  AtcPlay();
  
  virtual ~AtcPlay();
  
  bool notify(const int32_t msg, int32_t ext1 = 0,
      int32_t ext2 = 0, void *obj = NULL);

public:
  gchar **uris;
  gint32 num_uris;
  gint32 cur_idx;

  bool buffering;
  bool is_live;

  /* configuration */
  bool gapless;

  bool eos;

  bool repeatall;

  GSource * playfail_timeout;
    
  GSource * timeout_source;

	AtcMediaPlayer::media_play_state state;
  AtcMediaPlayer::media_status media_status;

  AtcMediaPlayer *player;

  IAtcSurface *video_surface;
	GCond	  * play_cond;
	GMutex	* play_mutex;

  double rate;

  VOID *main_context;
  custom_subtitle_render_t  subtitle_render;
  IAtcSubtitleRenderVtbl    subtitle_render_vtbl;
} ;

static guint32 atc_play_get_file_cnt(AtcPlay * play);
static bool atc_play_next (AtcPlay * play);
static bool atc_play_prev (AtcPlay * play);
static void atc_play_reset (AtcPlay * play);
static void atc_play_resume (AtcPlay * play);
static void atc_play_pause (AtcPlay * play);
static bool atc_play_set_video_surface(AtcPlay *play, int32_t width, int32_t height);
static bool atc_play_set_sub_surface(AtcPlay *play, int32_t width, int32_t height);
static void atc_play_set_state(AtcPlay *play, AtcMediaPlayer::media_play_state s);
static void atc_play_set_media_status(AtcPlay *play, int32_t status);
static bool atc_play_start_play(AtcPlay *play); 
static void atc_restore_terminal (AtcPlay *play);
static bool atc_play_get_metadata(AtcPlay *play);
static gboolean atc_play_start_timer(AtcPlay * play, guint32 interval,
  GSourceFunc    function,
  gpointer       data);
static gboolean atc_play_stop_timer(AtcPlay * play);
static gboolean atc_play_fail(gpointer data);
static gboolean atc_play_start_check_play(AtcPlay * play, guint32 interval,
  GSourceFunc    function,
  gpointer       data);
static gboolean atc_play_stop_check_play(AtcPlay * play);


static const char *atcmediastatestr[] = {
  "Paused", 
  "Playing",
  "Stopped",
  "Error"
};

#define GET_ATCSAMPLE_STATE_STR(s) \
  (((s) <= AtcMediaPlayer::ErrorState) ? atcmediastatestr[s] : "UNKNOWN")

AtcPlay::AtcPlay()
{
  uris = NULL;
  num_uris = 0;
  cur_idx = -1;

  buffering = FALSE;
  is_live = FALSE;

  /* configuration */
  gapless = FALSE;

  eos = FALSE;
  
  timeout_source = NULL;

  playfail_timeout = NULL;

	state = AtcMediaPlayer::StoppedState;
  media_status = AtcMediaPlayer::NoMedia;

  player = NULL;
  video_surface = NULL;
	play_cond = NULL;
	play_mutex = NULL;
}
  
AtcPlay::~AtcPlay()
{
  LOGE ("AtcPlay::~~AtcPlay\r\n");
}

bool AtcPlay::notify(const int32_t msg, int32_t ext1,
      int32_t ext2, void *obj)
{
  switch(msg) {
		case AtcMediaPlayer::PlayStateChanged:
      {
        switch ((AtcMediaPlayer::media_play_state)ext1) {
          case AtcMediaPlayer::StoppedState:
          case AtcMediaPlayer::PausedState:
          case AtcMediaPlayer::PlayingState:
            atc_play_set_state(this, (AtcMediaPlayer::media_play_state)ext1);
            break;
          case AtcMediaPlayer::ErrorState:
            LOGE ("in Error State now , the app should change to play the next file\r\n");
            atc_play_set_state(this, (AtcMediaPlayer::media_play_state)ext1);
            break;
          default:
            break;
          }
      }
      break;
    case AtcMediaPlayer::MediaStatusUpdate:
      atc_play_set_media_status(this, ext1);
			break;
    case AtcMediaPlayer::MediaSetRateComplete:
			LOGE ("AtcMediaPlayer::MediaSetRateComplete. err1=%d, err2=%d\r\n", ext1, ext2);
			break;
    case AtcMediaPlayer::MediaSeekComplete:
			LOGE ("AtcMediaPlayer::MediaSeekComplete. err1=%d, err2=%d\r\n", ext1, ext2);
			break;
    case AtcMediaPlayer::MediaErrorNotify:
			LOGE ("AtcMediaPlayer::MediaErrorNotify. err1=%d, err2=%d\r\n", ext1, ext2);
      if (atc_play_get_file_cnt(this) > 1) {
        if (!atc_play_next (this)) {
          LOGE ("\nAtcPlayerSample fail in atc_play_next.\n");
          g_mutex_lock(play_mutex);
          g_cond_signal(play_cond);
          g_mutex_unlock(play_mutex);
        }
      } else {
        g_mutex_lock(play_mutex);
        g_cond_signal(play_cond);
        g_mutex_unlock(play_mutex);
      }
			break;
    case AtcMediaPlayer::MediaInfoNotify:
      switch (ext1) {
      case AtcMediaPlayer::MetaDataUpdate:
        atc_play_get_metadata(this);
        break;
      }
			break;
    case AtcMediaPlayer::SetVideoSurfaceNotify:
      if (!atc_play_set_video_surface(this, ext1, ext2)) {
        LOGE ("AtcMediaPlayer::SetVideoSurfaceNotify fail in set surface. err1=%d, err2=%d\r\n", ext1, ext2);
      }
      break;
    case AtcMediaPlayer::SetSubtitleSurfaceNotify:
      if (!atc_play_set_sub_surface(this, ext1, ext2)) {
        LOGE ("AtcMediaPlayer::SetSubtitleSurfaceNotify fail in set surface. err1=%d, err2=%d\r\n", ext1, ext2);
      }
      break;
		default:
			break;
  }
  
  return TRUE;
}

static bool atc_play_start_play(AtcPlay *play) 
{
  LOGE ("[AtcPlayerSample] %s --> start to play\r\n", __FUNCTION__);
  play->player->play();
  return TRUE;
}

static void atc_play_set_state(AtcPlay *play, AtcMediaPlayer::media_play_state s)
{

  if ((s == AtcMediaPlayer::PausedState) && 
      (play->state == AtcMediaPlayer::StoppedState) && 
      (play->media_status == AtcMediaPlayer::LoadedMedia)) {
    LOGE ("%s --> media_play_state change from Stopped to Paused, start to play\r\n",
      __FUNCTION__);
    play->state = s;
    atc_play_start_play(play);
    return;
  }

  if (s == AtcMediaPlayer::PlayingState) {    
    atc_play_stop_check_play(play);
  }

  LOGE ("atc_play_set_state--> old play state: %s, new play state: %d\r\n", 
    GET_ATCSAMPLE_STATE_STR(play->state),
    GET_ATCSAMPLE_STATE_STR(s));
  
  play->state = s;
}

static void atc_play_set_media_status(AtcPlay *play, int32_t status) 
{
  LOGE ("atc_play_set_media_status --> old media_status: %d, new media_status: %d\r\n", 
    play->media_status, status);

  if (((AtcMediaPlayer::media_status)status == AtcMediaPlayer::LoadedMedia) && 
      (play->media_status == AtcMediaPlayer::LoadingMedia) && 
      (play->state == AtcMediaPlayer::PausedState)) {
      play->media_status = (AtcMediaPlayer::media_status)status;
      atc_play_start_play(play);
      return;
  }

  if (((AtcMediaPlayer::media_status)status == AtcMediaPlayer::EndOfMedia) && 
      ((play->state == AtcMediaPlayer::PlayingState) ||
       (play->state == AtcMediaPlayer::PausedState))) {
      play->media_status = (AtcMediaPlayer::media_status)status;
      if (!atc_play_next (play)) {
        LOGE ("\nReached end of play list.\n");
        g_mutex_lock(play->play_mutex);
        g_cond_signal(play->play_cond);
        g_mutex_unlock(play->play_mutex);
      }
      return;
  }
  
  play->media_status = (AtcMediaPlayer::media_status)status;
}

static bool atc_play_get_metadata(AtcPlay *play)
{
  AtcMediaMetadata :: metadata_value_type eValueType;
  void *pvValue = NULL;
  int32_t vallen = 0;
  int32_t i4Key = 0;

  for (i4Key = AtcMediaMetadata::METADATA_KEY_TRACK_NUMBER;
      i4Key < AtcMediaMetadata::METADATA_KEY_MAX;
      i4Key++) {
    if (play->player->getMetadata((AtcMediaMetadata::metadata_key_type)i4Key, 
      &eValueType, &pvValue, &vallen)) {
      /* LOGE("get_metadata (key = %d) eValueType=%d, vallen: %d\r\n",
          i4Key, eValueType, vallen);
          */
      switch (eValueType) {
      case AtcMediaMetadata::MetaData_String:
        break;
      case AtcMediaMetadata::MetaData_ByteArray:
        LOGE("get_metadata (key = %d) eValueType=%d, vallen: %d\r\n",
          i4Key, eValueType, vallen);        
        break;
      case AtcMediaMetadata::MetaData_Int64:
        break;
      case AtcMediaMetadata::MetaData_Int32:
        break;
      case AtcMediaMetadata::MetaData_Float:
        break;
      case AtcMediaMetadata::MetaData_Double:
        break;
      }
    }
  }

  return TRUE;
}

static bool atc_play_set_video_surface(AtcPlay *play, int32_t width, int32_t height)
{
  LOGE ("[AtcPlayerSample] %s -- atc_createsurface()\r\n", __FUNCTION__);
  play->video_surface = atc_createsurface(ATCSURF_TYPE_DEFAULT, width, height, ATC_PIX_FMT_NV12M_PRIVATE1);
  if (NULL == play->video_surface) {
    LOGE ("[AtcPlayerSample] %s fail in CreateVideoSurface\r\n", __FUNCTION__);
    return FALSE;
  }

  //IAtcSurface_setCapability(play->video_surface, ATCSURF_CAPS_DESTINATION_COLORKEY);
  //IAtcSurface_setColorkey(play->video_surface, 0xFF080808);
  IAtcSurface_setLayerZOrder(play->video_surface, 3);
  IAtcSurface_setWindow(play->video_surface, 0, 0, 1024, 600);
  //IAtcSurface_setWindow(play->video_surface, 100, 50, 800, 400);
  //IAtcSurface_setLayerZOrder(play->video_surface, 3);
  
  if (!play->player->setVideoSurface((void *)play->video_surface)) {
    LOGE ("[AtcPlayerSample] %s fail in player->setVideoSurface\r\n", __FUNCTION__);
    return FALSE;
  }
  
  return TRUE;
}

static int32_t drawSubText(IAtcSubtitleRender *render, atc_subtext_t *subtext) {
	LOGE("[AtcPlayerSample] drawSubText 1111111111111111111\n");
	return 0;
}

static int32_t drawSubPicture(IAtcSubtitleRender *render, atc_subpic_t *subpic) {
	LOGE("[AtcPlayerSample] drawSubPicture 1111111111111111111\n");
	return 0;
}

static int32_t clearSubtitle(IAtcSubtitleRender *render) {
	LOGE("[AtcPlayerSample] clearSubtitle 1111111111111111111\n");
	return 0;
}

static bool atc_play_set_sub_surface(AtcPlay *play, int32_t width, int32_t height)
{
  LOGE ("[AtcPlayerSample] %s -- atc_createsurface()\r\n", __FUNCTION__);
  play->player->setSubtitleSurface(&(play->subtitle_render.subtitle_render));
  
  return FALSE;
}


static gboolean
atc_play_timeout (gpointer user_data)
{
  AtcPlay *play = (AtcPlay *)user_data;
  gint64 pos = -1, dur = -1;
  gchar status[64] = { 0, };
  
  if ((NULL == play) ||
    (NULL == play->player))
  {
     return TRUE;
  }

  if (play->player->state() != AtcMediaPlayer::PlayingState)
    g_snprintf (status, sizeof (status), "Paused");
  else
    memset (status, ' ', sizeof (status) - 1);
  
  pos = play->player->getPosition();
  dur = play->player->getDuration();

  LOGE ("pos = %lldms, dur = %lldms\r\n", pos, dur);
  if (pos >= 0 && dur > 0) {
    gchar dstr[32], pstr[32];

    /* FIXME: pretty print in nicer format */
    g_snprintf (pstr, 32, "%lld", pos);
    pstr[9] = '\0';
    g_snprintf (dstr, 32, "%lld", dur);
    dstr[9] = '\0';
    LOGE ("%s ms/ %s ms, %s\r", pstr, dstr, status);
  }

  return TRUE;
}


static bool 
atc_play_new (AtcPlay *play, gchar ** uris)
{
  gint32 i = 0;
  if ((NULL == play) ||
		(NULL == play->player) ||
		(NULL == uris)) {
		LOGE("%s fail for invalid args\r\n", __FUNCTION__);
		return FALSE;
  }
	
  play->uris = uris;
  play->num_uris = g_strv_length (uris);
	LOGE("%s -- play->num_uris = %d\r\n", __FUNCTION__, play->num_uris);

  for (i = 0; i < play->num_uris; i++) {
    LOGE("%s -- play->uris[%d]: %s\r\n", __FUNCTION__, i, 
      (play->uris[i]) ? (play->uris[i]) : "NULL");
  }
  play->cur_idx = -1;
		
  return TRUE;
}

static void
atc_play_free (AtcPlay * play)
{
	LOGE("%s -- enter\r\n", __FUNCTION__);;
  if (play) {
    atc_play_reset (play);
    atc_restore_terminal(play);

    if (play->video_surface) {
      IAtcSurface_release(play->video_surface);
      play->video_surface = NULL;
    }

  	if (NULL != play->player) {
  		delete play->player;
  		play->player = NULL;
  	}

    if (NULL != play->play_cond) {
      g_cond_free (play->play_cond);
      play->play_cond = NULL;
    }
    
    if (NULL != play->play_mutex) {
      g_mutex_free (play->play_mutex);
      play->play_mutex = NULL;
    }


		delete (play);
  }
  DeInitOSAL();
	LOGE("%s -- exit\r\n", __FUNCTION__);;
}

/* reset for new file/stream */
static void
atc_play_reset (AtcPlay * play)
{
  if (NULL == play)
    return;
  

  play->buffering = FALSE;
  play->is_live = FALSE;
  play->eos = FALSE;
  
	LOGE ("%s line %d play->state = %d before reset\r\n",
    __FUNCTION__, __LINE__, ( play->state = play->player->state()));
  
	play->player->reset();
  play->rate = 1.0;
    
  atc_play_stop_timer (play);
  atc_play_stop_check_play(play);

	LOGE ("%s line %d play->state = %d after reset\r\n",
    __FUNCTION__, __LINE__, ( play->state = play->player->state()));

  if (play->video_surface) {
    IAtcSurface_release(play->video_surface);
    play->video_surface = NULL;
    play->player->setVideoSurface(NULL);
  }

  play->player->setSubtitleSurface(NULL);

}

static gchar *
atc_play_uri_get_display_name (AtcPlay * play, const gchar * uri)
{
  gchar *loc;

  if (gst_uri_has_protocol (uri, "file")) {
    loc = gst_atc_filename_from_uri (uri, NULL, NULL);
  } else if (gst_uri_has_protocol (uri, "pushfile")) {
    loc = gst_atc_filename_from_uri (uri + 4, NULL, NULL);
  } else {
    loc = g_strdup (uri);
  }

  /* Maybe additionally use glib's filename to display name function */
  return loc;
}

static gboolean atc_play_start_timer(AtcPlay * play, guint32 interval,
  GSourceFunc    function,
  gpointer       data)
{
  GSource *source = NULL;

  atc_play_stop_timer(play);
  
  g_return_val_if_fail (function != NULL, 0);

  source = g_timeout_source_new (interval);

  g_source_set_callback (source, function, data, NULL);
  g_source_attach (source, (GMainContext *)(play->main_context));

  play->timeout_source = source;

  g_print ("atc_play_start_timer, success -- play->timeout_source: %p\r\n",
    play->timeout_source);

  return TRUE;
}

static gboolean atc_play_stop_timer(AtcPlay * play)
{
  if (play->main_context == NULL) {
    g_print ("atc_play_stop_timer success, no main_context\r\n");
    return TRUE;
  }
  
  if (play->timeout_source == NULL) {
    g_print ("atc_play_stop_timer success, no timeout source\r\n");
    return TRUE;
  }
  
  g_source_destroy (play->timeout_source);
  play->timeout_source = NULL;
  
  return TRUE;
}

static gboolean atc_play_start_check_play(AtcPlay * play, guint32 interval,
  GSourceFunc    function,
  gpointer       data)
{
  GSource *source = NULL;

  atc_play_stop_timer(play);
  
  g_return_val_if_fail (function != NULL, 0);

  source = g_timeout_source_new (interval);

  g_source_set_callback (source, function, data, NULL);
  g_source_attach (source, (GMainContext *)(play->main_context));

  play->playfail_timeout = source;

  g_print ("atc_play_start_check_play, success -- play->playfail_timeout: %p\r\n",
    play->playfail_timeout);

  return TRUE;
}

static gboolean atc_play_stop_check_play(AtcPlay * play)
{
  if (play->main_context == NULL) {
    g_print ("atc_play_stop_check_play success, no main_context\r\n");
    return TRUE;
  }
  
  if (play->playfail_timeout == NULL) {
    g_print ("atc_play_stop_check_play success, no timeout source\r\n");
    return TRUE;
  }
  
  g_source_destroy (play->playfail_timeout);
  play->playfail_timeout = NULL;
  
  return TRUE;
}


static void
atc_play_uri (AtcPlay * play, const gchar * next_uri)
{
  gchar *loc;
	
  if ((NULL == play) ||
		(NULL == play->player) ||
		(NULL == next_uri)) {
		LOGE("%s fail for invalid args\r\n", __FUNCTION__);
		return ;
  }

  play->eos = FALSE;
  
  atc_play_reset (play);

  loc = atc_play_uri_get_display_name (play, next_uri);
  LOGE ("Now playing %s\r\n", loc);
  g_free (loc);

  atc_play_start_check_play(play, 10000, atc_play_fail, play);

  play->rate = 1.0;

	LOGE ("%s line %d next_uri= %s\r\n", __FUNCTION__, __LINE__, (next_uri ? next_uri : "NULL"));
	play->player->openUri(next_uri);
	LOGE ("%s line %d play->state = %d\r\n", __FUNCTION__, __LINE__, ( play->state = play->player->state()));
  
  if (!atc_play_start_timer (play, 1000, atc_play_timeout, (gpointer)play)) {
    LOGE ("%s line %d fail in g_timeout_add\r\n", __FUNCTION__, __LINE__);
  }

  return;
}

static guint32 
atc_play_get_file_cnt(AtcPlay * play)
{
  LOGE ("AtcMediaPlayer::MediaErrorNotify. num_uris=%d\r\n", play->num_uris);
  return play->num_uris;
}

static gboolean atc_play_fail(gpointer data) {
  AtcPlay * play = (AtcPlay *)data;
  
  if (!atc_play_next (play)) {
    LOGE ("\nReached end of play list, so exit the player sample.\r\n");
    play->eos = TRUE;
    g_mutex_lock(play->play_mutex);
    g_cond_signal(play->play_cond);
    g_mutex_unlock(play->play_mutex);
  }

  return TRUE;
}

/* returns FALSE if we have reached the end of the playlist */
static bool
atc_play_next (AtcPlay * play)
{
  if (play->repeatall) {
    if ((play->cur_idx + 1) >= play->num_uris)
      play->cur_idx = -1;
  } else {
    if ((play->cur_idx + 1) >= play->num_uris)
      return FALSE;
  }

  atc_play_uri (play, play->uris[++play->cur_idx]);
  return TRUE;
}

/* returns FALSE if we have reached the beginning of the playlist */
static bool
atc_play_prev (AtcPlay * play)
{
  if (play->cur_idx == 0 || play->num_uris <= 1)
    return FALSE;

  atc_play_uri (play, play->uris[--play->cur_idx]);
  return TRUE;
}

static bool 
atc_play_ff_rw(AtcPlay *play, gboolean forward)
{
  bool seekable = FALSE;
  AtcMediaPlayer::media_play_rate playrate;
  double rate = play->rate;
  double set_rate = play->rate;

  seekable = play->player->canSeek();  
  if (!seekable) {
    LOGE ("\nCan't do ff_rw for seekable is FALSE.\r\n");
    return FALSE;
  }

  if (forward) {
    if (rate < 0) rate = 1.0;
    else rate *= 2;
  } else {
    if (rate > 0) rate = -2.0;
    else rate *= 2;
  }

  if ((rate > 32.5) || (rate < -32.5)) {
    rate = 1.0;
  }

  if ((-0.5 < rate) && (rate < 1.5)) {
    playrate = AtcMediaPlayer::PlayRateNormal;
    set_rate = 1.0;
  } else if ((1.5 < rate) && (rate < 2.5)) {
    playrate = AtcMediaPlayer::PlayRateFF2X;
    set_rate = 2.0;
  } else if ((3.5 < rate) && (rate < 4.5)) {
    playrate = AtcMediaPlayer::PlayRateFF4X;
    set_rate = 4.0;
  } else if ((7.5 < rate) && (rate < 8.5)) {
    playrate = AtcMediaPlayer::PlayRateFF8X;
    set_rate = 8.0;
  } else if ((15.5 < rate) && (rate < 16.5)) {
    playrate = AtcMediaPlayer::PlayRateFF16X;
    set_rate = 16.0;
  } else if ((31.5 < rate) && (rate < 32.5)) {
    playrate = AtcMediaPlayer::PlayRateFF32X;
    set_rate = 32.0;
  } else if ((-2.5 < rate) && (rate < -1.5)) {
    playrate = AtcMediaPlayer::PlayRateRW2X;
    set_rate = -2.0;
  } else if ((-4.5 < rate) && (rate < -3.5)) {
    playrate = AtcMediaPlayer::PlayRateRW4X;
    set_rate = -4.0;
  } else if ((-8.5 < rate) && (rate < -7.5)) {
    playrate = AtcMediaPlayer::PlayRateRW8X;
    set_rate = -8.0;
  } else if ((-16.5 < rate) && (rate < -15.5)) {
    playrate = AtcMediaPlayer::PlayRateRW16X;
    set_rate = -16.0;
  } else if ((-32.5 < rate) && (rate < -31.5)) {
    playrate = AtcMediaPlayer::PlayRateRW32X;
    set_rate = -32.0;
  } else {
    g_print ("\nCould not ff_rw for invalid rate: %f.\n", rate);
    goto seek_failed;
  }

  play->rate = set_rate;
  
   LOGE ("%s -- player->ff_fr--\r\n", __func__);
   play->player->setPlaybackRate(playrate);
  return TRUE;

seek_failed:
  {
    g_print ("\nCould not seek.\n");
  }
return FALSE;
}

static void
atc_do_play (AtcPlay * play)
{
  gint32 i;

  /* dump playlist */
  for (i = 0; i < play->num_uris; ++i)
    g_print ("%4u : %s", i, play->uris[i]);

  if (!atc_play_next (play))
    return;
}

static void
atc_play_resume (AtcPlay * play)
{
	g_print ("%s line %d --> resume\n", __FUNCTION__, __LINE__);

	play->player->play();

	g_print ("%s line %d play->state = %d\n", __FUNCTION__, __LINE__,( play->state = play->player->state()));
}

static void
atc_play_pause (AtcPlay * play)
{
	g_print ("%s line %d --> pause\n", __FUNCTION__, __LINE__);
	
	play->player->pause();
	
	g_print ("%s line %d play->state = %d\n", __FUNCTION__, __LINE__,( play->state = play->player->state()));
}

static void
atc_add_to_playlist (GPtrArray * playlist, const gchar * filename)
{
  GDir *dir;
  gchar *uri;

  if (gst_uri_is_valid (filename)) {
    g_ptr_array_add (playlist, g_strdup (filename));
    return;
  }

  if ((dir = g_dir_open (filename, 0, NULL))) {
    const gchar *entry;

    /* FIXME: sort entries for each directory? */
    while ((entry = g_dir_read_name (dir))) {
      gchar *path;

      path = g_strconcat (filename, G_DIR_SEPARATOR_S, entry, NULL);
      atc_add_to_playlist (playlist, path);
      g_free (path);
    }

    g_dir_close (dir);
    return;
  }

  uri = gst_filename_to_uri (filename, NULL);
  if (uri != NULL)
    g_ptr_array_add (playlist, uri);
  else
    g_warning ("Could not make URI out of filename '%s'", filename);
}

static void
atc_restore_terminal (AtcPlay * play)
{
  atc_play_kb_set_key_handler (NULL, NULL, play->main_context);
}

static void
atc_toggle_paused (AtcPlay * play)
{
  if (play->state == AtcMediaPlayer::StoppedState){
		atc_do_play(play);
  }
	else if (play->state == AtcMediaPlayer::PlayingState) {
		atc_play_pause(play);
	}
	else if (play->state == AtcMediaPlayer::PausedState) {
		atc_play_resume(play);
  }
}

static void
atc_relative_seek (AtcPlay * play, gdouble percent)
{
  bool seekable = FALSE;
  gint64 dur = -1, pos = -1;

  g_return_if_fail (percent >= -1.0 && percent <= 1.0);

	pos = play->player->getPosition();
	dur = play->player->getDuration();

  seekable = play->player->canSeek();

  if (!seekable) {
    LOGE ("\nCan't do seek for seekable is FALSE.\r\n");
    return;
  }
  
  if (dur <= 0) {
    LOGE ("\nCan't do seek for dur <= 0.\r\n");
    return;
  }

  LOGE ("%s -- pos = %lldms, dur = %lldms\r\n", __func__, pos, dur);
  pos = pos + dur * percent;
  if (pos > dur) {
    LOGE ("%s -- switch to the next file\r\n", __func__);
    if (!atc_play_next (play)) {
      LOGE ("\nReached end of play list, so exit the player sample.\r\n");
      play->eos = TRUE;
      g_mutex_lock(play->play_mutex);
      g_cond_signal(play->play_cond);
      g_mutex_unlock(play->play_mutex);
    }
  } else {
    if (pos < 0)
      pos = 0;
    LOGE ("%s -- player->seek(%lldms)\r\n", __func__, pos);
    play->player->seek(pos);
  }

  return;
}

static void
atc_keyboard_cb (const gchar * key_input, gpointer user_data)
{
  AtcPlay *play = (AtcPlay *) user_data;
  LOGE ("keyboard input: %c", g_ascii_tolower (key_input[0]));

  switch (g_ascii_tolower (key_input[0])) {
    case ' ':
      atc_toggle_paused (play);
      break;
    case 'q':
      g_print ("keyboard input: Q, do exit");
      g_mutex_lock(play->play_mutex);
      g_cond_signal(play->play_cond);
      g_mutex_unlock(play->play_mutex);
      break;
    case '>':
      if (!atc_play_next (play)) {
        g_print ("\nReached end of play list.\n");
        g_mutex_lock(play->play_mutex);
        g_cond_signal(play->play_cond);
        g_mutex_unlock(play->play_mutex);
      }
      break;
    case '<':
      atc_play_prev (play);
      break;
    case 'f':
      atc_play_ff_rw (play, TRUE);
      break;
    case 'w':
      atc_play_ff_rw (play, FALSE);
      break;
    case 27:                   /* ESC */
      if (key_input[1] == '\0') {
        g_mutex_lock(play->play_mutex);
        g_cond_signal(play->play_cond);
        g_mutex_unlock(play->play_mutex);
        break;
      }
    case 's':
      {
          gint audiocount = 0;
          gint curaudioidx = 0;

          if (play->eos) {
            return;
          }

					audiocount = play->player->getStreamCount(AtcMediaPlayer::AudioStream);
					curaudioidx = play->player->getActiveStream(AtcMediaPlayer::AudioStream);

          g_print ("keyboard input: S");
          g_print ("audio count = %d", audiocount);
          g_print ("current audio idx = %d", curaudioidx);

					curaudioidx++;
					curaudioidx %= audiocount;

					play->player->setActiveStream(AtcMediaPlayer::AudioStream, curaudioidx);


          break;
      }

      /* fall through */
    case 'k':
      g_print ("keyboard input: k, SEEK Forward");
      atc_relative_seek (play, +0.2);
      break;

    case 'j':
      g_print ("keyboard input: j, SEEK Backward");
      atc_relative_seek (play, -0.2);
      break;

    case 'p':
      g_print ("keyboard input: p, do Pause");
      atc_play_pause(play);
      break;
      
    case 'r':
      g_print ("keyboard input: r, do Resume");
      atc_play_resume(play);
      break;

      /* fall through */
    default:
      g_print ("keyboard input:");
      for (; *key_input != '\0'; ++key_input) {
        g_print ("  %c", *key_input);
      }
      break;
  }
}

int
main (int argc, char **argv)
{
  GPtrArray *playlist;
  bool repeatall = FALSE; /* FIXME: maybe enable by default? */
  gchar **filenames = NULL;
  gchar **uris;
  gint num, i;
  GError *err = NULL;
  gchar *playlist_file = NULL;
  AtcPlay *play = NULL;
	char szPlayerName[50] = {0};
  GOptionEntry options[] = {
    {"playlist", 0, 0, G_OPTION_ARG_FILENAME, &playlist_file,
        N_("Playlist file containing input media files"), NULL},
    {"repeatall", 0, 0, G_OPTION_ARG_NONE, &repeatall,
        N_("Interactive control via keyboard"), NULL},
    {G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY, &filenames, NULL},
    {NULL}
  };
  
  google_breakpad::MinidumpDescriptor descriptor("/tmp");
  google_breakpad::ExceptionHandler eh(descriptor, NULL, NULL, NULL, true, -1);
    
  InitOSAL();

  setlocale (LC_ALL, "");

#ifdef ENABLE_NLS
  bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
#endif

  sprintf(szPlayerName, "atcplayersample-%d.%d", GST_VERSION_MAJOR, GST_VERSION_MINOR);

  LOGI("[VER][%s] %02d.%02d.%03d\r\n", 
    ATC_PLAYERSAMPLE_NAME, ATC_PLAYERSAMPLE_VER_MAIN, ATC_PLAYERSAMPLE_VER_MIN,
    ATC_PLAYERSAMPLE_VER_REV);

  LOGE("%s -- g_set_prgname(%s)", __FUNCTION__, szPlayerName);

  g_set_prgname (szPlayerName);
  
  LOGE ("atcplayer enter\r\n");
 
  LOGE ("gst-play new playlist\r\n");
  
  LOGE("%s -- g_new0 (AtcPlay, 1)\r\n", __FUNCTION__);

#if 1
  play = new AtcPlay();
  if (NULL == play) {
  	LOGE("%s fail in g_new0 AtcPlay\r\n", __FUNCTION__);
  	return 0;
  }
#else
  AtcPlay play1;
  play = &play1;
  LOGE("%s -- play->notify(0)\r\n", __FUNCTION__);

  play->notify(0);
#endif

  LOGE("%s -- play(0x%08x)->player = new AtcMediaPlayer()\r\n", __FUNCTION__, play);

  play->player = new AtcMediaPlayer(options, &argc, &argv);
  if (NULL == play->player) {
  	LOGE("%s fail in g_new0 AtcMediaPlayer\r\n", __FUNCTION__);
		delete (play);
  	return 0;
  }
  
	LOGE("%s success in g_new0 AtcMediaPlayer \r\n",
    __FUNCTION__);
  
  play->subtitle_render_vtbl.drawSubText = drawSubText;
  play->subtitle_render_vtbl.drawSubPicture = drawSubPicture;
  play->subtitle_render_vtbl.clearSubtitle = clearSubtitle;
  play->subtitle_render.subtitle_render.vtbl = &(play->subtitle_render_vtbl);

	play->player->setListener((AtcMediaListener *)play);
	  
	LOGE("%s new playlist\r\n", __FUNCTION__);;

  playlist = g_ptr_array_new ();

  if (playlist_file != NULL) {
    gchar *playlist_contents = NULL;
    gchar **lines = NULL;

    if (g_file_get_contents (playlist_file, &playlist_contents, NULL, &err)) {
      lines = g_strsplit (playlist_contents, "\n", 0);
      num = g_strv_length (lines);

      for (i = 0; i < num; i++) {
        if (lines[i][0] != '\0') {
          g_print ("Playlist[%d]: %s", i + 1, lines[i]);
          atc_add_to_playlist (playlist, lines[i]);
        }
      }
      g_strfreev (lines);
      g_free (playlist_contents);
    } else {
      g_printerr ("Could not read playlist: %s\n", err->message);
      g_clear_error (&err);
    }
    g_free (playlist_file);
    playlist_file = NULL;
  }

  if (playlist->len == 0 && (filenames == NULL || *filenames == NULL)) {
    g_printerr (("Usage: atcplayersample-%d.%d FILE1|URI1 [FILE2|URI2] [FILE3|URI3] ..."),
        GST_VERSION_MAJOR, GST_VERSION_MINOR);
    g_printerr ("\n\n"),
        g_printerr ("%s\n\n",
        _("You must provide at least one filename or URI to play."));
    /* No input provided. Free array */
    g_ptr_array_free (playlist, TRUE);
		atc_play_free(play);

    return 1;
  }

  /* fill playlist */
  if (filenames != NULL && *filenames != NULL) {
    num = g_strv_length (filenames);
    for (i = 0; i < num; ++i) {
      g_print ("command line argument: %s", filenames[i]);
      atc_add_to_playlist (playlist, filenames[i]);
    }
    g_strfreev (filenames);
  }

  num = playlist->len;
  g_ptr_array_add (playlist, NULL);

  uris = (gchar **) g_ptr_array_free (playlist, FALSE);
	
	LOGE("%s -- call atc_play_new\r\n", __FUNCTION__);

  play->repeatall = repeatall;
  
  if (!atc_play_new (play, uris)) {
		LOGE ("fail in atc_play_new.\r\n");
		/* clean up */
		atc_play_free (play);
				
		LOGE ("\n");
		return 0;
  }

	LOGE("%s -- call Atc_play_kb_set_key_handler\r\n", __FUNCTION__);;
  play->main_context = play->player->getContext();
  if (atc_play_kb_set_key_handler(atc_keyboard_cb, play, play->main_context)) {
    LOGE ("Interactive keyboard handling in terminal available.\n");
  } else {
    LOGE ("Interactive keyboard handling in terminal not available.\n");
  }
	LOGE("%s -- call atc_do_play\r\n", __FUNCTION__);
  
  play->play_mutex = g_mutex_new();
  if (NULL == play->play_mutex) {
    LOGE("%s -- Could not create play_mutex\r\n", __FUNCTION__);
    atc_play_free(play);
    return 0;
  }
  
  play->play_cond = g_cond_new();
  if (NULL == play->play_cond) {
    LOGE("%s -- Could not create play_cond\r\n", __FUNCTION__);
    atc_play_free(play);
    return 0;
  }

  /* play */
  atc_do_play (play);

  g_mutex_lock(play->play_mutex);
	LOGE("%s -- wait play_cond, i.e. exit signal\r\n", __FUNCTION__);;
  g_cond_wait(play->play_cond, play->play_mutex);
	LOGE("%s -- wait play_cond OK\r\n", __FUNCTION__);;
  g_mutex_unlock(play->play_mutex);

	LOGE("%s -- call atc_play_free\r\n", __FUNCTION__);;

  /* clean up */
  atc_play_free (play);

  LOGE ("\n");
  DeInitOSAL();
  return 0;
}


