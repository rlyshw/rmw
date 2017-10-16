/*
 * main.c
 *
 * This file is part of rmw (https://github.com/andy5995/rmw/wiki)
 *
 *  Copyright (C) 2012-2016  Andy Alt (andy400-dev@yahoo.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 *
 */

#include "main.h"

int
main (int argc, char *argv[])
{
  struct waste_containers waste[WASTENUM_MAX];

  const char *const short_options = "hvc:goz:lsuBwVfir";

  const struct option long_options[] = {
    {"help", 0, NULL, 'h'},
    {"verbose", 0, NULL, 'v'},
    {"config", 1, NULL, 'c'},
    {"list", 0, NULL, 'l'},
    {"purge", 0, NULL, 'g'},
    {"orphaned", 0, NULL, 'o'},
    {"restore", 1, NULL, 'z'},
    {"select", 0, NULL, 's'},
    {"undo-last", 0, NULL, 'u'},
    {"bypass", 0, NULL, 'B'},
    {"warranty", 0, NULL, 'w'},
    {"version", 0, NULL, 'V'},
    {"interactive", 0, NULL, 'i'},
    {"recurse", 0, NULL, 'r'},
    {"force", 0, NULL, 'f'},
    {NULL, 0, NULL, 0}
  };

  short int next_option = 0;

  verbose = 0; /* already declared, a global */

  bool purgeYes = 0;
  bool orphan_chk = 0;
  bool restoreYes = 0;
  bool select = 0;
  bool undo_last = 0;
  bool bypass = 0;
  bool list = 0;
  bool force = 0;

  const char *alt_config = NULL;

  do
  {
    next_option = getopt_long (argc, argv, short_options, long_options, NULL);

    switch (next_option)
    {
    case 'h':
      print_usage ();
      exit (0);
    case 'v':
      verbose = 1;
      break;
    case 'c':
      alt_config = optarg;
      break;
    case 'l':
      list = 1;
      break;
    case 'g':
      purgeYes = 1;
      break;
    case 'o':
      orphan_chk = 1;
      break;
    case 'z':
      restoreYes = 1;
      break;
    case 's':
      select = 1;
      break;
    case 'u':
      undo_last = 1;
      break;
    case 'B':
      bypass = 1;
      break;
    case 'w':
      warranty ();
      break;
    case 'V':
      version ();
      break;
    case 'i':
      fprintf(stdout, "-i / --interactive: not implemented\n");
      break;
    case 'r':
      fprintf(stdout, "-r / --recurse: not implemented\n");
      break;
    case 'f':
      force = 1;
      break;
    case '?':
      print_usage ();
      exit (0);
    case -1:
      break;
    default:
      abort ();
    }

  }
  while (next_option != -1);

  #ifndef WIN32
  char *HOMEDIR = getenv ("HOME");
  #else
  char *HOMEDIR = getenv ("LOCALAPPDATA");
  #endif

  if (HOMEDIR != NULL)
  {
    if (bufchk (HOMEDIR, MP))
      return EXIT_BUF_ERR;
  }
  else
  {
    fprintf (stderr, "Error: Environmental variable $HOME can't be used.\n\
Unable to determine home directory\n");
    return 1;
  }

  char data_dir[MP];

  if (bufchk_string_op (COPY, data_dir,HOMEDIR, MP))
    return EXIT_BUF_ERR;

  /* Looks like more variable names needs changing */
  if (bufchk_string_op (CONCAT, data_dir, DATA_DIR, MP))
    return EXIT_BUF_ERR;

  if (make_dir (data_dir))
  {
    fprintf (stderr, "\
Error: Unable to create config/data directory\n\
Please check your configuration file and permissions\n\
If you need further help, or to report a possible bug,\n\
visit the rmw web site at\n\n\
  https://github.com/andy5995/rmw/wiki\n\n\
Unable to continue. Exiting...\n");
    return 1;
  }

  char protected_dir[PROTECT_MAX][MP];

  /* is reassigned a value in get_config_data() */
  ushort purge_after = 0;

  short conf_err =
    get_config_data (waste, alt_config, &purge_after, protected_dir, &force);

  if (conf_err == NO_WASTE_FOLDER)
    return NO_WASTE_FOLDER;
  else if (conf_err == EXIT_BUF_ERR)
    return EXIT_BUF_ERR;

    /* used for DeletionDate in trashinfo file
     * and for comparison in purge() */
  char time_now[21];
  get_time_string (time_now, 21, "%FT%T");

  if (purgeYes && !purge_after)
    printf ("purging is disabled, 'purge_after' is set to '0'\n");

  if (purge_after != 0)
  {
    if (is_time_to_purge (force) != 0 || purgeYes != 0)
    {
      if (force)
        purge (purge_after, waste, time_now);
      else
        fprintf (stderr, "purge skipped: use -f or --force\n");
    }
  }

/* I don't know of any reason why --list would be needed with other features,
 * so exit rmw (if -l was passed, it was performed in get_config_data())
 */
  if (list)
  {
    short ctr = START_WASTE_COUNTER;
    while (strcmp (waste[++ctr].parent, "NULL") != 0)
      fprintf (stdout, "%s\n", waste[ctr].parent);

    return 0;
  }

  /* String appended to duplicate filenames */
  char time_str_appended[16];
  get_time_string (time_str_appended, 16, "_%H%M%S-%y%m%d");

  if (orphan_chk)
  {
    orphan_maint(waste, time_now, time_str_appended);
    return 0;
  }

  /* FIXME:
   * restore_select() should return a value
   */
  if (select)
  {
    restore_select (waste, time_str_appended);
    return 0;
  }

  /* FIXME:
   * undo_last_rmw() should return a value
   */
  if (undo_last)
  {
    undo_last_rmw (time_str_appended, waste);
    return 0;
  }

  int file_arg = 0;

  if (restoreYes)
  {
    /* subtract 1 from optind otherwise the first file in the list isn't
     * restored
     */
    for (file_arg = optind - 1; file_arg < argc; file_arg++)
      Restore (argv[file_arg], time_str_appended, waste);

    return 0;
  }

  bool undo_opened = 0;
  FILE *undo_file_ptr = NULL;
  char undo_path[MP];

  struct rmw_target file;

  short main_error = 0;

  if (optind < argc)
  {
    strcpy (undo_path, HOMEDIR);
    strcat (undo_path, UNDO_FILE);

    /**
     * No files are open at this point, so just using 'return;'
     */
    if (bufchk (undo_path, MP))
      return EXIT_BUF_ERR;

    int rmwed_files = 0;

    for (file_arg = optind; file_arg < argc; file_arg++)
    {
      strcpy (file.main_argv, argv[file_arg]);

      /**
       * The undo file may be open at this point, so using 'break'
       * After the for loop is the statement to close file, then
       * return EXIT_BUF_ERR
       */
      if ((main_error = bufchk (file.main_argv, MP)))
        break;

      /**
       * Check to see if the file exists, and if so, see if it's protected
       */

      if (exists (file.main_argv) == 0)
      {
        main_error = resolve_path (file.main_argv, file.real_path);

        if (main_error == 1)
          continue;
        else if (main_error > 1)
          break;

        if (!bypass)
        {
          bool flagged = 0;

          short prot_ctr = START_WASTE_COUNTER;

          while (strcmp (protected_dir[++prot_ctr], "NULL") != 0)
          {
            if (strncmp (file.real_path, protected_dir[prot_ctr],
                          strlen (protected_dir[prot_ctr])) == 0)
            {
              flagged = 1;
              break;
            }
          }

          if (flagged)
          {
            fprintf (stderr, "File is in protected directory: %s\n",
                file.real_path);
            continue;
          }
        }
      }
      else
      {
        fprintf (stderr, "File not found: '%s'\n", file.main_argv);
        continue;
      }

      /**
       * Make some variables
       * get ready for the ReMoval
       */

      struct stat st;
      bool match = 0;
      short rename_status = 0;
      bool info_status = 0;
      file.is_duplicate = 0;

      unsigned short current_waste_num = 0;

      strcpy (file.base_name, basename (file.main_argv));

      /**
       * cycle through wasteDirs to see which one matches
       * device number of file.main_argv. Once found, the ReMoval
       * happens (provided all the tests are passed.
       */
      short dir_num = START_WASTE_COUNTER;
      while (strcmp (waste[++dir_num].parent, "NULL") != 0)
      {
        lstat (file.main_argv, &st);

        if (waste[dir_num].dev_num == st.st_dev)
        {
          // used by create_trashinfo
          current_waste_num = dir_num;
          strcpy (file.dest_name, waste[dir_num].files);
          strcat (file.dest_name, file.base_name);

          /* If a duplicate file exists
           */
          if (exists (file.dest_name) == 0)
          {
            // append a time string
            strcat (file.dest_name, time_str_appended);

            // passed to create_trashinfo()
            file.is_duplicate = 1;
          }

          if ((main_error = bufchk (file.dest_name, MP)))
              break;

          rename_status = rename (file.main_argv, file.dest_name);

          if (rename_status == 0)
          {
            if (verbose)
              printf ("'%s' -> '%s'\n", file.main_argv, file.dest_name);

            rmwed_files++;
            info_status = create_trashinfo (file, waste,
                              time_now, time_str_appended, current_waste_num);

            if (info_status == 0)
            {
                /**
                 * Open undo_file for writing if it hasn't been yet
                 */
                if (!undo_opened)
                {
                  if ((undo_file_ptr = fopen (undo_path, "w")) != NULL)
                    undo_opened = 1;
                  else
                  {
                    open_err (undo_path, __func__);
                    return 1;
                  }
                }
              fprintf (undo_file_ptr, "%s\n", file.dest_name);
            }
            else
              fprintf (stderr, "create_trashinfo() returned error %d\n", info_status);
          }
          else
          {
            fprintf (stderr, "Error %d moving %s :\n",
                rename_status, file.main_argv);
            perror ("remove_to_waste()");
            return 1;
          }

      /**
       * If we get to this point, it means a WASTE folder was found
       * that matches the file system file.main_argv was on.
       * Setting match to 1 and breaking from the for loop
       */
          match = 1;
          break;
        }
      }

      if (!match)
      {
        fprintf (stderr, "No suitable filesystem found for \"%s\"\n",
                 file.main_argv);
        return 1;
      }
    }
    if (undo_opened)
      close_file (undo_file_ptr, undo_path, __func__);
    else
      free (undo_file_ptr);

    fprintf(stdout, "%d %s ReMoved to Waste\n", rmwed_files,
            (rmwed_files == 1) ? "file was" : "files were");

    if (main_error > 1)
      return main_error;
  }
  else if (!purgeYes)
      fprintf (stderr, "missing filenames or command line options\n\
Try '%s -h' for more information\n", argv[0]);

  return 0;
}
