/*
 * enfle.c -- graphic loader Enfle main program
 * (C)Copyright 2000, 2001, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Dec  7 01:51:07 2003.
 * $Id: enfle.c,v 1.52 2003/12/08 01:43:51 sian Exp $
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

#include "utils/libstring.h"
#include "utils/libconfig.h"
#include "utils/misc.h"
#include "utils/cpucaps.h"
#include "enfle/enfle-plugin.h"
#include "enfle/ui.h"
#include "enfle/streamer.h"
#include "enfle/loader.h"
#include "enfle/saver.h"
#include "enfle/effect.h"
#include "enfle/archiver.h"
#include "enfle/player.h"
#ifdef USE_SPI
#  include "enfle/spi.h"
#endif

#include "getopt-support.h"

/* Static plugin prototypes */
STATIC_PLUGIN_PROTO

static Option enfle_options[] = {
  { "ui",        'u', _REQUIRED_ARGUMENT, "Specify which UI to use." },
  { "video",     'v', _REQUIRED_ARGUMENT, "Specify which video to use." },
  { "audio",     'a', _REQUIRED_ARGUMENT, "Specify which audio to use." },
  { "wallpaper", 'w', _NO_ARGUMENT,       "Set the first image as wallpaper(alias for -u Wallpaper)." },
  { "convert",   'C', _OPTIONAL_ARGUMENT, "Convert images automatically (default PNG, for Convert UI)." },
  { "magnify",   'm', _REQUIRED_ARGUMENT, "Specify magnification method(for Normal UI)." },
  { "config",    'c', _REQUIRED_ARGUMENT, "Additional config like -c '/enfle/plugins/saver/jpeg/quality = 80'." },
  { "include",   'i', _REQUIRED_ARGUMENT, "Specify the pattern to include." },
  { "exclude",   'x', _REQUIRED_ARGUMENT, "Specify the pattern to exclude." },
  { "info",      'I', _NO_ARGUMENT,       "Print more information." },
  { "help",      'h', _NO_ARGUMENT,       "Show help message." },
  { "version",   'V', _NO_ARGUMENT,       "Show version." },
  { NULL , '\0', _NO_ARGUMENT, NULL }
};

static void
usage(void)
{
  const char *ext = 
#if defined(USE_MMX) || defined(USE_SHM) || defined(USE_XV) || defined(USE_PTHREAD) || defined(USE_SPI) || defined(__INTEL_COMPILER)
#ifdef USE_MMX
    "MMX "
#endif
#ifdef USE_SHM
    "SHM "
#endif
#ifdef USE_XV
    "Xv "
#endif
#ifdef USE_PTHREAD
    "pthread "
#endif
#ifdef USE_SPI
    "spi "
#endif
#ifdef __INTEL_COMPILER
    "icc "
#endif
#else
    "None"
#endif
    "\n";

  printf(PROGNAME " version " VERSION "\n" COPYRIGHT_MESSAGE "\n\n");
  printf("usage: enfle [options] [path...]\n");
  printf("Extension: %s", ext);
#ifdef __i386__
  {
    CPUCaps caps = cpucaps_get();
#ifdef USE_MMX
    if (caps & _MMX)
      printf("MMX is available.\n");
    else
      printf("MMX is not available.\n");
#else
    if (caps & _MMX)
      printf("MMX is available, but disabled at compile-time.\n");
#endif
  }
#endif
  printf("Options:\n");
  print_option_usage(enfle_options);
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
  if (((tmp = config_get_str(c, string_get(s))) != NULL) &&
      !strcasecmp(tmp, "yes")) {
    debug_message("unload %s (disabled)\n", name);
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
  PluginList *pl;
  EnflePlugin *ep;
  Plugin *p;
  void *k;
  unsigned int kl;
  const char *plugintypename;
  char *pluginname;
  const unsigned char *description, *author;

  printf("Plugins:\n");
  for (i = 0; i < ENFLE_PLUGIN_END; i++) {
    pl = eps->pls[i];
    plugintypename = enfle_plugin_type_to_name((PluginType)i);
    printf(" %s:", plugintypename);
    if (level >= 1)
      printf("\n");
    pluginlist_iter(pl, k, kl, p) {
      ep = plugin_get(p);
      pluginname = (char *)k;
      if (level == 0) {
	printf(" %s", pluginname);
      } else {
	description = enfle_plugin_description(ep);
	printf("  %s", description);
	if (level >= 2) {
	  author = enfle_plugin_author(ep);
	  printf(" by %s", author);
	}
	printf("\n");
      }
    }
    pluginlist_iter_end
    if (level == 0)
      printf("\n");
  }
}

static int
scan_and_load_plugins(EnflePlugins *eps, Config *c, char *plugin_path)
{
  Archive *a;
  PluginType type;
  char *fullpath, *path, *ext, *base_name, *name;
  int nplugins = 0;
#ifdef USE_SPI
  int spi_enabled = 1;
  int tmp, result;

  tmp = config_get_boolean(c, "/enfle/plugins/loader/spi/disabled", &result);
  if (result > 0 && tmp)
    spi_enabled = 0;
  else if (result < 0)
    warning("Invalid string in spi/disable.\n");
#endif

  /* Add static linked plugins */
  STATIC_PLUGIN_ADD

  a = archive_create(ARCHIVE_ROOT);
  archive_read_directory(a, plugin_path, 0);
  path = archive_iteration_start(a);
  while (path) {
    base_name = misc_basename(path);
    fullpath = archive_getpathname(a, path);
    if ((ext = strrchr(path, '.'))) {
      if (!strcasecmp(ext, ".so") &&
	  !(strncasecmp(base_name, "ui_", 3) &&
	    strncasecmp(base_name, "video_", 6) &&
	    strncasecmp(base_name, "audio_", 6) &&
	    strncasecmp(base_name, "loader_", 7) &&
	    strncasecmp(base_name, "saver_", 6) &&
	    strncasecmp(base_name, "player_", 7) &&
	    strncasecmp(base_name, "streamer_", 9) &&
	    strncasecmp(base_name, "archiver_", 9) &&
	    strncasecmp(base_name, "effect_", 7))) {

	if ((name = enfle_plugins_load(eps, fullpath, &type)) == NULL) {
	  warning("enfle_plugin_load %s failed.\n", fullpath);
	} else {
	  nplugins++;
	  nplugins -= check_and_unload(eps, c, type, name);
	}
#ifdef USE_SPI
      } else if (spi_enabled && !strcasecmp(ext, ".spi")) {
	if ((name = spi_load(eps, fullpath, &type)) == NULL) {
	  warning("spi_load %s failed.\n", fullpath);
	} else {
	  nplugins++;
	  nplugins -= check_and_unload(eps, c, type, name);
	}
#endif
#if 0
      } else if (ext && !strcasecmp(ext, ".dll")
		 //  || !strcasecmp(ext, ".acm")
		 ) {
	PluginType type;

	if ((name = spi_load(eps, fullpath, &type)) == NULL) {
	  warning(stderr, "spi_load %s failed.\n", fullpath);
	} else {
	  nplugins++;
	  nplugins -= check_and_unload(eps, c, type, name);
	}
#endif
      }
    }
    free(fullpath);
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
  Config *c;
  String *rcpath;
  int i, ch;
  int print_more_info = 0;
  int magnify_method = 0;
  int include_fnmatch = 0;
  int exclude_fnmatch = 0;
  char *pattern = NULL;
  char *homedir;
  char *plugin_path;
  char *format = NULL;
  char *ui_name = NULL, *video_name = NULL, *audio_name = NULL;
  char *optstr;
  Dlist *override_config;
  Dlist_data *dd;

  override_config = dlist_create();
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
    case 'i':
      if (include_fnmatch) {
	err_message("Sorry, only one -i option is permitted.\n");
	return 1;
      }
      include_fnmatch = 1;
      pattern = strdup(optarg);
      break;
    case 'x':
      if (exclude_fnmatch) {
	err_message("Sorry, only one -x option is permitted.\n");
	return 1;
      }
      exclude_fnmatch = 1;
      pattern = strdup(optarg);
      break;
    case 'I':
      print_more_info++;
      break;
    case 'C':
      ui_name = strdup("Convert");
      if (optarg)
	format = strdup(optarg);
      else
	format = strdup("PNG");
      break;
    case 'm':
      magnify_method = atoi(optarg);
      break;
    case 'c':
      dlist_add_str(override_config, optarg);
      break;
    case 'w':
      ui_name = strdup("Wallpaper");
      break;
    case 'V':
      printf(PROGNAME " version " VERSION "\n" COPYRIGHT_MESSAGE "\n");
      return 0;
    case '?':
    default:
      usage();
      return 1;
    }
  }
  free(optstr);

  if ((homedir = getenv("HOME")) == NULL) {
    err_message("Please set HOME environment.\n");
    return 1;
  }

  c = uidata.c = config_create();

  rcpath = string_create();
  string_set(rcpath, homedir);
  string_cat(rcpath, "/.enfle/config");
  if (!(config_load(c, "./enfle.rc") ||
	config_load(c, string_get(rcpath)) ||
	config_load(c, ENFLE_DATADIR "/enfle.rc")))
    warning("No configuration file. Incomplete install?\n");
  string_destroy(rcpath);

  switch (magnify_method) {
  case 0:
    config_parse(c, (char *)"/enfle/plugins/ui/normal/render = normal");
    break;
  case 1:
    config_parse(c, (char *)"/enfle/plugins/ui/normal/render = double");
    break;
  case 2:
    config_parse(c, (char *)"/enfle/plugins/ui/normal/render = short");
    break;
  case 3:
    config_parse(c, (char *)"/enfle/plugins/ui/normal/render = long");
    break;
  default:
    break;
  }
  if (print_more_info > 0)
    config_parse(c, (char *)"/enfle/plugins/ui/normal/show_comment = yes");

  dlist_iter (override_config, dd) {
    void *d = dlist_data(dd);
    config_parse(c, d);
  }
  dlist_destroy(override_config);

  if (format)
    config_set_str(c, (char *)"/enfle/plugins/ui/convert/format", format);

  eps = enfle_plugins_create();
  set_enfle_plugins(eps);

  if ((plugin_path = config_get_str(c, "/enfle/plugins/dir")) == NULL) {
    plugin_path = (char *)ENFLE_PLUGINDIR;
    debug_message("plugin_path defaults to %s\n", plugin_path);
  }
  if (!scan_and_load_plugins(eps, c, plugin_path)) {
    err_message("scan_and_load_plugins() failed.  plugin path = %s\n", plugin_path);
    return 1;
  }

  if (argc == optind) {
    usage();
    print_plugin_info(eps, print_more_info);
    config_destroy(c);
    enfle_plugins_destroy(eps);
    return 0;
  }

  uidata.a = archive_create(ARCHIVE_ROOT);

  if (strcmp(argv[optind], "-") == 0) {
    archive_add(uidata.a, argv[1], strdup(argv[1]));
  } else {
    for (i = optind; i < argc; i++) {
      if (strcmp(argv[i], "-") == 0) {
	warning("Cannot specify stdin(-) with other files. ignored.\n");
      } else {
	archive_add(uidata.a, argv[i], strdup(argv[i]));
      }
    }
  }

  if (include_fnmatch)
    archive_set_fnmatch(uidata.a, pattern, _ARCHIVE_FNMATCH_INCLUDE);
  else if (exclude_fnmatch)
    archive_set_fnmatch(uidata.a, pattern, _ARCHIVE_FNMATCH_EXCLUDE);

  if (ui_name == NULL && ((ui_name = config_get_str(c, "/enfle/plugins/ui/default")) == NULL)) {
    err_message("Cannot get UI plugin name\n");
    return 1;
  }

  if (audio_name == NULL)
    audio_name = config_get_str(c, "/enfle/plugins/audio/default");

  uidata.ap = NULL;
  if (audio_name) {
    if ((uidata.ap = enfle_plugins_get(eps, ENFLE_PLUGIN_AUDIO, audio_name)) == NULL)
      err_message("No %s Audio plugin\n", audio_name);
    else
      debug_message("Audio plugin %s\n", audio_name);
  } else {
    show_message("No audio plugin specified.\n");
  }

  if (video_name == NULL && ((video_name = config_get_str(c, "/enfle/plugins/video/default")) == NULL)) {
    err_message("Cannot get Video plugin name\n");
    return 1;
  }

  if ((uidata.vp = enfle_plugins_get(eps, ENFLE_PLUGIN_VIDEO, video_name)) == NULL) {
    err_message("No %s Video plugin\n", video_name);
  } else {
    if (!ui_call(eps, ui_name, &uidata))
      err_message("No UI %s or UI %s initialize failed\n", ui_name, ui_name);
  }

  archive_destroy(uidata.a);
  config_destroy(c);
  enfle_plugins_destroy(eps);

  return 0;
}
