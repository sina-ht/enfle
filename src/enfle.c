/*
 * enfle.c -- graphic loader Enfle main program
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Thu Dec 28 08:21:46 2000.
 * $Id: enfle.c,v 1.17 2000/12/27 23:26:31 sian Exp $
 *
 * Enfle is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Enfle is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <stdio.h>
#include <stdlib.h>

#define REQUIRE_STRING_H
#define REQUIRE_UNISTD_H
#include "compat.h"

#ifdef HAVE_GETOPT_H
#  include <getopt.h>
#endif

#include "common.h"

#include "libstring.h"
#include "libconfig.h"
#include "enfle-plugin.h"
#include "ui.h"
#include "streamer.h"
#include "loader.h"
#include "archiver.h"
#include "player.h"
#include "spi.h"

typedef enum _argument_requirement {
  _NO_ARGUMENT,
  _REQUIRED_ARGUMENT,
  _OPTIONAL_ARGUMENT
} ArgumentRequirement;

typedef struct _option {
  const char *longopt; /* not supported so far */
  char opt;
  ArgumentRequirement argreq;
  const unsigned char *description;
} Option;

static Option enfle_options[] = {
  { "help",  'h', _NO_ARGUMENT,       "Show help message." },
  { "ui",    'u', _REQUIRED_ARGUMENT, "Specify which UI to use." },
  { "video", 'v', _REQUIRED_ARGUMENT, "Specify which video to use." },
  { "audio", 'a', _REQUIRED_ARGUMENT, "Specify which audio to use." },
  { "info", 'I', _NO_ARGUMENT, "Print more information." },
  { NULL }
};

static void
usage(void)
{
  int i;

  printf(PROGNAME " version " VERSION "\n");
  printf("(C)Copyright 2000 by Hiroshi Takekawa\n\n");
  printf("usage: enfle [options] [path...]\n");

#if defined(USE_SHM) || defined (USE_PTHREAD)
  printf("Extension: "
#ifdef USE_SHM
 "SHM "
#endif
#ifdef USE_PTHREAD
 "pthread "
#endif
	 "\n");
#endif

  printf("Options:\n");
  i = 0;
  while (enfle_options[i].longopt != NULL) {
    printf(" %c(%s): \t%s\n",
	   enfle_options[i].opt, enfle_options[i].longopt, enfle_options[i].description);
    i++;
  }
}

static char *
gen_optstring(Option opt[])
{
  int i;
  String *s;
  char *optstr;

  s = string_create();
  i = 0;
  while (opt[i].longopt != NULL) {
    string_cat_ch(s, opt[i].opt);
    switch (opt[i].argreq) {
    case _NO_ARGUMENT:
      break;
    case _REQUIRED_ARGUMENT:
      string_cat_ch(s, ':');
      break;
    case _OPTIONAL_ARGUMENT:
      string_cat(s, "::");
      break;
    }
    i++;
  }

  optstr = strdup(string_get(s));

  string_destroy(s);

  return optstr;
}
  
static int
check_and_unload(EnflePlugins *eps, Config *c, PluginType type, char *name)
{
  String *s = string_create();
  char *tmp;
  int result;

  string_set(s, "/enfle/plugins/");
  string_cat(s, enfle_plugin_type_to_name(type));
  string_cat_ch(s, '/');
  string_cat(s, name);
  string_cat_ch(s, '/');
  string_cat(s, "disabled");

  result = 0;
  if (((tmp = config_get(c, string_get(s))) != NULL) &&
      !strcasecmp(tmp, "yes")) {
    //printf("unload %s (disabled)\n", name);
    enfle_plugins_unload(eps, type, name);
    result = 1;
  }
  string_destroy(s);

  return result;
}

static void
print_plugin_info(EnflePlugins *eps, int level)
{
  int i;
  Dlist *dl;
  Dlist_data *dd;
  const unsigned char *plugintypename;
  char *pluginname;
  const unsigned char *description, *author;

  printf("Plugins:\n");
  for (i = 0; i < ENFLE_PLUGIN_END; i++) {
    plugintypename = enfle_plugin_type_to_name((PluginType)i);
    if ((dl = enfle_plugins_get_names(eps, i))) {
      printf(" %s:", plugintypename);
      if (level >= 1)
	printf("\n");
      dlist_iter(dl, dd) {
	pluginname = dlist_data(dd);
	if (level == 0) {
	  printf(" %s", pluginname);
	} else {
	  description = enfle_plugins_get_description(eps, i, pluginname);
	  printf("  %s", description);
	  if (level >= 2) {
	    author = enfle_plugins_get_author(eps, i, pluginname);
	    printf(" by %s", author);
	  }
	  printf("\n");
	}
      }
      if (level == 0)
	printf("\n");
    }
  }
}

static int
scan_and_load_plugins(EnflePlugins *eps, Config *c, char *plugin_path, int spi_enabled)
{
  Archive *a;
  char *path, *ext, *name;
  int nplugins = 0;

  /* scanning... */
  a = archive_create();
  archive_read_directory(a, plugin_path, 0);
  path = archive_iteration_start(a);
  while (path) {
    if ((ext = strrchr(path, '.')) && !strncmp(ext, ".so", 3)) {
      PluginType type;

      if ((name = enfle_plugins_load(eps, path, &type)) == NULL) {
	fprintf(stderr, "enfle_plugin_load %s failed.\n", path);
	return 0;
      }
      nplugins++;
      nplugins -= check_and_unload(eps, c, type, name);
    } else if (spi_enabled && !strcasecmp(ext, ".spi")) {
      PluginType type;

      if ((name = spi_load(eps, path, &type)) == NULL) {
	fprintf(stderr, "spi_load %s failed.\n", path);
	return 1;
      }
      printf("%s(SPI) %s\n", name,
	     enfle_plugins_get_description(eps, type, name));
      nplugins++;
      nplugins -= check_and_unload(eps, c, type, name);
    }
    path = archive_iteration_next(a);
  }
  archive_destroy(a);

  return nplugins;
}

int
main(int argc, char **argv)
{
  extern char *optarg;
  extern int optind;
  UIData uidata;
  EnflePlugins *eps;
  UI *ui;
  Config *c;
  Streamer *st;
  Loader *ld;
  Archiver *ar;
  Player *player;
  String *rcpath;
  int i, ch, spi_enabled = 1;
  int print_more_info = 0;
  char *homedir;
  char *plugin_path;
  char *ui_name = NULL, *video_name = NULL, *audio_name = NULL;
  char *optstr, *tmp;

  optstr = gen_optstring(enfle_options);
  while ((ch = getopt(argc, argv, optstr)) != -1) {
    i = 0;
    switch (ch) {
    case 'h':
      usage();
      if (ui_name)
	free(ui_name);
      return 0;
    case 'u':
      ui_name = strdup(optarg);
      break;
    case 'v':
      video_name = strdup(optarg);
      break;
    case 'a':
      audio_name = strdup(optarg);
      break;
    case 'I':
      print_more_info++;
      break;
    case '?':
    default:
      usage();
      fatal(1, "option error\n");
    }
  }
  free(optstr);

  if ((homedir = getenv("HOME")) == NULL) {
    fprintf(stderr, "Please set HOME environment.\n");
    return 1;
  }

  c = uidata.c = config_create();

  rcpath = string_create();
  string_set(rcpath, homedir);
  string_cat(rcpath, "/.enfle/config");
  if (!(config_load(c, "./enfle.rc") ||
	config_load(c, string_get(rcpath)) ||
	config_load(c, ENFLE_DATADIR "/enfle.rc")))
    fprintf(stderr, "No configuration file. Incomplete install?\n");
  string_destroy(rcpath);

  eps = uidata.eps = enfle_plugins_create();
  st = uidata.st = streamer_create();
  ui = ui_create();
  ld = uidata.ld = loader_create();
  ar = uidata.ar = archiver_create();
  player = uidata.player = player_create();

  if ((tmp = config_get(c, "/enfle/plugins/spi/disabled")) &&
      strcasecmp(tmp, "yes") == 0)
    spi_enabled = 0;

  if ((plugin_path = config_get(c, "/enfle/plugins/dir")) == NULL) {
    plugin_path = ENFLE_PLUGINDIR;
    fprintf(stderr, "plugin_path defaults to %s\n", plugin_path);
  }
  if (!scan_and_load_plugins(eps, c, plugin_path, spi_enabled)) {
    fprintf(stderr, "scan_and_load_plugins() failed.\n");
    return 1;
  }

  if (argc == optind) {
    usage();
    print_plugin_info(eps, print_more_info);
    return 0;
  }

  uidata.a = archive_create();

  if (strcmp(argv[optind], "-") == 0) {
    archive_add(uidata.a, argv[1], strdup(argv[1]));
  } else {
    for (i = optind; i < argc; i++) {
      if (strcmp(argv[i], "-") == 0) {
	fprintf(stderr, "Cannot specify stdin(-) with other files. ignored.\n");
      } else {
	archive_add(uidata.a, argv[i], strdup(argv[i]));
      }
    }
  }

  if (ui_name == NULL && ((ui_name = config_get(c, "/enfle/plugins/ui/default")) == NULL)) {
    fprintf(stderr, "configuration error\n");
    return 1;
  }

  if (audio_name == NULL)
    audio_name = config_get(c, "/enfle/plugins/audio/default");

  uidata.ap = NULL;
  if (audio_name) {
    if ((uidata.ap = enfle_plugins_get(eps, ENFLE_PLUGIN_AUDIO, audio_name)) == NULL)
      fprintf(stderr, "No %s Audio plugin\n", audio_name);
    else
      debug_message("Audio plugin %s\n", audio_name);
  } else {
    show_message("No audio plugin specified.\n");
  }

  if (video_name == NULL && ((video_name = config_get(c, "/enfle/plugins/video/default")) == NULL)) {
    fprintf(stderr, "configuration error\n");
    return 1;
  }

  if ((uidata.vp = enfle_plugins_get(eps, ENFLE_PLUGIN_VIDEO, video_name)) == NULL) {
    fprintf(stderr, "No %s Video plugin\n", video_name);
  } else {
    if (!ui_call(ui, eps, ui_name, &uidata))
      fprintf(stderr, "No UI %s or UI %s initialize failed\n", ui_name, ui_name);
  }

  player_destroy(uidata.player);
  archive_destroy(uidata.a);
  archiver_destroy(ar);
  streamer_destroy(st);
  config_destroy(c);
  loader_destroy(ld);
  ui_destroy(ui);
  enfle_plugins_destroy(eps);

  return 0;
}
