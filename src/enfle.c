/*
 * enfle.c -- graphic loader Enfle main program
 * (C)Copyright 2000-2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Jun  6 21:40:07 2009.
 * $Id: enfle.c,v 1.74 2006/02/24 18:54:55 sian Exp $
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
#include <sys/stat.h>
#include <libgen.h>

#define REQUIRE_STRING_H
#define REQUIRE_UNISTD_H
#include "compat.h"

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

#ifdef USE_DMO
#  include "enfle/dmo.h"
#endif

#include "getopt-support.h"

/* Static plugin prototypes */
STATIC_PLUGIN_PROTO

#define DEFAULT_SLIDE_INTERVAL 5

static Option enfle_options[] = {
  { "ui",        'u', _REQUIRED_ARGUMENT, "Specify which UI to use." },
  { "video",     'v', _REQUIRED_ARGUMENT, "Specify which video to use." },
  { "audio",     'a', _REQUIRED_ARGUMENT, "Specify which audio to use." },
  { "wallpaper", 'w', _NO_ARGUMENT,       "Set the first image as wallpaper(alias for -u Wallpaper)." },
  { "convert",   'C', _OPTIONAL_ARGUMENT, "Convert images automatically (default PNG, for Convert UI)." },
  { "magnify",   'm', _REQUIRED_ARGUMENT, "Specify magnification method(for Normal UI)." },
  { "config",    'c', _REQUIRED_ARGUMENT, "Additional config like -c '/enfle/plugins/saver/jpeg/quality = 80'." },
  { "nth",       'n', _REQUIRED_ARGUMENT, "Start at nth file/archive." },
  { "nocache",   'q', _NO_ARGUMENT,       "Don't use plugin cache." },
  { "recache",   'N', _NO_ARGUMENT,       "Re-create cache file." },
  { "include",   'i', _REQUIRED_ARGUMENT, "Specify the pattern to include." },
  { "exclude",   'x', _REQUIRED_ARGUMENT, "Specify the pattern to exclude." },
  { "minwidth",  'X', _REQUIRED_ARGUMENT, "Specify the minimum width of an image to display." },
  { "minheight", 'Y', _REQUIRED_ARGUMENT, "Specify the minimum height of an image to display." },
  { "readdir",   'd', _NO_ARGUMENT,       "Read the directory wherein the specified file resides." },
  { "slideshow", 's', _OPTIONAL_ARGUMENT, "Slideshow, optionally accepts interval timer in second." },
  { "info",      'I', _NO_ARGUMENT,       "Print more information." },
  { "help",      'h', _NO_ARGUMENT,       "Show help message." },
  { "version",   'V', _NO_ARGUMENT,       "Show version." },
  { NULL , '\0', _NO_ARGUMENT, NULL }
};

static void
usage(void)
{
  const char *ext = 
#if defined(USE_MMX) || defined(USE_SSE) || defined(USE_SHM) || defined(USE_XV) || defined(USE_PTHREAD) || defined(USE_SPI) || defined(__INTEL_COMPILER)
#ifdef USE_MMX
    "MMX "
#endif
#ifdef USE_SSE
    "SSE "
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
#ifdef USE_DMO
    "dmo "
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
#if defined(__i386__)
  {
    CPUCaps caps = cpucaps_get();

    printf("CPU Caps.: ");
#if defined(USE_MMX)
    if (caps & _MMX)
      printf("MMX ");
#else
    if (caps & _MMX)
      printf("MMX* ");
#endif
#if defined(USE_SSE)
    if (caps & _SSE)
      printf("SSE ");
    if (caps & _SSE2)
      printf("SSE2 ");
    if (caps & _SSE3)
      printf("SSE3 ");
#else
    if (caps & _SSE)
      printf("SSE* ");
    if (caps & _SSE2)
      printf("SSE2* ");
    if (caps & _SSE3)
      printf("SSE3* ");
#endif
    printf("\n");
#if !defined(USE_MMX) || !defined(USE_SSE)
    printf("*, like MMX*, means CPU has that capability but enfle was not compiled for it.");
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
print_plugin_info(EnflePlugins *eps, const char *path, int level)
{
  int i;
  PluginList *pl;
  EnflePlugin *ep;
  Plugin *p;
  void *k;
  unsigned int kl;
  const char *plugintypename;
  char *pluginname;
  const char *description, *author;

  printf("Plugins: path = %s\n", path);
  for (i = 0; i < ENFLE_PLUGIN_END; i++) {
    pl = eps->pls[i];
    plugintypename = enfle_plugin_type_to_name((PluginType)i);
    printf(" %s:", plugintypename);
    if (level >= 1)
      printf("\n");
    pluginlist_iter(pl, k, kl, p) {
      pluginname = (char *)k;
      if (level == 0) {
	printf(" %s", pluginname);
      } else {
	ep = plugin_get(p);
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

#if defined(USE_SPI)
static int
scan_and_load_spi_plugins(EnflePlugins *eps, Config *c, char *plugin_path)
{
  Archive *a;
  PluginType type;
  char *fullpath, *path, *ext = NULL, *base_name, *name;
  int nplugins = 0;
  int tmp, result;

  tmp = config_get_boolean(c, "/enfle/plugins/spi/disabled", &result);
  if (result > 0 && tmp)
    return 0;
  else if (result < 0) {
    warning("Invalid string in spi/disable.  spi disabled.\n");
    return 0;
  }

  a = archive_create(ARCHIVE_ROOT);
  archive_read_directory(a, plugin_path, 0);
  path = archive_iteration_start(a);
  while (path) {
    base_name = misc_basename(path);
    fullpath = archive_getpathname(a, path);
    if ((ext = strrchr(path, '.'))) {
      if (!strcasecmp(ext, ".spi")) {
	if ((name = spi_load(eps, fullpath, &type)) == NULL) {
	  warning("spi_load %s failed.\n", fullpath);
	} else {
	  nplugins++;
	  nplugins -= check_and_unload(eps, c, type, name);
	}
      }
    }
    free(fullpath);
    path = archive_iteration_next(a);
  }
  archive_destroy(a);

  return nplugins;
}
#endif

#if defined(USE_DMO)
static int
scan_and_load_dmo_plugins(EnflePlugins *eps, Config *c, char *plugin_path)
{
  Archive *a;
  PluginType type;
  char *fullpath, *path, *ext = NULL, *base_name, *name;
  int nplugins = 0;
  int tmp, result;

  tmp = config_get_boolean(c, "/enfle/plugins/dmo/disabled", &result);
  if (result > 0 && tmp)
    return 0;
  else if (result < 0) {
    warning("Invalid string in dmo/disable.  dmo disabled.\n");
    return 0;
  }

  a = archive_create(ARCHIVE_ROOT);
  archive_read_directory(a, plugin_path, 0);
  path = archive_iteration_start(a);
  while (path) {
    base_name = misc_basename(path);
    fullpath = archive_getpathname(a, path);
    if ((ext = strrchr(path, '.'))) {
      if (!strcasecmp(ext, ".dll")) {
	if ((name = dmo_load(eps, fullpath, &type)) == NULL) {
	  warning("dmo_load %s failed.\n", fullpath);
	} else {
	  nplugins++;
	  nplugins -= check_and_unload(eps, c, type, name);
	}
      }
    }
    free(fullpath);
    path = archive_iteration_next(a);
  }
  archive_destroy(a);

  return nplugins;
}
#endif

static int
scan_and_load_plugins(EnflePlugins *eps, Config *c, char *plugin_path)
{
  Archive *a;
  PluginType type;
  char *fullpath, *path, *ext, *base_name, *name;
  FILE *fp = NULL;
  int nplugins = 0;

  /* Add static linked plugins */
  STATIC_PLUGIN_ADD;

  if (eps->cache_path) {
    if (eps->cache_to_be_created) {
      if ((fp = fopen(eps->cache_path, "wb")) == NULL) {
	err_message_fnc("Cannot create cache file %s\n", eps->cache_path);
	eps->cache_to_be_created = 0;
	free(eps->cache_path);
	eps->cache_path = NULL;
      }
      show_message("Cache file %s to be created.\n", eps->cache_path);
    } else {
      debug_message_fnc("Using cache file %s\n", eps->cache_path);
      return 1;
    }
  }

  a = archive_create(ARCHIVE_ROOT);
  if (!archive_read_directory(a, plugin_path, 0))
    return 0;
  path = archive_iteration_start(a);
  while (path) {
    base_name = misc_basename(path);
    fullpath = archive_getpathname(a, path);
    if ((ext = strrchr(path, '.'))) {
      if (!strcasecmp(ext, ".so") &&
	  !(strncasecmp(base_name, "ui_", 3) &&
	    strncasecmp(base_name, "video_", 6) &&
	    strncasecmp(base_name, "videodecoder_", 13) &&
	    strncasecmp(base_name, "audio_", 6) &&
	    strncasecmp(base_name, "audiodecoder_", 13) &&
	    strncasecmp(base_name, "demultiplexer_", 14) &&
	    strncasecmp(base_name, "loader_", 7) &&
	    strncasecmp(base_name, "saver_", 6) &&
	    strncasecmp(base_name, "player_", 7) &&
	    strncasecmp(base_name, "streamer_", 9) &&
	    strncasecmp(base_name, "archiver_", 9) &&
	    strncasecmp(base_name, "effect_", 7))) {

	if ((name = enfle_plugins_load(eps, fullpath, &type)) == NULL) {
	  warning("enfle_plugin_load %s failed.\n", fullpath);
	} else {
	  if (!check_and_unload(eps, c, type, name)) {
	    nplugins++;
	    if (fp)
	      fprintf(fp, "%s:%s:%s\n", enfle_plugin_type_to_name(type), name, fullpath);
	  }
	}
      }
    }
    free(fullpath);
    path = archive_iteration_next(a);
  }
  archive_destroy(a);

  if (fp)
    fclose(fp);

  return nplugins;
}

int
main(int argc, char **argv)
{
  extern char *optarg;
  extern int optind;
  struct option *long_opt;
  int option_index = 0;
  UIData uidata;
  EnflePlugins *eps;
  Config *c;
  String *rcpath;
  int i, ch, result;
  int print_more_info = 0;
  int magnify_method = 0;
  int include_fnmatch = 0;
  int exclude_fnmatch = 0;
  int if_use_cache = -1;
  int if_readdir = 0;
  int if_slideshow = 0;
  int slide_interval = DEFAULT_SLIDE_INTERVAL;
  int nth = 0;
  int minw = 0, minh = 0;
  char *pattern = NULL;
  char *homedir;
  char *plugin_path;
  char *format = NULL;
  char *ui_name = NULL, *video_name = NULL, *audio_name = NULL;
  char *optstr;
  Dlist *override_config;
  Dlist_data *dd;

  override_config = dlist_create();
  optstr = gen_optstring(enfle_options, &long_opt);
#if defined(HAVE_GETOPT_LONG)
  while ((ch = getopt_long(argc, argv, optstr, long_opt, &option_index)) != -1) {
#else
  while ((ch = getopt(argc, argv, optstr)) != -1) {
#endif
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
      if (include_fnmatch || exclude_fnmatch) {
	err_message("Sorry, only one -i/-x option is permitted.\n");
	return 1;
      }
      include_fnmatch = 1;
      pattern = strdup(optarg);
      break;
    case 'x':
      if (include_fnmatch || exclude_fnmatch) {
	err_message("Sorry, only one -i/-x option is permitted.\n");
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
      debug_message_fnc("format = %s\n", format);
      break;
    case 'm':
      magnify_method = atoi(optarg);
      break;
    case 'c':
      dlist_add_str(override_config, optarg);
      break;
    case 'n':
      nth = atoi(optarg);
      break;
    case 'q':
      if_use_cache = 0;
      break;
    case 'N':
      if_use_cache = 2;
      break;
    case 'w':
      ui_name = strdup("Wallpaper");
      break;
    case 'X':
      minw = atoi(optarg);
      break;
    case 'Y':
      minh = atoi(optarg);
      break;
    case 'd':
      if_readdir = 1;
      break;
    case 's':
      if_slideshow = 1;
      if (optarg) {
	slide_interval = atoi(optarg);
	if (slide_interval == 0)
	  if_slideshow = 2;
      }
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
  if (long_opt)
    free(long_opt);

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

  {
    int path_ok = 0;
    struct stat st;

    if ((plugin_path = getenv("ENFLE_PLUGINDIR")) != NULL) {
      if (stat(plugin_path, &st) == 0)
	path_ok = 1;
    }
    if (!path_ok && (plugin_path = config_get_str(c, "/enfle/plugins/dir"))) {
      if (stat(plugin_path, &st) == 0)
	path_ok = 1;
    }
    if (!path_ok) {
      plugin_path = (char *)ENFLE_PLUGINDIR;
      debug_message("plugin_path defaults to %s\n", plugin_path);
    }
  }

  if_use_cache = if_use_cache == -1 ? config_get_boolean(c, "/enfle/use_cache", &result) : if_use_cache;
  eps = enfle_plugins_create(plugin_path, if_use_cache);
  set_enfle_plugins(eps);

  if (!scan_and_load_plugins(eps, c, plugin_path)) {
    err_message("scan_and_load_plugins() failed.  plugin path = %s\n", plugin_path);
    return 1;
  }

  if (if_use_cache == 2) {
    show_message("Plugin cache updated.\n");
    enfle_plugins_destroy(eps);
    return 0;
  }

#if defined(USE_SPI)
  {
    char *spi_plugin_path;
    if ((spi_plugin_path = config_get_str(c, "/enfle/plugins/spi/dir")) != NULL) {
      if (!scan_and_load_spi_plugins(eps, c, spi_plugin_path)) {
	show_message("spi requested, but no spi plugins loaded from %s.\n", spi_plugin_path);
      }
    }
  }
#endif
#if defined(USE_DMO)
  {
    char *dmo_plugin_path;
    if ((dmo_plugin_path = config_get_str(c, "/enfle/plugins/dmo/dir")) != NULL) {
      if (!scan_and_load_dmo_plugins(eps, c, dmo_plugin_path)) {
	show_message("dmo requested, but no dmo plugins loaded from %s.\n", dmo_plugin_path);
      }
    }
  }
#endif

  if (argc == optind) {
    usage();
    print_plugin_info(eps, plugin_path, print_more_info);
    config_destroy(c);
    enfle_plugins_destroy(eps);
    return 0;
  }

  uidata.nth = nth;
  uidata.minw = minw;
  uidata.minh = minh;
  uidata.if_readdir = if_readdir;
  uidata.if_slideshow = if_slideshow;
  uidata.slide_interval = slide_interval;
  uidata.a = archive_create(ARCHIVE_ROOT);

  if (include_fnmatch)
    archive_set_fnmatch(uidata.a, pattern, _ARCHIVE_FNMATCH_INCLUDE);
  else if (exclude_fnmatch)
    archive_set_fnmatch(uidata.a, pattern, _ARCHIVE_FNMATCH_EXCLUDE);

  if (strcmp(argv[optind], "-") == 0) {
    archive_add(uidata.a, argv[optind], strdup(argv[optind]));
  } else {
    for (i = optind; i < argc; i++) {
      if (strcmp(argv[i], "-") == 0) {
	warning("Cannot specify stdin(-) with other files. ignored.\n");
      } else {
	if (uidata.if_readdir == 1) {
	  struct stat st;

	  if (!stat(argv[i], &st) == 0) {
	    warning("stat() failed with '-d' for %s.\n", argv[i]);
	  } else {
	    if (!S_ISREG(st.st_mode)) {
	      warning("Non regular file: %s.\n", argv[i]);
	    } else {
	      char *path = strdup(argv[i]);
	      char *dir = dirname(path);
	      char *path2 = strdup(argv[i]);

	      uidata.path = basename(path2);
	      show_message_fnc("readdir: %s -> %s\n", uidata.path, dir);
	      uidata.if_readdir++;
	      archive_add(uidata.a, dir, dir);
	      continue;
	    }
	  }
	}
	  archive_add(uidata.a, argv[i], strdup(argv[i]));
      }
    }
  }

  if (ui_name == NULL && ((ui_name = config_get_str(c, "/enfle/plugins/ui/default")) == NULL)) {
    err_message("Cannot get UI plugin name\n");
    return 1;
  }

  if (audio_name == NULL)
    audio_name = config_get_str(c, "/enfle/plugins/audio/default");

  uidata.ap = NULL;
  if (audio_name) {
    debug_message("Audio plugin %s\n", audio_name);
    if ((uidata.ap = enfle_plugins_get(eps, ENFLE_PLUGIN_AUDIO, audio_name)) == NULL)
      err_message("No %s Audio plugin\n", audio_name);
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
