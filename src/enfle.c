/*
 * enfle.c -- graphic loader Enfle main program
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Oct 29 02:58:42 2000.
 * $Id: enfle.c,v 1.9 2000/10/28 19:07:16 sian Exp $
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
  char *longopt; /* not supported so far */
  char opt;
  ArgumentRequirement argreq;
  unsigned char *description;
} Option;

static Option enfle_options[] = {
  { "help",  'h', _NO_ARGUMENT,       "Show help message." },
  { "ui",    'u', _REQUIRED_ARGUMENT, "Specify which UI to use." },
  { "video", 'v', _REQUIRED_ARGUMENT, "Specify which video to use." },
  { NULL }
};

static void
usage(void)
{
  int i;

  printf(PROGNAME " version " VERSION "\n");
  printf("(C)Copyright 2000 by Hiroshi Takekawa\n\n");
  printf("usage: enfle [options] [path...]\n");
  
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
  Archive *a;
  int i, ch;
  char *plugin_path, *path, *ext, *name, *ui_name = NULL, *video_name = NULL;
  char *optstr;

  optstr = gen_optstring(enfle_options);
  while ((ch = getopt(argc, argv, optstr)) != EOF) {
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
    default:
      fprintf(stderr, "unknown option %c\n", ch);
      usage();
      if (ui_name)
	free(ui_name);
      exit(1);
    }
  }
  free(optstr);

  if (argc == optind) {
    usage();
    if (ui_name)
      free(ui_name);
    return 0;
  }

  c = uidata.c = config_create();
  config_load(c, "enfle.rc");

  eps = uidata.eps = enfle_plugins_create();
  st = uidata.st = streamer_create();
  ui = ui_create();
  ld = uidata.ld = loader_create();
  ar = uidata.ar = archiver_create();
  player = uidata.player = player_create();

  /* scanning... */
  if ((plugin_path = config_get(c, "/enfle/plugins/dir")) == NULL) {
    fprintf(stderr, "plugins/dir not found: configuration error\n");
    return 1;
  }
  a = archive_create();
  archive_read_directory(a, plugin_path, 0);
  path = archive_iteration_start(a);
  while (path) {
    if ((ext = strrchr(path, '.')) && !strncmp(ext, ".so", 3)) {
      PluginType type;

      if ((name = enfle_plugins_load(eps, path, &type)) == NULL) {
	fprintf(stderr, "enfle_plugin_load %s failed.\n", path);
	return 1;
      }
      printf("%s by %s\n",
	     enfle_plugins_get_description(eps, type, name),
	     enfle_plugins_get_author(eps, type, name));
      {
	String *s = string_create();
	char *tmp;

	string_set(s, "/enfle/plugins/");
	string_cat(s, enfle_plugin_type_to_name(type));
	string_cat_ch(s, '/');
	string_cat(s, name);
	string_cat_ch(s, '/');
	string_cat(s, "disabled");

	if (((tmp = config_get(c, string_get(s))) != NULL) &&
	    !strcasecmp(tmp, "yes")) {
	  enfle_plugins_unload(eps, type, name);
	  printf("unload %s (disabled)\n", name);
	}
	string_destroy(s);
      }
    } else if (!strcasecmp(ext, ".spi")) {
      PluginType type;

      if ((name = spi_load(eps, path, &type)) == NULL) {
	fprintf(stderr, "spi_load %s failed.\n", path);
	return 1;
      }
      printf("SPI: %s\n",
	     enfle_plugins_get_description(eps, type, name));
    }
    path = archive_iteration_next(a);
  }
  archive_destroy(a);

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

  archive_destroy(uidata.a);
  archiver_destroy(ar);
  streamer_destroy(st);
  config_destroy(c);
  loader_destroy(ld);
  ui_destroy(ui);
  enfle_plugins_destroy(eps);

  return 0;
}
