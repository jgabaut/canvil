// jgabaut @ github.com/jgabaut
// SPDX-License-Identifier: GPL-3.0-only
/*
    Copyright (C) 2024-2026 jgabaut

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include "canvil.h"
#include "canvil_optparse.h"
#define DUMBTIMER_IMPLEMENTATION
#include "../dumbtimer/dumbtimer.h"
#include <dirent.h>

void canvil_usage(char* progname)
{
    printf("Usage: %s [-qxhvlLwXRFe] [-gB] [-bripd] [-I <builds_dir>] [-O <stego_dir>] [-D <target_dir>] [-K <kazoj_dir>] [-S <source_name>] [-E <bin_name>] [-M <minmake_tag>] [-G <dir_name>] [-C <configure_arg>] [-Z <cflags>] [-V <verbose_lvl>] [-a <anvil_version>] [-k <anvil_kern>]\n", progname);
}

void canvil_help(char* progname)
{
    printf("canvil v%s\n", CANVIL_API_VERSION_STRING);
    printf("Usage: %s [OPTIONS] [TAG] [COMMAND]\n", progname);
    printf("Commands:\n  test [-b|-l]                   Run all tests or the passed TESTNAME\n  build                          Tries building latest tag\n  init [-k <KERN>] [TEMPLATE]    Prepare a new anvil project\n  version                        Prints canvil version\n  help                           Print this message or the help of the given subcommand(s)\nArguments:\n  [TAG]  Optional tag argument\n\n");
    printf("Example usage:  %s [(-I|-O|-D|-M|-S|-K|-E|-G|-C|-Z|-x|-V|-a|-k) <ARG>] [-TBtg] [-bripd] [-hvsqlLXWPJRFe] [TAG]\n", progname);
    printf("Options:\n  -D, --amboso-dir <BIN_DIR>         Specify the directory to host tags [default: ./bin]\n  -I, --builds-dir <BUILDS_DIR>      Specify the directory to host build [default: .]\n  -O, --stego-dir <STEGO_DIR>        Specify the directory to host stego.lock file [default: wd, BIN_DIR]\n  -K, --kazoj-dir <TESTS_DIR>        Specify the directory to host tests\n  -S, --source <SOURCE_NAME>         Specify the source name\n  -E, --execname <EXEC_NAME>         Specify the target executable name\n  -M, --maketag <MAKE_MINTAG>        Specify min tag using make as build/clean step\n  -a, --anvil-version <AMBOSO_VERS>  Specify amboso version to use\n  -k, --anvil-kern <AMBOSO_KERN>     Specify amboso kern to use\n  -G, --gen-c-header <C_HEADER_DIR>  Generate anvil C header for passed dir\n  -x, --linter <LINT_TARGET>         Act as stego linter for passed file\n  -T, --test                         Specify test mode\n  -B, --base                         Specify base mode\n  -g, --git                          Specify git mode\n  -t, --testmacro                    Specify test macro mode\n  -i, --init                         Build all tags for current mode\n  -p, --purge                        Delete binaries for all tags for current mode\n  -d, --delete                       Delete binary for passed tag\n  -b, --build                        Build binary for passed tag\n  -r, --run                          Run binary for passed tag\n  -l, --list                         Print supported tags for current mode\n  -L, --list-all                     Print supported tags for all modes\n  -q, --quiet                        Less output\n  -s, --silent                       Almost no output\n  -V, --verbose <VERBOSE>            More output [default: 3]\n  -w, --watch                        Report timer\n  -v, --version                      Print current version and quit\n  -W, --warranty                     Print warranty info and quit\n  -X, --no-gitcheck                  Ignore git mode checks\n  -J, --logged                       Output to log file\n  -P, --no-color                     Disable color output\n  -F, --force                        Enable force build\n  -R, --no-rebuild                   Disable calling make rebuild\n  -C, --config <CONFIG_ARG>          Pass configuration argument\n  -Z, --cflags <CFLAGS>              Pass CFLAGS for single file mode\n  -e, --strict                       Turn off extensions to 2.0\n  -h, --help                         Print help\n");
}

#ifdef CANVIL_NOGIT2
int canvil_nogit_status(void){
    /*
    const char* cmd_args[2] = {
        [0] = "git status",
        [1] = NULL,
    };
    Koliseo* k = kls_new(KLS_DEFAULT_SIZE);
    Koliseo_Temp* kls_t = kls_temp_start(k);
    Komando cmd = new_shell_command_kls_t(1, cmd_args, kls_t);
    bool run_res = run_command(cmd);
    kls_free(k);
    */

#ifndef _WIN32
    FILE *fp = popen("git status --untracked-files=no --porcelain", "r");
#else
    FILE *fp = _popen("git status --untracked-files=no --porcelain", "r");
#endif // _WIN32

    int c = fgetc(fp);
    bool has_output = false;

    if (c == EOF) {
        return 0;
    } else {
        has_output = true;
        putchar(c);
        while ((c = fgetc(fp)) != EOF) {
            putchar(c);
        }
    }

#ifndef _WIN32
    int status = pclose(fp);
#else
    int status = _pclose(fp);
#endif // _WIN32

#ifndef _WIN32
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        return has_output && WEXITSTATUS(status);
    }
#else
    if (status != 0) {
        return has_output && status;
    }
#endif // _WIN32
    return has_output;
}
#endif // CANVIL_NOGIT2

int has_uncommitted_changes(const char *repo_path) {
    int has_changes = 0;
#ifndef CANVIL_NOGIT2
    git_repository *repo = NULL;
    int error = git_repository_open_ext(&repo, repo_path, 0, NULL);
    if (error != 0) {
        printf("Failed to open repository at '%s'.\n", repo_path);
        return -1; // Return an error code
    }


    // Iterate over the status of the repository
    git_status_options options = GIT_STATUS_OPTIONS_INIT;
    options.show = GIT_STATUS_SHOW_INDEX_AND_WORKDIR;
    //options.flags = GIT_STATUS_OPT_INCLUDE_UNTRACKED | GIT_STATUS_OPT_RECURSE_UNTRACKED_DIRS;

    git_status_list *status_list = NULL;
    error = git_status_list_new(&status_list, repo, &options);
    if (error != 0) {
        printf("Failed to get status of repository at '%s'.\n", repo_path);
        git_repository_free(repo);
        return -1; // Return an error code
    }

    size_t count = git_status_list_entrycount(status_list);
    for (size_t i = 0; i < count; ++i) {
        const git_status_entry *entry = git_status_byindex(status_list, i);
        if (entry->status != GIT_STATUS_CURRENT) {
            has_changes = 1;
            break;
        }
    }

    git_status_list_free(status_list);
    git_repository_free(repo);
#else
    has_changes = canvil_nogit_status();
#endif // CANVIL_NOGIT2

    return has_changes;
}

int check_path_is_clean_repo(const char* path, Anvil_Args args, Spuro logger)
{
    int res = -1;
#ifndef CANVIL_NOGIT2
    git_libgit2_init(); // Initialize libgit2

    // Check if the given path is a repository
    git_repository *repo = NULL;
    int error = git_repository_open_ext(&repo, path, 0, NULL);
    if (error == 0) {
        spr_logf_to(logger, SPR_DEBUG, "The path '%s' is a Git repository.", path);

        // Get repository information
        const char *repo_path = git_repository_path(repo);
        spr_logf_to(logger, SPR_DEBUG, "Repository root path: %s", repo_path);
#else
        const char* repo_path = path;
#endif // CANVIL_NOGIT2

        // Check for uncommitted changes
        int changes = has_uncommitted_changes(repo_path);
        if (changes == -1) {
            spr_logf_to(logger, SPR_ERROR, "Failed to check for uncommitted changes.");
        } else if (changes == 1) {
            spr_logf_to(logger, SPR_ERROR, "The repository has uncommitted changes.");
            res = 1;
        } else {
            spr_logf_to(logger, SPR_DEBUG, "The repository has no uncommitted changes.");
            res = 0;
        }

#ifndef CANVIL_NOGIT2
        // Free the repository object
        git_repository_free(repo);
    } else {
        if (args.strict == 0) {
            if (error == GIT_ENOTFOUND) {
                res = 0;
            } else {
                spr_logf_to(logger, SPR_ERROR, "An error occurred while opening the path '%s' as a Git repository", path);
            }
        } else {
            spr_logf_to(logger, SPR_ERROR, "The path '%s' is not a Git repository or an error occurred.", path);
        }
    }
    git_libgit2_shutdown(); // Shutdown libgit2
#endif // CANVIL_NOGIT2

    return res;
}

Spuro extra_logger;

void close_extra_logger(void)
{
    fclose(extra_logger.fp);
}

void canvil_report_elapsed(int watch, DumbTimer timer, Spuro logger)
{
    if (watch == 1) {
        double elapsed = dt_elapsed(&timer);
        spr_logf_to(logger, SPR_INFO, "Elapsed: {%lf s}", elapsed);
    }
}

#define BUF_SIZE 8192
int copy_file(const char *src, const char *dst) {
    FILE *in = fopen(src, "rb");
    if (!in) {
        perror("fopen src");
        return -1;
    }
    FILE *out = fopen(dst, "wb");
    if (!out) {
        perror("fopen dst");
        fclose(in);
        return -1;
    }

    char buf[BUF_SIZE];
    size_t n;
    while ((n = fread(buf, 1, BUF_SIZE, in)) > 0) {
        if (fwrite(buf, 1, n, out) != n) {
            perror("fwrite");
            fclose(in);
            fclose(out);
            return -1;
        }
    }

    fclose(in);
    fclose(out);
    return 0;
}

int copy_directory(const char *src, const char *dst) {
    DIR *dir = opendir(src);
    if (!dir) {
        perror("opendir");
        return -1;
    }

    // Create destination directory (ignore if already exists)
#ifndef _WIN32
    mkdir(dst, 0755);
#else
    mkdir(dst);
#endif // _WIN32

    struct dirent *entry;
    Koliseo* kls = kls_new(KLS_DEFAULT_SIZE);
    while ((entry = readdir(dir)) != NULL) {
        Koliseo_Temp* kls_t = kls_temp_start(kls);
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0) {
            kls_temp_end(kls_t);
            continue;
        }

#ifndef _WIN32
        char* src_path = KLS_SPRINTF_T(kls_t, "%s/%s", src, entry->d_name);
#else
        char* src_path = KLS_SPRINTF_T(kls_t, "%s\\%s", src, entry->d_name);
#endif // _WIN32
#ifndef _WIN32
        char* dst_path = KLS_SPRINTF_T(kls_t, "%s/%s", dst, entry->d_name);
#else
        char* dst_path = KLS_SPRINTF_T(kls_t, "%s\\%s", dst, entry->d_name);
#endif // _WIN32

        struct stat st;
        if (stat(src_path, &st) == -1) {
            perror("stat");
            closedir(dir);
            kls_free(kls);
            return -1;
        }

        if (S_ISDIR(st.st_mode)) {
            // Recurse into subdirectory
            if (copy_directory(src_path, dst_path) == -1) {
                closedir(dir);
                kls_free(kls);
                return -1;
            }
        } else if (S_ISREG(st.st_mode)) {
            // Copy file
            if (copy_file(src_path, dst_path) == -1) {
                closedir(dir);
                kls_free(kls);
                return -1;
            }
        }
        kls_temp_end(kls_t);
    }

    closedir(dir);
    kls_free(kls);
    return 0;
}

int canvil_main(int argc, char** argv, Koliseo* default_kls)
{
    Spuro logger = {
        .out = SPR_STDERR,
        .fp = NULL,
        .lvl = SPR_WARN,
        .timed = false,
        .colored = SPR_COLORED_HEADER,
        .traced = false,
    };

    Anvil_Args canvil_args = {
        .version = 0,
        .help = 0,
        .linter = 0,
        .base_mode = 0,
        .git_mode = 0,
        .passed_stego_dir = 0,
        .stegodir_optarg = NULL,
        .passed_target_dir = 0,
        .passed_builds_dir = 0,
        .builds_dir_optarg = NULL,
        .targetdir_optarg = NULL,
        .passed_source_name = 0,
        .source_optarg = NULL,
        .passed_minmake_tag = 0,
        .minmake_optarg = NULL,
        .passed_kazoj_dir = 0,
        .kazojdir_optarg = NULL,
        .passed_config_arg = 0,
        .config_optarg = NULL,
        .ignore_gitcheck = 0,
        .list = 0,
        .list_all = 0,
        .do_init = 0,
        .do_purge = 0,
        .do_delete = 0,
        .do_build = 0,
        .do_run = 0,
        .do_test = 0,
        .passed_test_name = NULL,
        .do_test_macro = 0,
        .strict = 0,
        .passed_anvil_version = 0,
        .anvil_version_optarg = EXPECTED_AMBOSO_API_LEVEL,
        .passed_anvil_kern = 0,
        .anvil_kern_optarg = "amboso-C",
        .anvil_custombuilder = NULL,
        .passed_cflags = 0,
        .cflags_optarg = NULL,
        .minautomake_version = NULL,
        .watch = 0,
    };
    char *arg;
    int opt, longindex;
    struct optparse options;
    struct optparse_long longopts[] = {
        {"version", 'v', OPTPARSE_NONE},
        {"warranty", 'W', OPTPARSE_NONE},
        {"help", 'h', OPTPARSE_NONE},
        {"linter", 'x', OPTPARSE_REQUIRED},
        {"stego-dir", 'O', OPTPARSE_REQUIRED},
        {"builds-dir", 'I', OPTPARSE_REQUIRED},
        {"amboso-dir", 'D', OPTPARSE_REQUIRED},
        {"kazoj-dir", 'K', OPTPARSE_REQUIRED},
        {"source", 'S', OPTPARSE_REQUIRED},
        {"execname", 'E', OPTPARSE_REQUIRED},
        {"maketag", 'M', OPTPARSE_REQUIRED},
        {"verbose", 'V', OPTPARSE_REQUIRED},
        {"gen-c-header", 'G', OPTPARSE_REQUIRED},
        {"config", 'C', OPTPARSE_REQUIRED},
        {"no-gitcheck", 'X', OPTPARSE_NONE},
        {"list", 'l', OPTPARSE_NONE},
        {"list-all", 'L', OPTPARSE_NONE},
        {"base", 'B', OPTPARSE_NONE},
        {"git", 'g', OPTPARSE_NONE},
        {"init", 'i', OPTPARSE_NONE},
        {"purge", 'p', OPTPARSE_NONE},
        {"delete", 'd', OPTPARSE_NONE},
        {"build", 'b', OPTPARSE_NONE},
        {"run", 'r', OPTPARSE_NONE},
        {"test", 'T', OPTPARSE_REQUIRED},
        {"testmacro", 't', OPTPARSE_NONE},
        {"quiet", 'q', OPTPARSE_NONE},
        {"silent", 's', OPTPARSE_NONE},
        {"force", 'F', OPTPARSE_NONE},
        {"no-rebuild", 'R', OPTPARSE_NONE},
        {"no-color", 'P', OPTPARSE_NONE},
        {"logged", 'J', OPTPARSE_NONE},
        {"strict", 'e', OPTPARSE_NONE},
        {"anvil-version", 'a', OPTPARSE_REQUIRED},
        {"anvil-kern", 'k', OPTPARSE_REQUIRED},
        {"cflags", 'Z', OPTPARSE_REQUIRED},
        {"watch", 'w', OPTPARSE_NONE},
        {0, 0, 0}
    };

    optparse_init(&options, argv);
    while ((opt = optparse_long(&options, longopts, &longindex)) != -1) {
        switch (opt) {
            case 'O': {
                canvil_args.passed_stego_dir = 1;
                if (strlen(options.optarg) >= (FILENAME_MAX - strlen("Xstego.lock") -1)) {
                    spr_logf_to(logger, SPR_ERROR, "Passed stego_dir arg is too big: {%i} >= {%li}", strlen(options.optarg), (FILENAME_MAX - strlen("Xstego.lock") -1));
                    return 1;
                }
                canvil_args.stegodir_optarg = KLS_PUSH_ARR(default_kls, char, strlen(options.optarg)+1);
                memcpy(canvil_args.stegodir_optarg, options.optarg, strlen(options.optarg)+1);
            }
            break;
            case 'I': {
                canvil_args.passed_builds_dir = 1;
                if (strlen(options.optarg) >= (FILENAME_MAX)) {
                    spr_logf_to(logger, SPR_ERROR, "Passed builds_dir arg is too big: {%i} >= {%li}", strlen(options.optarg), (FILENAME_MAX));
                    return 1;
                }
                canvil_args.builds_dir_optarg = KLS_PUSH_ARR(default_kls, char, strlen(options.optarg)+1);
                memcpy(canvil_args.builds_dir_optarg, options.optarg, strlen(options.optarg)+1);
            }
            break;
            case 'D': {
                canvil_args.passed_target_dir = 1;
                if (strlen(options.optarg) >= (FILENAME_MAX)) {
                    spr_logf_to(logger, SPR_ERROR, "Passed target_dir arg is too big: {%i} >= {%li}", strlen(options.optarg), (FILENAME_MAX));
                    return 1;
                }
                canvil_args.targetdir_optarg = KLS_PUSH_ARR(default_kls, char, strlen(options.optarg)+1);
                memcpy(canvil_args.targetdir_optarg, options.optarg, strlen(options.optarg)+1);
            }
            break;
            case 'K': {
                canvil_args.passed_kazoj_dir = 1;
                if (strlen(options.optarg) >= (FILENAME_MAX)) {
                    spr_logf_to(logger, SPR_ERROR, "Passed kazoj_dir arg is too big: {%i} >= {%li}", strlen(options.optarg), (FILENAME_MAX));
                    return 1;
                }
                canvil_args.kazojdir_optarg = KLS_PUSH_ARR(default_kls, char, strlen(options.optarg)+1);
                memcpy(canvil_args.kazojdir_optarg, options.optarg, strlen(options.optarg)+1);
            }
            break;
            case 'V': {
                size_t arg_len = strlen(options.optarg);
                if (arg_len < 1) {
                    spr_logf_to(logger, SPR_ERROR, "Passed argument is not long enough");
                    return 1;
                }
                if (isdigit(options.optarg[0])) {
                    int verb_val = atoi(options.optarg);
                    if (verb_val < 0 || verb_val > 5) {
                        spr_logf_to(logger, SPR_ERROR, "Passed argument is not a valid digit: {%i}", verb_val);
                        return 1;
                    } else {
                        spr_logf_to(logger, SPR_INFO, "Setting verbose level: {%i}", verb_val);
                        logger.lvl = verb_val;
                    }
                } else {
                    spr_logf_to(logger, SPR_ERROR, "Passed argument is not a digit: {%s}", options.optarg);
                    return 1;
                }
            }
            break;
            case 'q': {
                // Lower logger level
                if (logger.lvl > 0) logger.lvl -= 1;
            }
            break;
            case 's': {
                // Zero logger level
                logger.lvl = 0;
            }
            break;
            case 'S': {
                canvil_args.passed_source_name = 1;
                if (strlen(options.optarg) >= (FILENAME_MAX)) {
                    spr_logf_to(logger, SPR_ERROR, "Passed source_name arg is too big: {%i} >= {%li}", strlen(options.optarg), (FILENAME_MAX));
                    return 1;
                }
                canvil_args.source_optarg = KLS_PUSH_ARR(default_kls, char, strlen(options.optarg)+1);
                memcpy(canvil_args.source_optarg, options.optarg, strlen(options.optarg)+1);
            }
            break;
            case 'E': {
                canvil_args.passed_bin_name = 1;
                if (strlen(options.optarg) >= (FILENAME_MAX)) {
                    spr_logf_to(logger, SPR_ERROR, "Passed bin_name arg is too big: {%i} >= {%li}", strlen(options.optarg), (FILENAME_MAX));
                    return 1;
                }
                canvil_args.bin_optarg = KLS_PUSH_ARR(default_kls, char, strlen(options.optarg)+1);
                memcpy(canvil_args.bin_optarg, options.optarg, strlen(options.optarg)+1);
            }
            break;
            case 'M': {
                canvil_args.passed_minmake_tag = 1;
                if (strlen(options.optarg) >= (FILENAME_MAX)) {
                    spr_logf_to(logger, SPR_ERROR, "Passed minmake_tag arg is too big: {%i} >= {%li}", strlen(options.optarg), (FILENAME_MAX));
                    return 1;
                }
                canvil_args.minmake_optarg = KLS_PUSH_ARR(default_kls, char, strlen(options.optarg)+1);
                memcpy(canvil_args.minmake_optarg, options.optarg, strlen(options.optarg)+1);
            }
            break;
            case 'x': {
                canvil_args.linter = 1;
                if (strlen(options.optarg) >= (FILENAME_MAX)) {
                    spr_logf_to(logger, SPR_ERROR, "Passed linter_target arg is too big: {%i} >= {%li}", strlen(options.optarg), (FILENAME_MAX));
                    return 1;
                }
                canvil_args.linter_optarg = KLS_PUSH_ARR(default_kls, char, strlen(options.optarg)+1);
                memcpy(canvil_args.linter_optarg, options.optarg, strlen(options.optarg)+1);
            }
            break;
            case 'h': {
                canvil_args.help = 1;
            }
            break;
            case 'v': {
                canvil_args.version = 1;
            }
            break;
            case 'W': {
                canvil_args.warranty = 1;
            }
            break;
            case 'l': {
                canvil_args.list = 1;
            }
            break;
            case 'L': {
                canvil_args.list_all = 1;
            }
            break;
            case 'B': {
                if (canvil_args.git_mode > 0) {
                    spr_logf_to(logger, SPR_ERROR, "Can't specify both -B and -g");
                    return 1;
                }
                canvil_args.base_mode = 1;
            }
            break;
            case 'g': {
                if (canvil_args.base_mode > 0) {
                    spr_logf_to(logger, SPR_ERROR, "Can't specify both -B and -g");
                    return 1;
                }
                canvil_args.git_mode = 1;
            }
            break;
            case 'X': {
                canvil_args.ignore_gitcheck = 1;
            }
            break;
            case 'i': {
                canvil_args.do_init = 1;
            }
            break;
            case 'p': {
                canvil_args.do_purge = 1;
            }
            break;
            case 'd': {
                canvil_args.do_delete = 1;
            }
            break;
            case 'b': {
                canvil_args.do_build = 1;
            }
            break;
            case 'r': {
                canvil_args.do_run = 1;
            }
            break;
            case 't': {
                canvil_args.do_test_macro = 1;
            }
            break;
            case 'T': {
                canvil_args.do_test = 1;
                if (strlen(options.optarg) >= (FILENAME_MAX)) {
                    spr_logf_to(logger, SPR_ERROR, "Passed test_target arg is too big: {%i} >= {%li}", strlen(options.optarg), (FILENAME_MAX));
                    return 1;
                }
                canvil_args.passed_test_name = KLS_PUSH_ARR(default_kls, char, strlen(options.optarg)+1);
                memcpy(canvil_args.passed_test_name, options.optarg, strlen(options.optarg)+1);
            }
            break;
            case 'G': {
                canvil_args.gen_header = 1;
                if (strlen(options.optarg) >= (FILENAME_MAX)) {
                    spr_logf_to(logger, SPR_ERROR, "Passed dir_name arg is too big: {%i} >= {%li}", strlen(options.optarg), (FILENAME_MAX));
                    return 1;
                }
                canvil_args.passed_gen_header_dir = KLS_PUSH_ARR(default_kls, char, strlen(options.optarg)+1);
                memcpy(canvil_args.passed_gen_header_dir, options.optarg, strlen(options.optarg)+1);
            }
            break;
            case 'C': {
                canvil_args.passed_config_arg = 1;
                if (strlen(options.optarg) >= (FILENAME_MAX)) {
                    spr_logf_to(logger, SPR_ERROR, "Passed config_arg is too big: {%i} >= {%li}", strlen(options.optarg), (FILENAME_MAX));
                    return 1;
                }
                canvil_args.config_optarg = KLS_PUSH_ARR(default_kls, char, strlen(options.optarg)+1);
                memcpy(canvil_args.config_optarg, options.optarg, strlen(options.optarg)+1);
            }
            break;
            case 'F': {
                canvil_args.force_build = 1;
            }
            break;
            case 'R': {
                canvil_args.no_rebuild = 1;
            }
            break;
            case 'P': {
                canvil_args.no_color = 1;
                logger.colored = SPR_COLORED_NONE;
            }
            break;
            case 'J': {
                canvil_args.logged = 1;
                FILE* log_file = fopen("./anvil.log", "w");
                extra_logger = spr_new_file(log_file);
                extra_logger.lvl = SPR_WARN;
                spr_add_tee(&logger, &extra_logger);
                atexit(&close_extra_logger);
            }
            break;
            case 'e': {
                canvil_args.strict = 1;
            }
            break;
            case 'a': {
                canvil_args.passed_anvil_version = 1;
                int major = -1;
                int minor = -1;
                int patch = -1;
                if (!parseSemVer(options.optarg, &major, &minor, &patch)) {
                    spr_logf_to(logger, SPR_ERROR, "Passed anvil_version is not a valid semver");
                    return 1;
                } else {
                    bool matched = false;
                    SemVer target = {
                        .major = major,
                        .minor = minor,
                        .patch = patch
                    };
                    for (int i=0; i < (sizeof(supported_anvil_versions)/sizeof(SemVer)) && !matched; i++) {
                        if (canvil_SemVer_cmp(target, supported_anvil_versions[i]) == 0) {
                            matched = true;
                        }
                    }
                    if (!matched) {
                        spr_logf_to(logger, SPR_ERROR, "Passed anvil_version {%s} is not a supported anvil version", options.optarg);
                        return 1;
                    } else {
                        if (canvil_SemVer_cmp(target, MIN_AMBOSO_V_EXTENSIONS) < 0) {
                            spr_logf_to(logger, SPR_DEBUG, "Turning off extensions");
                            canvil_args.strict = 1;
                        }
                    }
                }
                canvil_args.anvil_version_optarg = KLS_PUSH_ARR(default_kls, char, strlen(options.optarg)+1);
                memcpy(canvil_args.anvil_version_optarg, options.optarg, strlen(options.optarg)+1);
            }
            break;
            case 'k': {
                if (canvil_args.strict == 0) {
                    canvil_args.passed_anvil_kern = 1;
                    bool matched = false;
                    if (!strcmp(options.optarg, "amboso-C") ||
                            !strcmp(options.optarg, "anvilPy") ||
                            !strcmp(options.optarg, "custom")) {
                        matched = true;
                    }

                    if (matched) {
                        spr_logf_to(logger, SPR_DEBUG, "Using anvil_kern {%s}", options.optarg);
                        canvil_args.anvil_kern_optarg = KLS_PUSH_ARR(default_kls, char, strlen(options.optarg)+1);
                        memcpy(canvil_args.anvil_kern_optarg, options.optarg, strlen(options.optarg)+1);
                    } else {
                        spr_logf_to(logger, SPR_ERROR, "Unsupported anvil_kern: {%s}", options.optarg);
                        return 1;
                    }
                }
            }
            break;
            case 'Z': {
                canvil_args.passed_cflags = 1;
                if (strlen(options.optarg) >= (FILENAME_MAX)) {
                    spr_logf_to(logger, SPR_ERROR, "Passed cflags arg is too big: {%i} >= {%li}", strlen(options.optarg), (FILENAME_MAX));
                    return 1;
                }
                canvil_args.cflags_optarg = KLS_PUSH_ARR(default_kls, char, strlen(options.optarg)+1);
                memcpy(canvil_args.cflags_optarg, options.optarg, strlen(options.optarg)+1);
            }
            break;
            case 'w': {
                canvil_args.watch = 1;
            }
            break;
            case '?': {
                printf("%s: %s\n", argv[0], options.errmsg);
                spr_tracef("ERROR\n");
                return 1;
            }
            break;
            default: {/* '?' */
                canvil_usage(argv[0]);
                return 1;
            }
            break;
        }
    }

    DumbTimer timer = {0};

    if (canvil_args.watch == 1) {
        timer = dt_new();
    }

    if (canvil_args.do_test == 1 && canvil_args.do_init == 1) {
        SemVer target = {0};
        parseSemVer(canvil_args.anvil_version_optarg, &(target.major), &(target.minor), &(target.patch));
        if (canvil_SemVer_cmp(target, MIN_AMBOSO_V_REFUSE_TI) >= 0) {
            spr_logf_to(logger, SPR_ERROR, "Combining -T and -i is not permitted");
            canvil_report_elapsed(canvil_args.watch, timer, logger);
            return 1;
        }
    }

    if (canvil_args.git_mode == 0 && canvil_args.base_mode == 0) {
        spr_logf_to(logger, SPR_DEBUG, "Setting git mode by default");
        canvil_args.git_mode = 1;
    }

    //int res = -1;
    if (canvil_args.help == 1) {
        canvil_help(argv[0]);
        canvil_report_elapsed(canvil_args.watch, timer, logger);
        return 0;
    }

    if (canvil_args.version == 1) {
        if (logger.lvl > 3) {
            printf("%s v%s (Compat: %s)\n", argv[0], CANVIL_API_VERSION_STRING, canvil_args.anvil_version_optarg);
        } else {
            printf("%s v%s\n", argv[0], CANVIL_API_VERSION_STRING);
        }
        canvil_report_elapsed(canvil_args.watch, timer, logger);
        return 0;
    }

    printf("%s, version %s\nCopyright (C) 2024-2026  jgabaut\n\n  This program comes with ABSOLUTELY NO WARRANTY; for details type `%s -W`.\n  This is free software, and you are welcome to redistribute it\n  under certain conditions; see file `LICENSE` for details.\n\n  Full source is available at https://github.com/jgabaut/canvil\n", argv[0], CANVIL_API_VERSION_STRING, argv[0]);

    if (canvil_args.warranty == 1) {
        printf("\n  %s\n\n", "THERE IS NO WARRANTY FOR THE PROGRAM, TO THE EXTENT PERMITTED BY\n  APPLICABLE LAW.  EXCEPT WHEN OTHERWISE STATED IN WRITING THE COPYRIGHT\n  HOLDERS AND/OR OTHER PARTIES PROVIDE THE PROGRAM \"AS IS\" WITHOUT WARRANTY\n  OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,\n  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR\n  PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE PROGRAM\n  IS WITH YOU.  SHOULD THE PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF\n  ALL NECESSARY SERVICING, REPAIR OR CORRECTION.");
    }

    char stego_pathbuf[FILENAME_MAX] = {0};
    if (canvil_args.passed_stego_dir == 1) {
        spr_logf_to(logger, SPR_INFO, "Using passed stego_dir: {%s}", canvil_args.stegodir_optarg);
#ifndef _WIN32
        sprintf(stego_pathbuf, "%s/stego.lock", canvil_args.stegodir_optarg);
#else
        sprintf(stego_pathbuf, "%s\\stego.lock", canvil_args.stegodir_optarg);
#endif
    } else {
        const char* default_stegodir = ".";
        spr_logf_to(logger, SPR_DEBUG, "Using default stego_dir: {%s}", default_stegodir);
        // Use "." for current dir.
#ifndef _WIN32
        sprintf(stego_pathbuf, "%s/stego.lock", default_stegodir);
#else
        sprintf(stego_pathbuf, "%s\\stego.lock", default_stegodir);
#endif
        canvil_args.stegodir_optarg = KLS_PUSH_STR(default_kls, default_stegodir);
        memcpy(canvil_args.stegodir_optarg, default_stegodir, strlen(default_stegodir)+1);
    }


    char builds_dir_pathbuf[FILENAME_MAX] = {0};
    if (canvil_args.passed_builds_dir == 1) {
        spr_logf_to(logger, SPR_INFO, "Using passed builds_dir: {%s}", canvil_args.builds_dir_optarg);
        sprintf(builds_dir_pathbuf, "%s", canvil_args.builds_dir_optarg);
    } else {
        const char* default_builds_dir = ".";
        spr_logf_to(logger, SPR_DEBUG, "Using default builds_dir: {%s}", default_builds_dir);
        // Use "." for current dir.
        sprintf(builds_dir_pathbuf, "%s", default_builds_dir);
    }

    spr_logf_to(logger, SPR_TRACE, "Setting args builds_dir to {%s}", builds_dir_pathbuf);
    canvil_args.builds_dir_optarg = builds_dir_pathbuf;


    if (canvil_args.linter == 1) {
        assert(canvil_args.linter_optarg != NULL);
        Canvil_Lint_Mode lint_mode = CANVIL_LINT_FULL_CHECK;
        if (canvil_args.list_all) {
            spr_logf_to(logger, SPR_DEBUG, "Doing -L linting");
            lint_mode = CANVIL_LINT_LEX;
        }
        if (canvil_args.list) {
            spr_logf_to(logger, SPR_DEBUG, "Doing -l linting");
            lint_mode = CANVIL_LINT_ONLY;
        }
        if (!lint_stegopath(canvil_args.linter_optarg, lint_mode, logger)) {
            spr_logf_to(logger, SPR_ERROR, "Failed lint for {%s}", canvil_args.linter_optarg);
            canvil_report_elapsed(canvil_args.watch, timer, logger);
            return 1;
        } else {
            spr_logf_to(logger, SPR_DEBUG, "Lint success for {%s}", canvil_args.linter_optarg);
            canvil_report_elapsed(canvil_args.watch, timer, logger);
            return 0;
        }
    }

    spr_clogf_to(logger, SPR_MAGENTA, SPR_INFO, "%s v %s", argv[0], CANVIL_API_VERSION_STRING);
    spr_clogf_to(logger, SPR_YELLOW, SPR_INFO, "Using koliseo v%s", string_koliseo_version());

    canvil_args.extra_args = calloc(argc-1, sizeof(char*));
    canvil_args.extra_args_len = 0;
    int count_args = 0;
    while ((arg = optparse_arg(&options))) {
        if (count_args > 0) {
            spr_logf_to(logger, SPR_TRACE, "Extra arg: {%s}", arg);
            canvil_args.extra_args[canvil_args.extra_args_len] = KLS_PUSH_STR(default_kls, arg);
            memcpy(canvil_args.extra_args[canvil_args.extra_args_len], arg, strlen(arg)+1);
            canvil_args.extra_args_len += 1;
        }
        count_args++;
    }
    canvil_args.extra_args = realloc(canvil_args.extra_args, canvil_args.extra_args_len*(sizeof(char*)));
    //Subcommands
    if (count_args > 0) {
        if (!strcmp(argv[argc - count_args], "help")) {
            canvil_help(argv[0]);
            return 0;
        } else if (!strcmp(argv[argc - count_args], "init")) {
            if (count_args < 2) {
                spr_logf_to(logger, SPR_ERROR, "Missing target name for init subcommand");
                canvil_report_elapsed(canvil_args.watch, timer, logger);
                return 1;
            } else {
                char* template_name = NULL;
                if (!strcmp(canvil_args.anvil_kern_optarg, "custom")) {
                    if (count_args < 3) {
                        spr_logf_to(logger, SPR_ERROR, "Missing template name for init subcommand");
                        spr_logf_to(logger, SPR_ERROR, "Usage: canvil init -k custom %s <TEMPLATE>", argv[argc - count_args+1]);
                        canvil_report_elapsed(canvil_args.watch, timer, logger);
                        return 1;
                    } else {
                        template_name = argv[argc - count_args +2];
                        spr_logf_to(logger, SPR_DEBUG, "Doing init for {%s} template", template_name);
                    }
                }
                bool res = canvil_init_project(argv[argc - count_args +1], canvil_args.anvil_kern_optarg, template_name, logger, default_kls);
                canvil_report_elapsed(canvil_args.watch, timer, logger);
                if (res) return 0;
                return 1;
            }
            return 0;
        } else if (!strcmp(argv[argc - count_args], "version")) {
            printf("%s v%s\n", argv[0], CANVIL_API_VERSION_STRING);
            canvil_report_elapsed(canvil_args.watch, timer, logger);
            return 0;
        } else if (!strcmp(argv[argc - count_args], "test")) {
            Anvil_Env anvil_env = {0};
            if (!getargs_from_filepath_as_stego(&canvil_args, stego_pathbuf, default_kls, true, &anvil_env, logger)) {
                spr_clogf_to(logger, SPR_RED, SPR_ERROR, "Failed checking stego.lock at {%s}", stego_pathbuf);
                canvil_report_elapsed(canvil_args.watch, timer, logger);
                return 1;
            }
            Canvil_Test_List test_list = Canvil_Test_List_nullList();
            Canvil_Test_List errortest_list = Canvil_Test_List_nullList();
            test_list = anvil_env.tests;
            errortest_list = anvil_env.errortests;
            if (count_args > 1) {
                Canvil_Test query = (Canvil_Test){
                    .name = argv[argc - count_args +1],
                };
                bool is_bone_test = Canvil_Test_List_member(&query, test_list);
                bool is_kulpo_test = Canvil_Test_List_member(&query, errortest_list);
                if (! is_bone_test && ! is_kulpo_test) {
                    spr_logf_to(logger, SPR_ERROR, "Passed test is not in test list: {%s}", query.name);
                    return -1;
                } else {
                    // We need to retrieve the queried test from the proper list
                    bool found = false;
                    if (is_bone_test) {
                        while (!found && ! Canvil_Test_List_isEmpty(test_list)) {
                            Canvil_Test* node_pt = Canvil_Test_List_head(test_list);
                            if (canvil_test_cmp(node_pt, &query)) {
                                found = true;
                                query = *node_pt;
                            }
                            test_list = Canvil_Test_List_tail(test_list);
                        }
                    } else {
                        while (!found && ! Canvil_Test_List_isEmpty(errortest_list)) {
                            Canvil_Test* node_pt = Canvil_Test_List_head(errortest_list);
                            if (canvil_test_cmp(node_pt, &query)) {
                                found = true;
                                query = *node_pt;
                            }
                            test_list = Canvil_Test_List_tail(errortest_list);
                        }
                    }
                }
                bool test_res = canvil_op_test(&canvil_args, query, logger, default_kls);
                canvil_report_elapsed(canvil_args.watch, timer, logger);
                return test_res;
            } else if (canvil_args.list == 1) {
                while (! Canvil_Test_List_isEmpty(test_list)) {
                    Canvil_Test* node = Canvil_Test_List_head(test_list);
                    spr_logf_to(logger, SPR_INFO, "Test: {%s}", node->name);
                    test_list = Canvil_Test_List_tail(test_list);
                }
                while (! Canvil_Test_List_isEmpty(errortest_list)) {
                    Canvil_Test* node = Canvil_Test_List_head(errortest_list);
                    spr_logf_to(logger, SPR_INFO, "Test: {%s}", node->name);
                    errortest_list = Canvil_Test_List_tail(errortest_list);
                }
                return 0;
            } else {
                int res = canvil_op_test_macro(&canvil_args, test_list, errortest_list, logger, default_kls);
                canvil_report_elapsed(canvil_args.watch, timer, logger);
                return res;
            }
        }
    }

    Anvil_Env anvil_env = {0};
    if (!check_filepath_as_stego(stego_pathbuf, logger)) {
        spr_clogf_to(logger, SPR_RED, SPR_ERROR, "Failed checking stego.lock at {%s}", stego_pathbuf);
        canvil_report_elapsed(canvil_args.watch, timer, logger);
        return 1;
    }

    const char* home = getenv("HOME");
    char global_stego_path[FILENAME_MAX] = {0};
    sprintf(global_stego_path, "%s/.anvil/anvil.toml", home);

    FILE* global_stego_file = fopen(global_stego_path, "r");

    if (global_stego_file) {
        fclose(global_stego_file);
        if(!getargs_from_filepath_as_stego_global(&canvil_args, global_stego_path, default_kls, logger)) {
            spr_logf_to(logger, SPR_ERROR, "Failed checking anvil.toml at {%s}", global_stego_path);
            canvil_report_elapsed(canvil_args.watch, timer, logger);
            return 1;
        }
    }

    if (!getargs_from_filepath_as_stego(&canvil_args, stego_pathbuf, default_kls, true, &anvil_env, logger)) {
        spr_clogf_to(logger, SPR_RED, SPR_ERROR, "Failed checking stego.lock at {%s}", stego_pathbuf);
        canvil_report_elapsed(canvil_args.watch, timer, logger);
        return 1;
    }

    AnvilPy_Env canvil_py_env = {0};
    if (canvil_args.passed_anvil_kern == 1) {
        int major, minor, patch;

        parseSemVer(canvil_args.anvil_version_optarg, &major, &minor, &patch);
        SemVer target = { .major = major, .minor = minor, .patch = patch };

        if (canvil_SemVer_cmp(target, MIN_AMBOSO_V_KERN) < 0) {
            spr_logf_to(logger, SPR_DEBUG, "Ignoring passed anvil kern {%s}", canvil_args.passed_anvil_kern);
            memcpy(canvil_args.anvil_kern_optarg, "amboso-C", strlen("amboso-C")+1);
        }

        if (canvil_SemVer_cmp(target, MIN_AMBOSO_V_ANVILPY_KERN) >= 0) {
            if (!strcmp(canvil_args.anvil_kern_optarg, "anvilPy")) {
                bool res = anvilpy_getenv_from_filepath("./pyproject.toml", &canvil_py_env, default_kls, logger);
                if (res) spr_logf_to(logger, SPR_INFO, "Done reading anvilpy_env");
                print_anvilpy_env(canvil_py_env, logger);
            }
        }

        if (!strcmp(canvil_args.anvil_kern_optarg, "custom")) {
            if (!da_recipes_validate(anvil_env.recipes, logger)) {
                spr_clogf_to(logger, SPR_RED, SPR_ERROR, "Failed checking anvil_recipe");
                return 1;
            }
            da_recipes_sort(anvil_env.recipes, recipe_sorter);
            for (int i = 0; i < anvil_env.recipes->count; i++) {
                Anvil_Recipe* r = anvil_env.recipes->items[i];
                SemVer smv = *(r->vers);
                spr_logf_to(logger, SPR_DEBUG, "Recipe #%i build {%s} conf {%s} vers {" SemVer_Fmt "}", i, r->build, r->conf, SemVer_Arg(smv));
            }
        }
    }

    char targetdir_pathbuf[FILENAME_MAX] = {0};
    if (canvil_args.passed_target_dir == 1) {
        spr_logf_to(logger, SPR_INFO, "Using passed target_dir: {%s}", canvil_args.targetdir_optarg);
        sprintf(targetdir_pathbuf, "%s", canvil_args.targetdir_optarg);
    } else if (canvil_args.targetdir_optarg != NULL) {
        // We got a target dir from stego parsing in getargs_from_filepath_as_stego()
        spr_logf_to(logger, SPR_INFO, "Using stego defined target_dir: {%s}", canvil_args.targetdir_optarg);
        sprintf(targetdir_pathbuf, "%s", canvil_args.targetdir_optarg);
    } else {
        const char* default_targetdir = "./bin";
        spr_logf_to(logger, SPR_INFO, "Using default target_dir: {%s}", default_targetdir);
        sprintf(targetdir_pathbuf, "%s", default_targetdir);
    }

    spr_logf_to(logger, SPR_TRACE, "Setting args ambosodir to {%s}", targetdir_pathbuf);
    canvil_args.targetdir_optarg = targetdir_pathbuf;

    char sourcename_pathbuf[FILENAME_MAX] = {0};
    if (canvil_args.passed_source_name == 1) {
        spr_logf_to(logger, SPR_INFO, "Using passed source_name: {%s}", canvil_args.source_optarg);
        sprintf(sourcename_pathbuf, "%s", canvil_args.source_optarg);
    } else if (canvil_args.source_optarg != NULL) {
        // We got a source name from stego parsing in getargs_from_filepath_as_stego()
        spr_logf_to(logger, SPR_INFO, "Using stego defined source_name: {%s}", canvil_args.source_optarg);
        sprintf(sourcename_pathbuf, "%s", canvil_args.source_optarg);
    } else {
        const char* default_source_name = "";
        spr_logf_to(logger, SPR_INFO, "Using default source_name: {%s}", default_source_name);
        sprintf(sourcename_pathbuf, "%s", default_source_name);
    }

    spr_logf_to(logger, SPR_TRACE, "Setting args sourcename to {%s}", sourcename_pathbuf);
    canvil_args.source_optarg = sourcename_pathbuf;

    char binname_pathbuf[FILENAME_MAX] = {0};
    if (canvil_args.passed_bin_name == 1) {
        spr_logf_to(logger, SPR_INFO, "Using passed bin_name: {%s}", canvil_args.bin_optarg);
        sprintf(binname_pathbuf, "%s", canvil_args.bin_optarg);
    } else if (canvil_args.bin_optarg != NULL) {
        // We got a bin name from stego parsing in getargs_from_filepath_as_stego()
        spr_logf_to(logger, SPR_INFO, "Using stego defined bin_name: {%s}", canvil_args.bin_optarg);
        sprintf(binname_pathbuf, "%s", canvil_args.bin_optarg);
    } else {
        const char* default_bin_name = "";
        spr_logf_to(logger, SPR_INFO, "Using default bin_name: {%s}", default_bin_name);
        sprintf(binname_pathbuf, "%s", default_bin_name);
    }

    spr_logf_to(logger, SPR_TRACE, "Setting args binname to {%s}", binname_pathbuf);
    canvil_args.bin_optarg = binname_pathbuf;

    char minmake_buf[FILENAME_MAX] = {0};
    if (canvil_args.passed_minmake_tag == 1) {
        spr_logf_to(logger, SPR_INFO, "Using passed minmake_tag: {%s}", canvil_args.minmake_optarg);
        sprintf(minmake_buf, "%s", canvil_args.minmake_optarg);
    } else if (canvil_args.minmake_optarg != NULL) {
        // We got a min make tag from stego parsing in getargs_from_filepath_as_stego()
        spr_logf_to(logger, SPR_INFO, "Using stego defined minmake_tag: {%s}", canvil_args.minmake_optarg);
        sprintf(minmake_buf, "%s", canvil_args.minmake_optarg);
    } else {
        const char* default_minmake_tag = "9.9.9";
        spr_logf_to(logger, SPR_INFO, "Using default minmake_tag: {%s}", default_minmake_tag);
        sprintf(minmake_buf, "%s", default_minmake_tag);
    }

    spr_logf_to(logger, SPR_TRACE, "Setting args minmaketag to {%s}", minmake_buf);
    canvil_args.minmake_optarg = minmake_buf;

    //Build subcommand - needs Anvil_Env and setup of canvil_args
    if (count_args > 0) {
        if (!strcmp(argv[argc - count_args], "build")) {
            Canvil_Tag* latest_tag = Canvil_Tag_List_head(anvil_env.git_tags);
            char latest_tag_str[FILENAME_MAX] = {0};
            sprintf(latest_tag_str, "%i.%i.%i", latest_tag->version->major, latest_tag->version->minor, latest_tag->version->patch);

            spr_logf_to(logger, SPR_INFO, "Building latest tag: {%s}", latest_tag_str);
            if (!strcmp(canvil_args.anvil_kern_optarg, "custom")) {
                int major, minor, patch;

                parseSemVer(canvil_args.anvil_version_optarg, &major, &minor, &patch);
                SemVer target = { .major = major, .minor = minor, .patch = patch };
                if (canvil_SemVer_cmp(target, MIN_AMBOSO_V_ANVILCUSTOM_RECIPES) >= 0) {
                    Anvil_Recipe r = {0};
                    if (!find_recipe(anvil_env.recipes, *(latest_tag->version), &r)) {
                        spr_logf_to(logger, SPR_ERROR, "Could not find recipe for {%s}", latest_tag_str);
                        return 1;
                    }
                    spr_logf_to(logger, SPR_DEBUG, "Using custom builder {%s}", r.build);
                    canvil_args.anvil_custombuilder = r.build;
                }
            }

            bool build_res = canvil_op_build((canvil_args.git_mode == 1), (canvil_args.force_build == 1), (canvil_args.no_rebuild == 1), (canvil_args.passed_config_arg == 1), (canvil_args.config_optarg), canvil_args.minmake_optarg, canvil_args.minautomake_version, canvil_args.cflags_optarg, canvil_args.targetdir_optarg, canvil_args.builds_dir_optarg, latest_tag_str, canvil_args.bin_optarg, canvil_args.source_optarg, canvil_args.anvil_kern_optarg, canvil_args.anvil_version_optarg, canvil_py_env, canvil_args.anvil_custombuilder, canvil_args.extra_args, canvil_args.extra_args_len, logger, default_kls);
            canvil_report_elapsed(canvil_args.watch, timer, logger);

            if (build_res) return 0;
            return 1;
        }
    }

    int clean_repo_res = check_path_is_clean_repo(".", canvil_args, logger);

    if (clean_repo_res != 0) {
        spr_clogf_to(logger, SPR_RED, SPR_ERROR, "%s", "Failed git check.");
        if (canvil_args.ignore_gitcheck == 0) {
            canvil_report_elapsed(canvil_args.watch, timer, logger);
            return clean_repo_res;
        } else {
            spr_clogf_to(logger, SPR_YELLOW, SPR_WARN, "%s", "Ignoring failed git check");
        }
    }

    // Handle list flags
    if (canvil_args.list_all == 1) {
        canvil_print_tags(anvil_env);
        canvil_report_elapsed(canvil_args.watch, timer, logger);
        return 0;
    } else if (canvil_args.list == 1 && canvil_args.do_test_macro == 0) {
        if (canvil_args.git_mode > 0) {
            canvil_print_git_tags(anvil_env);
        } else {
            canvil_print_base_tags(anvil_env);
        }
        canvil_report_elapsed(canvil_args.watch, timer, logger);
        return 0;
    }

    int result = 0;
    bool no_op_requested = (canvil_args.do_init == 0 &&
            canvil_args.do_purge == 0 &&
            canvil_args.do_delete == 0 &&
            canvil_args.do_build == 0 &&
            canvil_args.do_run == 0 &&
            canvil_args.do_test == 0 &&
            canvil_args.do_test_macro == 0 &&
            canvil_args.gen_header == 0);

    if (count_args == 0 && no_op_requested) {
        if (!strcmp(canvil_args.anvil_kern_optarg, "amboso-C")) {
            spr_tracef("%s\n", "No argument after options");

            Koliseo_Temp* kls_t = kls_temp_start(default_kls);
            bool no_rebuild = true;
            bool use_autoconf = false;
            result = canvil_handle_make_call(use_autoconf, no_rebuild, (canvil_args.passed_config_arg == 1), (canvil_args.config_optarg), (canvil_args.extra_args), (canvil_args.extra_args_len), logger, kls_t);
            kls_temp_end(kls_t);
        } else if (!strcmp(canvil_args.anvil_kern_optarg, "anvilPy")) {
            Koliseo_Temp* kls_t = kls_temp_start(default_kls);
            result = canvil_py_handle_build(logger, canvil_args.builds_dir_optarg, kls_t);
            kls_temp_end(kls_t);
        } else if (!strcmp(canvil_args.anvil_kern_optarg, "custom")) {
            int major, minor, patch;

            parseSemVer(canvil_args.anvil_version_optarg, &major, &minor, &patch);
            SemVer target = { .major = major, .minor = minor, .patch = patch };
            if (canvil_SemVer_cmp(target, MIN_AMBOSO_V_ANVILCUSTOM_RECIPES) >= 0) {
                Anvil_Recipe r = {0};
                if (anvil_env.recipes->count > 0) {
                    r = *(anvil_env.recipes->items[0]);
                } else {
                    spr_logf_to(logger, SPR_ERROR, "Could not find recipes");
                    result = -1;
                    return result;
                }
                if (r.conf != NULL) {
                    spr_logf_to(logger, SPR_DEBUG, "Using custom configurer {%s}", r.conf);
                    Koliseo_Temp* kls_t = kls_temp_start(default_kls);
                    result = canvil_custom_handle_build(r.conf, canvil_args.targetdir_optarg, canvil_args.builds_dir_optarg, canvil_args.bin_optarg, "", canvil_args.extra_args, canvil_args.extra_args_len, logger, kls_t);
                    kls_temp_end(kls_t);
                }
                spr_logf_to(logger, SPR_DEBUG, "Using custom builder {%s}", r.build);
                canvil_args.anvil_custombuilder = r.build;
            }

            if (canvil_args.anvil_custombuilder == NULL) {
                spr_logf_to(logger, SPR_ERROR, "Missing custombuilder definition");
                result = -1;
                return result;
            }

            Koliseo_Temp* kls_t = kls_temp_start(default_kls);
            result = canvil_custom_handle_build(canvil_args.anvil_custombuilder, canvil_args.targetdir_optarg, canvil_args.builds_dir_optarg, canvil_args.bin_optarg, "", canvil_args.extra_args, canvil_args.extra_args_len, logger, kls_t);
            kls_temp_end(kls_t);
        }
    } else {
        // Pass on only the actual arguments, using count of args collected from the optparse_arg() loop
        // TODO: Program name is currently lost
        result = canvil_check_passed_args(&canvil_args, &anvil_env, canvil_py_env, &(argv[argc - count_args]), count_args, logger, default_kls);
    }
    canvil_report_elapsed(canvil_args.watch, timer, logger);
    return result;
}

int canvil_check_passed_args(Anvil_Args* canvil_args, Anvil_Env* canvil_env, AnvilPy_Env canvil_py_env, char** argv, size_t argc, Spuro logger, Koliseo* kls) {

    assert(canvil_args != NULL);
    assert(canvil_env != NULL);
    assert(kls != NULL);
    /*
    spr_logtf_to(logger, SPR_INFO, "argc: {%i}", argc);
    int last_useful_arg = 0;
    for (size_t i = last_useful_arg; i < argc; i++) {
        spr_logtf_to(logger, SPR_INFO, "Unused arg: {%s} {%i}", argv[i], i);
        (void) argv[i];
    }
    */

    if (canvil_args->gen_header == 1) {
        if (argc < 1) {
            spr_logf_to(logger, SPR_ERROR, "Missing argument: Tag. Argc: %i", argc);
            return -1;
        }
        const char* tagname = argv[0]; // We expect the first argument to be the tag name
        bool gen_res = canvil_gen_header(canvil_args->passed_gen_header_dir, canvil_args->anvil_kern_optarg, tagname, canvil_args->bin_optarg, logger, kls);
        if (gen_res) {
            spr_logf_to(logger, SPR_INFO, "Success in header gen for tag {%s}", tagname);
        } else {
            spr_logf_to(logger, SPR_ERROR, "Failed header gen for tag {%s}", tagname);
        }
    }


    assert(canvil_args->targetdir_optarg != NULL);

    bool amboso_dir_check = canvil_check_dir_create(canvil_args->targetdir_optarg);

    assert(amboso_dir_check);

    bool no_op_requested = (canvil_args->do_init == 0 &&
            canvil_args->do_purge == 0 &&
            canvil_args->do_delete == 0 &&
            canvil_args->do_build == 0 &&
            canvil_args->do_run == 0 &&
            canvil_args->do_test == 0 &&
            canvil_args->do_test_macro == 0);

    if (no_op_requested) {
        // do_query goes here
        if (argc < 1) {
            spr_logf_to(logger, SPR_ERROR, "Missing argument: Tag. Argc: %i", argc);
            spr_logf_to(logger, SPR_INFO, "Run with -h for help.");
            return -1;
        } else {
            const char* stego_lock = "stego.lock";
            size_t len = strlen(argv[0]);

            if (len >= strlen(stego_lock)) {
                if (!strcmp(argv[0] + len - strlen(stego_lock), stego_lock)) {
                    spr_logf_to(logger, SPR_DEBUG, "Interpreter branch");
                    Koliseo_Temp* kls_t = kls_temp_start(kls);
                    bool no_rebuild = true;
                    bool use_autoconf = false;
                    int result = canvil_handle_make_call(use_autoconf, no_rebuild, (canvil_args->passed_config_arg == 1), (canvil_args->config_optarg), (canvil_args->extra_args), (canvil_args->extra_args_len), logger, kls_t);
                    kls_temp_end(kls_t);
                    return result;
                }
            }
        }
    } else {

        if (canvil_args->do_test == 1) {
            Canvil_Test_List test_list = Canvil_Test_List_nullList();
            Canvil_Test_List errortest_list = Canvil_Test_List_nullList();
            test_list = canvil_env->tests;
            errortest_list = canvil_env->errortests;
            Canvil_Test query = (Canvil_Test){
                .name = canvil_args->passed_test_name
            };

            bool is_bone_test = Canvil_Test_List_member(&query, test_list);
            bool is_kulpo_test = Canvil_Test_List_member(&query, errortest_list);
            if (! is_bone_test && ! is_kulpo_test) {
                spr_logf_to(logger, SPR_ERROR, "Passed test is not in test list: {%s}", query.name);
                return -1;
            } else {
                // We need to retrieve the queried test from the proper list
                bool found = false;
                if (is_bone_test) {
                    while (!found && ! Canvil_Test_List_isEmpty(test_list)) {
                        Canvil_Test* node_pt = Canvil_Test_List_head(test_list);
                        if (canvil_test_cmp(node_pt, &query)) {
                            found = true;
                            query = *node_pt;
                        }
                        test_list = Canvil_Test_List_tail(test_list);
                    }
                } else {
                    while (!found && ! Canvil_Test_List_isEmpty(errortest_list)) {
                        Canvil_Test* node_pt = Canvil_Test_List_head(errortest_list);
                        if (canvil_test_cmp(node_pt, &query)) {
                            found = true;
                            query = *node_pt;
                        }
                        test_list = Canvil_Test_List_tail(errortest_list);
                    }
                }
            }
            spr_logf_to(logger, SPR_DEBUG, "Doing test: {%s}", query.name);
            bool test_res = canvil_op_test(canvil_args, query, logger, kls);
            return test_res ? 0 : 1;
        } else if (canvil_args->do_test_macro == 1) {
            Canvil_Test_List test_list = Canvil_Test_List_nullList();
            Canvil_Test_List errortest_list = Canvil_Test_List_nullList();
            test_list = canvil_env->tests;
            errortest_list = canvil_env->errortests;
            if (canvil_args->list == 1) {
                while (! Canvil_Test_List_isEmpty(test_list)) {
                    Canvil_Test* node = Canvil_Test_List_head(test_list);
                    spr_logf_to(logger, SPR_INFO, "Test: {%s}", node->name);
                    test_list = Canvil_Test_List_tail(test_list);
                }
                while (! Canvil_Test_List_isEmpty(errortest_list)) {
                    Canvil_Test* node = Canvil_Test_List_head(errortest_list);
                    spr_logf_to(logger, SPR_INFO, "Test: {%s}", node->name);
                    errortest_list = Canvil_Test_List_tail(errortest_list);
                }
                return 0;
            } else {
                int test_macro_res = canvil_op_test_macro(canvil_args, test_list, errortest_list, logger, kls);
                return test_macro_res;
            }
        }

        if (canvil_args->do_purge == 1) {
            Canvil_Tag_List tags_list = Canvil_Tag_List_nullList();
            if (canvil_args->git_mode == 1) {
                tags_list = canvil_env->git_tags;
            } else if (canvil_args->base_mode == 1) {
                tags_list = canvil_env->base_tags;
            } else {
                spr_logf_to(logger, SPR_ERROR, "Missing mode for purge op");
                return -1;
            }

            bool purge_res = canvil_op_purge(canvil_args->targetdir_optarg, tags_list, canvil_args->bin_optarg, logger, kls);
            if (purge_res) {
                spr_logf_to(logger, SPR_INFO, "Success purging {%s}", canvil_args->targetdir_optarg);
            } else {
                spr_logf_to(logger, SPR_ERROR, "Failed purging {%s}", canvil_args->targetdir_optarg);
                return -1;
            }
        }

        if (canvil_args->do_init == 1) {
            Canvil_Tag_List tags_list = Canvil_Tag_List_nullList();
            bool git_mode = true;
            if (canvil_args->git_mode == 1) {
                tags_list = canvil_env->git_tags;
            } else if (canvil_args->base_mode == 1) {
                tags_list = canvil_env->base_tags;
                git_mode = false;
            } else {
                spr_logf_to(logger, SPR_ERROR, "Missing mode for init op");
                return -1;
            }

            bool init_res = canvil_op_init(git_mode, (canvil_args->force_build == 1), (canvil_args->no_rebuild == 1), (canvil_args->passed_config_arg == 1), (canvil_args->config_optarg), canvil_args->minmake_optarg, canvil_args->minautomake_version, canvil_args->cflags_optarg, canvil_args->targetdir_optarg, tags_list, canvil_args->builds_dir_optarg, canvil_args->bin_optarg, canvil_args->source_optarg, canvil_args->anvil_kern_optarg, canvil_args->anvil_version_optarg, canvil_py_env, canvil_args->anvil_custombuilder, canvil_args->extra_args, canvil_args->extra_args_len, logger, kls);
            if (init_res) {
                spr_logf_to(logger, SPR_INFO, "Success init for {%s}", canvil_args->targetdir_optarg);
            } else {
                spr_logf_to(logger, SPR_ERROR, "Failed init for {%s}", canvil_args->targetdir_optarg);
                return -1;
            }
        }

        if (argc < 1) {
            spr_logf_to(logger, SPR_ERROR, "Missing argument: Tag. Argc: %i", argc);
            spr_logf_to(logger, SPR_INFO, "Run with -h for help.");
            return 1;
        }
        const char* tagname = argv[0]; // We expect the first argument to be the tag name
        if (!tagname) {
            spr_logf_to(logger, SPR_ERROR, "tagname was NULL");
            return -1;
        }
        Canvil_Tag_Type query_type = {0};
        Canvil_Tag_List tags_list = Canvil_Tag_List_nullList();
        if (canvil_args->git_mode == 1) {
            tags_list = canvil_env->git_tags;
            query_type = CANVIL_GIT_TAG;
        } else if (canvil_args->base_mode == 1) {
            tags_list = canvil_env->base_tags;
            query_type = CANVIL_BASE_TAG;
        } else {
            spr_logf_to(logger, SPR_ERROR, "Missing mode for build op");
            return -1;
        }
        int major = 0;
        int minor = 0;
        int patch = 0;
        if (!parseSemVer(tagname, &major, &minor, &patch)) {
            spr_logf_to(logger, SPR_ERROR, "Failed parsing tagname {%s} as SemVer", tagname);
            return -1;
        }
        SemVer semver = (SemVer) {
            .major = major,
            .minor = minor,
            .patch = patch
        };
        Canvil_Tag query = (Canvil_Tag){
            query_type,
            &semver,
            NULL
        };
        if (!Canvil_Tag_List_member(&query, tags_list)) {
            spr_logf_to(logger, SPR_ERROR, "Passed tag is not in tags list: {%s}", tagname);
            return -1;
        }
        if (canvil_args->do_delete == 1) {
            bool delete_res = canvil_op_delete(canvil_args->targetdir_optarg, tagname, canvil_args->bin_optarg, logger, kls);
            if (delete_res) {
                spr_logf_to(logger, SPR_INFO, "Success deleting {%s/v%s/%s}", canvil_args->targetdir_optarg, tagname, canvil_args->bin_optarg);
            } else {
                spr_logf_to(logger, SPR_ERROR, "Failed deleting {%s/v%s/%s}", canvil_args->targetdir_optarg, tagname, canvil_args->bin_optarg);
            }
        }

        if (canvil_args->do_build == 1) {
            bool build_res = canvil_op_build((canvil_args->git_mode == 1), (canvil_args->force_build == 1), (canvil_args->no_rebuild == 1), (canvil_args->passed_config_arg == 1), (canvil_args->config_optarg), canvil_args->minmake_optarg, canvil_args->minautomake_version, canvil_args->cflags_optarg, canvil_args->targetdir_optarg, canvil_args->builds_dir_optarg, tagname, canvil_args->bin_optarg, canvil_args->source_optarg, canvil_args->anvil_kern_optarg, canvil_args->anvil_version_optarg, canvil_py_env, canvil_args->anvil_custombuilder, canvil_args->extra_args, canvil_args->extra_args_len, logger, kls);
            if (build_res) {
                spr_logf_to(logger, SPR_INFO, "Success building {%s/v%s/%s}", canvil_args->targetdir_optarg, tagname, canvil_args->bin_optarg);
            } else {
                spr_logf_to(logger, SPR_ERROR, "Failed building {%s/v%s/%s}", canvil_args->targetdir_optarg, tagname, canvil_args->bin_optarg);
            }
        }

        if (canvil_args->do_run == 1) {
            bool run_res = canvil_op_run(canvil_args->targetdir_optarg, tagname, canvil_args->bin_optarg, logger, kls);
            if (run_res) {
                spr_logf_to(logger, SPR_INFO, "Success running {%s/v%s/%s}", canvil_args->targetdir_optarg, tagname, canvil_args->bin_optarg);
            } else {
                spr_logf_to(logger, SPR_ERROR, "Failed running {%s/v%s/%s}", canvil_args->targetdir_optarg, tagname, canvil_args->bin_optarg);
            }
        }
    }

    return 0;
}

/**
 * Returns the escaped version of passed C string, requesting memory from the passed Koliseo.
 * From https://stackoverflow.com/questions/3201451/how-to-convert-a-c-string-into-its-escaped-version-in-c
 * @param kls The Koliseo to get memory from.
 * @param cstr The C string to escape.
 * @return The escaped C string.
 */
static char* canvil_escape_str_kls(Koliseo* kls, const char* cstr)
{
    int i,j;
    int l = strlen(cstr) + 1;
    char esc_char[]= { '\a','\b','\f','\n','\r','\t','\v','\\'};
    char essc_str[]= {  'a', 'b', 'f', 'n', 'r', 't', 'v','\\'};
    char* dest  =  KLS_PUSH_ARR(kls, char, l*2);
    char* ptr = dest;
    for(i=0; i<l; i++){
        for(j=0; j< 8; j++){
            if(cstr[i] == esc_char[j]){
              *ptr++ = '\\';
              *ptr++ = essc_str[j];
                 break;
            }
        }
        if(j == 8) {
            *ptr++ = cstr[i];
        }
    }
    *ptr = '\0';
    return dest;
}

bool canvil_gen_header(const char* target_dir, const char* anvil_kern, const char* tag, const char* bin_name, Spuro logger, Koliseo* kls) {
    assert(target_dir != NULL);
    assert(tag != NULL);
    assert(bin_name != NULL);

    char sha1_oid[41]; // Buffer for formatted SHA1
    int64_t commit_time = -1;

    // We prepare the reference name to lookup
    // If we were looking for "HEAD", this part would not be necessary

    char tag_str[50]; // Buffer for reference name
    size_t tag_len = strlen(tag);
    size_t max_tag_len = 20;
    if (tag_len >= max_tag_len) {
        spr_logf_to(logger, SPR_ERROR, "Tag name was too long {%zu} > {%zu}", tag_len, max_tag_len);
        return false;
    } else {
        sprintf(tag_str, "refs/tags/%s", tag);
    }

    bool res = true;
#ifndef CANVIL_NOGIT2
    git_libgit2_init(); // Initialize libgit2

    // Check if the given path is a repository
    git_repository *repo = NULL;
    int error = git_repository_open_ext(&repo, target_dir, 0, NULL);
    if (error == 0) {
        printf("The path '%s' is a Git repository.\n", target_dir);

        // Get repository information
        const char *repo_path = git_repository_path(repo);
        printf("Repository root path: %s\n", repo_path);


        // From: https://stackoverflow.com/questions/15717625/how-to-get-the-last-commit-from-head-in-a-git-repository-using-libgit2
        int rc;
        git_commit* commit = NULL;
        git_oid oid_parent_commit; // The SHA1 for tag commit
        const git_signature* signature; // Signature of tag commit
        const char* commit_msg; // Message of tag commit

        //const char* head_str = "HEAD";

        // Resolve tag into a SHA1
        rc = git_reference_name_to_id(&oid_parent_commit, repo, tag_str);
        if (rc != 0) {
            // Try using your HEAD !
            spr_logf_to(logger, SPR_WARN, "Failed git_reference_name_to_id() for tag: {%s}, retrying using HEAD", tag_str);
            memcpy(tag_str, "HEAD", strlen("HEAD"));
            tag_str[strlen("HEAD")] = '\0';
            rc = git_reference_name_to_id(&oid_parent_commit, repo, tag_str);
            if (rc != 0) {
                spr_logf_to(logger, SPR_ERROR, "Failed git_reference_name_to_id() for HEAD");
                git_libgit2_shutdown(); // Shutdown libgit2
                return false;
            };
        }
        if (rc == 0) {
            // Format SHA1 oid
            rc = git_oid_fmt(sha1_oid, &oid_parent_commit);
            if (rc != 0) {
                spr_logf_to(logger, SPR_ERROR, "Failed git_oid_fmt() for tag: {%s}", tag_str);
                git_libgit2_shutdown(); // Shutdown libgit2
                return false;
            } else {
                // The SHA1 string formatted by git_oid_fmt() actually ends with "/tag",
                // So we gave it just enough space to fit the oid.
                // Plus, it seems it may not even be null terminated by itself, so better ensure we do that?
                // https://libgit2.org/libgit2/#HEAD/group/oid/git_oid_fmt
                sha1_oid[40] = '\0';
            }

            // Get the actual commit structure
            rc = git_commit_lookup(&commit, repo, &oid_parent_commit);
            if (rc == 0) {
                // Use the commit, then free it
                signature = git_commit_author(commit);
                spr_logf_to(logger, SPR_INFO, "Tag commit author: {%s}", signature->name);
                commit_msg = git_commit_message(commit);
                spr_logf_to(logger, SPR_INFO, "Tag commit message: {%s}", commit_msg);

                char* commit_msg_stripped = KLS_PUSH_ARR(kls, char, strlen(commit_msg)+1);
                memcpy(commit_msg_stripped, commit_msg, strlen(commit_msg)+1);

                while(commit_msg_stripped[strlen(commit_msg_stripped) -1] == '\n') {
                    //Drop trailing newlines in commit msg
                    commit_msg_stripped[strlen(commit_msg_stripped) -1] = '\0';
                }

                char* commit_msg_escaped = canvil_escape_str_kls(kls, commit_msg_stripped);

                commit_time = git_commit_time(commit);
                spr_logf_to(logger, SPR_INFO, "Tag commit time: {%i}", commit_time);
#else
    struct Canvil_Signature {
        const char* name;
    } sign = {
        .name = "Foo",
    };
    struct Canvil_Signature* signature = &sign;
    char* commit_msg_escaped = "Foo";
    char time_buf[FILENAME_MAX] = {0};
    char* git_cmd_str = "git show -q --clear-decorations --format=\"%%at\" %s";
    if (strlen(tag_str) > (FILENAME_MAX - strlen(git_cmd_str))) {
        spr_logf_to(logger, SPR_ERROR, "Tag name is too long");
        return false;
    }
    sprintf(time_buf, git_cmd_str, tag_str);
#ifndef _WIN32
    FILE *fp = popen(time_buf, "r");
#else
    FILE *fp = _popen(time_buf, "r");
#endif // _WIN32

    int c = fgetc(fp);
    if (c == EOF) {
        spr_logf_to(logger, SPR_ERROR, "Can't get commit time");
#ifndef _WIN32
        pclose(fp);
#else
        _pclose(fp);
#endif // _WIN32
        return false;
    } else {
        for (int pos=0; pos < FILENAME_MAX && c != EOF; pos++) {
            time_buf[pos] = c;
            c = fgetc(fp);
        }
        commit_time = atoi(time_buf);
    }

#ifndef _WIN32
    int status = pclose(fp);
#else
    int status = _pclose(fp);
#endif // _WIN32

#ifndef _WIN32
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        spr_logf_to(logger, SPR_ERROR, "Git command for commit time failed");
        return false;
    }
#else
    if (status != 0) {
        spr_logf_to(logger, SPR_ERROR, "Git command for commit time failed");
        return false;
    }
#endif // _WIN32

    char hash_buf[FILENAME_MAX] = {0};
    git_cmd_str = "git show -q --clear-decorations --format=\"%%H\" %s";
    if (strlen(tag_str) > (FILENAME_MAX - strlen(git_cmd_str))) {
        spr_logf_to(logger, SPR_ERROR, "Tag name is too long");
        return false;
    }
    sprintf(hash_buf, git_cmd_str, tag_str);
#ifndef _WIN32
    fp = popen(hash_buf, "r");
#else
    fp = _popen(hash_buf, "r");
#endif // _WIN32

    c = fgetc(fp);
    if (c == EOF) {
        spr_logf_to(logger, SPR_ERROR, "Can't get commit hash");
#ifndef _WIN32
        pclose(fp);
#else
        _pclose(fp);
#endif // _WIN32
        return false;
    } else {
        for (int pos=0; pos < FILENAME_MAX && c != EOF; pos++) {
            hash_buf[pos] = c;
            c = fgetc(fp);
        }
        strncpy(sha1_oid, hash_buf, sizeof(sha1_oid));
        sha1_oid[40] = '\0';
    }

#ifndef _WIN32
    status = pclose(fp);
#else
    status = _pclose(fp);
#endif // _WIN32

#ifndef _WIN32
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        spr_logf_to(logger, SPR_ERROR, "Git command for commit hash failed");
        return false;
    }
#else
    if (status != 0) {
        spr_logf_to(logger, SPR_ERROR, "Git command for commit hash failed");
        return false;
    }
#endif // _WIN32

    char author_buf[FILENAME_MAX] = {0};
    git_cmd_str = "git show -q --clear-decorations --format=\"%%an\" %s";
    if (strlen(tag_str) > (FILENAME_MAX - strlen(git_cmd_str))) {
        spr_logf_to(logger, SPR_ERROR, "Tag name is too long");
        return false;
    }
    sprintf(author_buf, git_cmd_str, tag_str);
#ifndef _WIN32
    fp = popen(author_buf, "r");
#else
    fp = _popen(author_buf, "r");
#endif // _WIN32

    c = fgetc(fp);
    if (c == EOF) {
        spr_logf_to(logger, SPR_ERROR, "Can't get commit author");
#ifndef _WIN32
        pclose(fp);
#else
        _pclose(fp);
#endif // _WIN32
        return false;
    } else {
        memset(author_buf, 0, sizeof(author_buf));
        for (int pos=0; pos < FILENAME_MAX && c != EOF; pos++) {
            author_buf[pos] = c;
            c = fgetc(fp);
        }
        author_buf[strlen(author_buf)-1] = '\0';
        signature->name = author_buf;
    }

#ifndef _WIN32
    status = pclose(fp);
#else
    status = _pclose(fp);
#endif // _WIN32

#ifndef _WIN32
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        spr_logf_to(logger, SPR_ERROR, "Git command for commit author failed");
        return false;
    }
#else
    if (status != 0) {
        spr_logf_to(logger, SPR_ERROR, "Git command for commit author failed");
        return false;
    }
#endif // _WIN32

    char commit_desc_buf[FILENAME_MAX] = {0};
    git_cmd_str = "git show -q --clear-decorations --format=\"%%s\" %s";
    if (strlen(tag_str) > (FILENAME_MAX - strlen(git_cmd_str))) {
        spr_logf_to(logger, SPR_ERROR, "Tag name is too long");
        return false;
    }
    sprintf(commit_desc_buf, git_cmd_str, tag_str);
#ifndef _WIN32
    fp = popen(commit_desc_buf, "r");
#else
    fp = _popen(commit_desc_buf, "r");
#endif // _WIN32

    c = fgetc(fp);
    if (c == EOF) {
        spr_logf_to(logger, SPR_ERROR, "Can't get commit desc");
#ifndef _WIN32
        pclose(fp);
#else
        _pclose(fp);
#endif // _WIN32
        return false;
    } else {
        memset(commit_desc_buf, 0, sizeof(author_buf));
        for (int pos=0; pos < FILENAME_MAX && c != EOF; pos++) {
            commit_desc_buf[pos] = c;
            c = fgetc(fp);
        }
        commit_desc_buf[strlen(commit_desc_buf)-1] = '\0';
        commit_msg_escaped = canvil_escape_str_kls(kls, commit_desc_buf);
    }

#ifndef _WIN32
    status = pclose(fp);
#else
    status = _pclose(fp);
#endif // _WIN32

#ifndef _WIN32
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        spr_logf_to(logger, SPR_ERROR, "Git command for commit desc failed");
        return false;
    }
#else
    if (status != 0) {
        spr_logf_to(logger, SPR_ERROR, "Git command for commit desc failed");
        return false;
    }
#endif // _WIN32
#endif // CANVIL_NOGIT2

                if (!strcmp(anvil_kern, "amboso-C")) {
                    char header_path[FILENAME_MAX] = {0};
                    sprintf(header_path, "%s/anvil__%s.h", target_dir, bin_name);
                    char cfile_path[FILENAME_MAX] = {0};
                    sprintf(cfile_path, "%s/anvil__%s.c", target_dir, bin_name);

                    spr_logf_to(logger, SPR_INFO, "Header path: {%s}", header_path);
                    spr_logf_to(logger, SPR_INFO, "C file path: {%s}", cfile_path);

                    FILE* header_fp = NULL;
                    header_fp = fopen(header_path, "w");

                    time_t headergen_time = time(NULL);

                    if (header_fp == NULL) {
                        spr_logf_to(logger, SPR_ERROR, "Failed opening header for writing at {%s}", header_path);
                        res = false;
                    } else {
                        fprintf(header_fp,
                                "//Generated by canvil v%s\n//Repo at https://github.com/jgabaut/canvil\n#ifndef ANVIL__%s__\n#define ANVIL__%s__\nstatic const char ANVIL__API_LEVEL__STRING[] = \"%s\"; /**< Represents amboso version used for generated header.*/\nstatic const char ANVIL__%s__VERSION_STRING[] = \"%s\"; /**< Represents current version for generated header.*/\nstatic const char ANVIL__%s__VERSION_DESC[] = \"%s\"; /**< Represents current version info for generated header.*/\nstatic const char ANVIL__%s__VERSION_DATE[] = \"%li\"; /**< Represents date for current version for generated header.*/\nstatic const char ANVIL__%s__VERSION_AUTHOR[] = \"%s\"; /**< Represents author for current version for generated header.*/\nstatic const char ANVIL__%s__HEADER_GENTIME[] = \"%jd\"; /**< Represents gen time for generated header.*/\nconst char *get_ANVIL__API__LEVEL__(void); /**< Returns a version string for amboso API of generated header.*/\nconst char *get_ANVIL__VERSION__(void); /**< Returns a version string for generated header.*/\nconst char *get_ANVIL__VERSION__DESC__(void); /**< Returns a version info string for generated header.*/\nconst char *get_ANVIL__VERSION__DATE__(void); /**< Returns a version date string for generated header.*/\nconst char *get_ANVIL__VERSION__AUTHOR__(void); /**< Returns a version author string for generated header.*/\nconst char *get_ANVIL__HEADER__GENTIME__(void); /**< Returns a string for time of gen for generated header.*/\n#ifndef CANVIL__%s__HEADER__\n#define CANVIL__%s__HEADER__\nstatic const char CANVIL__VERSION__STRING[] = \"%s\"; /**< Represents canvil version used for generated header.*/\nstatic const char CANVIL__OS__STRING[] = \"%s\"; /** Represents os used for generated header.*/\nstatic const char CANVIL__COMMIT__DESC__STRING[] = \"%s\"; /**< Represents message for HEAD commit used for generated header.*/\nconst char *get_CANVIL__API__LEVEL__(void); /**< Returns a version string for canvil version of generated header.*/\nconst char *get_CANVIL__COMMIT__DESC__(void); /**< Returns a string for HEAD commit message used for generated header.*/\n#endif // CANVIL__%s__HEADER__\n#endif\n", CANVIL_API_VERSION_STRING, bin_name, bin_name, EXPECTED_AMBOSO_API_LEVEL, bin_name, tag, bin_name, sha1_oid, bin_name, (long) commit_time, bin_name, signature->name, bin_name, (intmax_t) headergen_time, bin_name, bin_name, CANVIL_API_VERSION_STRING, CANVIL_OS_STRING, commit_msg_escaped, bin_name);

                        fclose(header_fp);
                    }

                    FILE* cfile_fp = NULL;
                    cfile_fp = fopen(cfile_path, "w");

                    if (cfile_fp == NULL) {
                        spr_ltracef(SPR_ERROR, "Failed opening C file for writing at {%s}", cfile_path);
                        res = false;
                    } else {
                        fprintf(cfile_fp,"//Generated by canvil v%s\n#include \"anvil__%s.h\"\n\nconst char *get_ANVIL__VERSION__(void)\n{\n    return ANVIL__%s__VERSION_STRING;\n}\n\nconst char *get_ANVIL__API__LEVEL__(void)\n{\n    return ANVIL__API_LEVEL__STRING;\n}\n\nconst char *get_ANVIL__VERSION__DESC__(void)\n{\n    return ANVIL__%s__VERSION_DESC;\n}\n\nconst char *get_ANVIL__VERSION__DATE__(void)\n{\n    return ANVIL__%s__VERSION_DATE;\n}\n\nconst char *get_ANVIL__VERSION__AUTHOR__(void)\n{\n    return ANVIL__%s__VERSION_AUTHOR;\n}\n\nconst char *get_ANVIL__HEADER__GENTIME__(void)\n{\n    return ANVIL__%s__HEADER_GENTIME;\n}\n\n#ifdef CANVIL__%s__HEADER__\n\nconst char *get_CANVIL__API__LEVEL__(void)\n{\n    return CANVIL__VERSION__STRING;\n}\n\nconst char *get_CANVIL__OS__(void)\n{\n    return CANVIL__OS__STRING;\n}\n\nconst char *get_CANVIL__COMMIT__DESC__(void)\n{\n    return CANVIL__COMMIT__DESC__STRING;\n}\n\n#endif\n", CANVIL_API_VERSION_STRING, bin_name, bin_name, bin_name, bin_name, bin_name, bin_name, bin_name);

                        fclose(cfile_fp);
                    }
                } else if (!strcmp(anvil_kern, "anvilPy")) {
                    char header_path[FILENAME_MAX] = {0};
                    sprintf(header_path, "%s/anvil__%s.py", target_dir, bin_name);
                    spr_logf_to(logger, SPR_INFO, "Header path: {%s}", header_path);

                    FILE* header_fp = NULL;
                    header_fp = fopen(header_path, "w");

                    time_t headergen_time = time(NULL);

                    if (header_fp == NULL) {
                        spr_logf_to(logger, SPR_ERROR, "Failed opening header for writing at {%s}", header_path);
                        res = false;
                    } else {
                        fprintf(header_fp, "# Generated by canvil v%s\n", CANVIL_API_VERSION_STRING);
                        fprintf(header_fp, "# Repo at https://github.com/jgabaut/amboso\n\n");
	                    fprintf(header_fp, "ANVIL__API_LEVEL__STRING = \"%s\" # amboso version\n", EXPECTED_AMBOSO_API_LEVEL);
	                    fprintf(header_fp, "ANVIL__%s__VERSION_STRING = \"%s\" # current version\n", bin_name, tag);
	                    fprintf(header_fp, "ANVIL__%s__VERSION_DESC = \"%s\" # version description\n", bin_name, sha1_oid);
	                    fprintf(header_fp, "ANVIL__%s__VERSION_DATE = \"%li\" # version date\n", bin_name, (long)commit_time);
	                    fprintf(header_fp, "ANVIL__%s__VERSION_AUTHOR = \"%s\" # author\n", bin_name, signature->name);
#ifndef _WIN32
	                    fprintf(header_fp, "ANVIL__%s__HEADER_GENTIME = \"%li\" # header generation time\n\n\n", bin_name, (intmax_t) headergen_time);
#else
	                    fprintf(header_fp, "ANVIL__%s__HEADER_GENTIME = \"%lli\" # header generation time\n\n\n", bin_name, (intmax_t) headergen_time);
#endif // _WIN32
                        fprintf(header_fp, "def get_ANVIL_API_LEVEL__() -> str:\n    return ANVIL__API_LEVEL_STRING\n\n");
                        fprintf(header_fp, "def get_ANVIL__VERSION__() -> str:\n    return ANVIL__%s__VERSION_STRING\n\n", bin_name);
                        fprintf(header_fp, "def get_ANVIL__VERSION__DESC__() -> str:\n    return ANVIL__%s__VERSION_DESC\n\n", bin_name);
                        fprintf(header_fp, "def get_ANVIL__VERSION__DATE__() -> str:\n    return ANVIL__%s__VERSION_DATE\n\n", bin_name);
                        fprintf(header_fp, "def get_ANVIL__VERSION__AUTHOR__() -> str:\n    return ANVIL__%s__VERSION_AUTHOR\n\n", bin_name);
                        fprintf(header_fp, "def get_ANVIL__HEADER_GENTIME__() -> str:\n    return ANVIL__%s__HEADER_GENTIME\n", bin_name);

                        //fprintf(header_fp,
                                //CANVIL_API_VERSION_STRING, bin_name, bin_name, EXPECTED_AMBOSO_API_LEVEL, bin_name, tag, bin_name, sha1_oid, bin_name, (long) commit_time, bin_name, signature->name, bin_name, (intmax_t) headergen_time, bin_name, bin_name, CANVIL_API_VERSION_STRING, CANVIL_OS_STRING, commit_msg_escaped, bin_name);

                        fclose(header_fp);
                    }

                } else if (!strcmp(anvil_kern, "custom")) {
                    spr_logf_to(logger, SPR_INFO, "TODO: implement header gen for custom kern");
                    res = false;
                } else {
                    spr_logf_to(logger, SPR_ERROR, "Unexpected anvil_kern: {%s}", anvil_kern);
                    res = false;
                }
#ifndef CANVIL_NOGIT2
                git_commit_free(commit);
            }
        } else {
            spr_logf_to(logger, SPR_ERROR, "Failed git_reference_name_to_id() for {%s}", tag_str);
            res = false;
        }
        // Free the repository object
        git_repository_free(repo);
    } else {
        printf("The path '%s' is not a Git repository or an error occurred.\n", target_dir);
        res = false;
    }

    git_libgit2_shutdown(); // Shutdown libgit2
#endif // CANVIL_NOGIT2

    return res;
}

void to_uppercase_copy(const char *src, char *dest, size_t dest_size) {
    size_t i;
    for (i = 0; i < dest_size - 1 && src[i] != '\0'; i++) {
        dest[i] = toupper((unsigned char)src[i]);
    }
    dest[i] = '\0';
}

void dashes_to_underscore_copy(const char *src, char *dest, size_t dest_size) {
    size_t i;
    for (i = 0; i < dest_size - 1 && src[i] != '\0'; i++) {
        if (src[i] == '-') dest[i] = '_';
        else dest[i] = src[i];
    }
    dest[i] = '\0';
}

bool canvil_init_project(const char* target_name, const char* anvil_kern, const char* template_name, Spuro logger, Koliseo* kls) {

    if (!strcmp(anvil_kern, "custom")) {
        if (!template_name) {
            spr_logf_to(logger, SPR_ERROR, "Template name was NULL");
            return false;
        }
        const char* home = getenv("HOME");
        char templates_dir_path[FILENAME_MAX] = {0};
        sprintf(templates_dir_path, "%s/.anvil/templates", home);
        struct dirent *entry;
        DIR *dir = opendir(templates_dir_path);
        if (dir == NULL) {
            spr_logf_to(logger, SPR_ERROR, "Failed opening tests dir {%s}", templates_dir_path);
        } else {
            Koliseo* tmp_kls = kls_new(KLS_DEFAULT_SIZE);
            bool matched = false;
            while ((entry = readdir(dir)) != NULL) {
                // Skip . and ..
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                    continue;

                kls_clear(tmp_kls);

                char* curr_path = KLS_PUSH_ARR(tmp_kls, char, strlen(home) +1 + strlen(".anvil/templates") +1 + strlen(entry->d_name) +1); // +1 for / and /0
                sprintf(curr_path, "%s/.anvil/templates/%s", home, entry->d_name);

                struct stat st;

                if (stat(curr_path, &st) == -1) {
                    perror("stat");
                    spr_logf_to(logger, SPR_ERROR, "Failed stat of test {%s}", curr_path);
                    continue;
                }

                if (S_ISDIR(st.st_mode)) {
                    spr_logf_to(logger, SPR_INFO, "Found template: {%s}", entry->d_name);
                    if (!strcmp(entry->d_name, template_name)) {
                        matched = true;
                        break;
                    }
                } else if (S_ISREG(st.st_mode)) {
                    spr_logf_to(logger, SPR_TRACE, "Found file: {%s}", entry->d_name);
                    continue;
                }
            }
            closedir(dir);
            kls_free(tmp_kls);
            if (!matched) {
                spr_logf_to(logger, SPR_ERROR, "Could not find a matching template for {%s}", template_name);
                return false;
            }
        }
    }

#ifndef CANVIL_NOGIT2
    git_libgit2_init();

    git_repository *repo = NULL;
    int error = git_repository_init(&repo, target_name, false); // false = not bare

    if (error < 0) {
        const git_error *e = git_error_last();
        spr_logf_to(logger, SPR_ERROR, "Error %d: %s", error, e && e->message ? e->message : "Unknown error");
    } else {
        spr_logf_to(logger, SPR_DEBUG, "Repository initialized successfully!");
    }

    if (!strcmp(anvil_kern, "amboso-C")) {
        git_submodule *submodule = NULL;
        const char *submodule_url = "https://github.com/jgabaut/amboso";
        const char *submodule_path = "amboso";

        // Setup the submodule (writes .gitmodules and config entry)
        error = git_submodule_add_setup(&submodule, repo, submodule_url, submodule_path, 1);
        if (error < 0) {
            spr_logf_to(logger, SPR_ERROR, "Failed setup of submodule");
            git_submodule_free(submodule);
            git_repository_free(repo);
            git_libgit2_shutdown();
            return false;
        }

        git_repository *subrepo = NULL;
        // Clone the submodule repo into the submodule path
        error = git_submodule_clone(&subrepo, submodule, NULL);
        if (error < 0) {
            spr_logf_to(logger, SPR_ERROR, "Failed clone of submodule");
            git_submodule_free(submodule);
            git_repository_free(subrepo);
            git_repository_free(repo);
            git_libgit2_shutdown();
            return false;
        }

        // Mark submodule as initialized and written to config
        error = git_submodule_init(submodule, 1); // writes to .git/config
        if (error < 0) {
            spr_logf_to(logger, SPR_ERROR, "Failed init of submodule");
            git_submodule_free(submodule);
            git_repository_free(subrepo);
            git_repository_free(repo);
            git_libgit2_shutdown();
            return false;
        }

        error = git_submodule_add_finalize(submodule);
        if (error < 0) {
            spr_logf_to(logger, SPR_ERROR, "Failed finalize of submodule");
            git_submodule_free(submodule);
            git_repository_free(subrepo);
            git_repository_free(repo);
            git_libgit2_shutdown();
            return false;
        }

        git_submodule_free(submodule);
        git_repository_free(subrepo);
    }
    git_repository_free(repo);
    git_libgit2_shutdown();
#else
    const char* cmd_args[4] = {
        [0] = "git",
        [1] = "init",
        [2] = target_name,
        [3] = NULL,
    };
    Koliseo_Temp* kls_t = kls_temp_start(kls);
    Komando cmd = new_command_kls_t(3, cmd_args, kls_t);
    bool run_res = run_command(cmd);
    kls_temp_end(kls_t);

    if (!run_res) {
        spr_logf_to(logger, SPR_ERROR, "Failed git init %s", target_name);
        return false;
    }

    const char* cd_args[3] = {
        [0] = "cd",
        [1] = target_name,
        [2] = NULL,
    };
    kls_t = kls_temp_start(kls);
    cmd = new_command_kls_t(3, cd_args, kls_t);
    run_res = run_command(cmd);
    kls_temp_end(kls_t);

    if (!run_res) {
        spr_logf_to(logger, SPR_ERROR, "Failed cd %s", target_name);
        return false;
    }

    const char* submod_args[5] = {
        [0] = "git",
        [1] = "submodule",
        [2] = "add",
        [3] = "https://github.com/jgabaut/amboso.git",
        [4] = NULL,
    };

    kls_t = kls_temp_start(kls);
    cmd = new_command_kls_t(3, submod_args, kls_t);
    run_res = run_command(cmd);
    kls_temp_end(kls_t);

    if (!run_res) {
        spr_logf_to(logger, SPR_ERROR, "Failed git submodule add https://github.com/jgabaut/amboso.git");
    }

    const char* cd_back_args[3] = {
        [0] = "cd",
        [1] = "-",
        [2] = NULL,
    };
    kls_t = kls_temp_start(kls);
    cmd = new_command_kls_t(2, cd_back_args, kls_t);
    run_res = run_command(cmd);
    kls_temp_end(kls_t);

    if (!run_res) {
        spr_logf_to(logger, SPR_ERROR, "Failed cd -");
        return false;
    }
#endif // CANVIL_NOGIT2

    char target_name_nodashes[FILENAME_MAX] = {0};
    dashes_to_underscore_copy(target_name, target_name_nodashes, sizeof(target_name_nodashes));

    if (!strcmp(anvil_kern, "amboso-C")) {
        char target[FILENAME_MAX] = {0};
        char linkpath[FILENAME_MAX] = {0};

        sprintf(target, "%s/amboso/amboso", target_name);
        sprintf(linkpath, "%s/anvil", target_name);

#ifndef _WIN32
        int result = symlink(target, linkpath);
        if (result != 0) {
            spr_logf_to(logger, SPR_ERROR, "Symlink failed");
            return false;
        }
#endif // _WIN32

        char target_name_srcdir[FILENAME_MAX] = {0};
#ifndef _WIN32
        sprintf(target_name_srcdir, "%s/%s", target_name, "src");
#else
        sprintf(target_name_srcdir, "%s\\%s", target_name, "src");
#endif

        bool src_dir_check = canvil_check_dir_create(target_name_srcdir);
        if (!src_dir_check) {
            spr_logf_to(logger, SPR_ERROR, "Could not create src dir");
            return false;
        }
    } else if (!strcmp(anvil_kern, "anvilPy")) {
        char target_name_srcdir[FILENAME_MAX] = {0};
#ifndef _WIN32
        sprintf(target_name_srcdir, "%s/%s", target_name, target_name_nodashes);
#else
        sprintf(target_name_srcdir, "%s\\%s", target_name, target_name_nodashes);
#endif
        bool src_dir_check = canvil_check_dir_create(target_name_srcdir);
        if (!src_dir_check) {
            spr_logf_to(logger, SPR_ERROR, "Could not create %s dir", target_name);
            return false;
        }
    } else if (!strcmp(anvil_kern, "custom")) {
        const char* home = getenv("HOME");
        char source_path[FILENAME_MAX] = {0};
        sprintf(source_path, "%s/.anvil/templates/%s", home, template_name);
        int res = copy_directory(source_path, target_name);
        return (res == 0);
    }
    char target_name_kazojdir[FILENAME_MAX] = {0};
#ifndef _WIN32
    sprintf(target_name_kazojdir, "%s/%s", target_name, "tests");
#else
    sprintf(target_name_kazojdir, "%s\\%s", target_name, "tests");
#endif

    bool kazoj_dir_check = canvil_check_dir_create(target_name_kazojdir);
    if (!kazoj_dir_check) {
        spr_logf_to(logger, SPR_ERROR, "Could not create kazoj dir");
        return false;
    }

    char target_name_testsdir[FILENAME_MAX] = {0};
#ifndef _WIN32
    sprintf(target_name_testsdir, "%s/tests/%s", target_name, "ok");
#else
    sprintf(target_name_testsdir, "%s\\tests\\%s", target_name, "ok");
#endif

    bool tests_dir_check = canvil_check_dir_create(target_name_testsdir);
    if (!tests_dir_check) {
        spr_logf_to(logger, SPR_ERROR, "Could not create tests dir");
        return false;
    }

    char target_name_errortestsdir[FILENAME_MAX] = {0};
#ifndef _WIN32
    sprintf(target_name_errortestsdir, "%s/tests/%s", target_name, "errors");
#else
    sprintf(target_name_errortestsdir, "%s\\tests\\%s", target_name, "errors");
#endif

    bool errortests_dir_check = canvil_check_dir_create(target_name_errortestsdir);
    if (!errortests_dir_check) {
        spr_logf_to(logger, SPR_ERROR, "Could not create errortests dir");
        return false;
    }

    if (!strcmp(anvil_kern, "amboso-C")) {
        char target_stego_lock[FILENAME_MAX] = {0};
#ifndef _WIN32
        sprintf(target_stego_lock, "%s/stego.lock", target_name);
#else
        sprintf(target_stego_lock, "%s\\stego.lock", target_name);
#endif

        FILE* fp = fopen(target_stego_lock, "w");

        if (!fp) {
            spr_logf_to(logger, SPR_ERROR, "Failed opening {%s}", target_stego_lock);
            return false;
        }

        fprintf(fp, "[build]\nsource = \"main.c\"\nbin = \"%s\"\nmakevers = \"0.1.0\"\nautomakevers = \"0.1.0\"\ntests = \"tests\"\n[tests]\ntestsdir = \"ok\"\nerrortestsdir = \"errors\"\n[versions]\n\"0.1.0\" = \"%s\"\n", target_name_nodashes, target_name_nodashes);
        fclose(fp);

        char target_gitignore[FILENAME_MAX] = {0};
#ifndef _WIN32
        sprintf(target_gitignore, "%s/.gitignore", target_name);
#else
        sprintf(target_gitignore, "%s\\.gitignore", target_name);
#endif

        fp = fopen(target_gitignore, "w");

        if (!fp) {
            spr_logf_to(logger, SPR_ERROR, "Failed opening {%s}", target_gitignore);
            return false;
        }

        fprintf(fp, "#Generated by canvil v%s\n# ignore object files\n*.o\n# also explicitly ignore our executable for good measure\n%s\n# also explicitly ignore our windows executable for good measure\n%s.exe\n# also explicitly ignore our debug executable for good measure\n%s_debug\n#We also want to ignore the dotfile dump if we ever use anvil with -c flag\namboso_cfg.dot\n# MacOS DS_Store ignoring\n.DS_Store\n# ignore debug log file\ndebug_log.txt\n# ignore files generated by Autotools\nautom4te.cache/\ncompile\nconfig.guess\nconfig.log\nconfig.status\nconfig.sub\nconfigure\ninstall-sh\nmissing\naclocal.m4\nconfigure~\nMakefile\nMakefile.in\n# ignore amboso log file\nanvil.log\n#ignore invil log file\ninvil.log\n", CANVIL_API_VERSION_STRING, target_name_nodashes, target_name_nodashes, target_name_nodashes);

        fclose(fp);

        char target_main[FILENAME_MAX] = {0};
#ifndef _WIN32
        sprintf(target_main, "%s/src/main.c", target_name);
#else
        sprintf(target_main, "%s\\src\\main.c", target_name);
#endif

        fp = fopen(target_main, "w");

        if (!fp) {
            spr_logf_to(logger, SPR_ERROR, "Failed opening {%s}", target_main);
            return false;
        }

        fprintf(fp, "#include <stdio.h>\nint main(void) {\nprintf(\"Hello, World!\");\nreturn 0;\n}\n");
        fclose(fp);

        char target_name_upper[FILENAME_MAX] = {0};

        to_uppercase_copy(target_name_nodashes, target_name_upper, sizeof(target_name_upper));

        char target_configure_ac[FILENAME_MAX] = {0};
#ifndef _WIN32
        sprintf(target_configure_ac, "%s/configure.ac", target_name);
#else
        sprintf(target_configure_ac, "%s\\configure.ac", target_name);
#endif

        fp = fopen(target_configure_ac, "w");

        if (!fp) {
            spr_logf_to(logger, SPR_ERROR, "Failed opening {%s}", target_configure_ac);
            return false;
        }

        fprintf(fp, "Generated by canvil v%s\nAC_INIT([%s], [0.1.0], [email@example.com])\nAM_INIT_AUTOMAKE([foreign -Wall])\nAC_CANONICAL_HOST\nbuild_linux=no\nbuild_windows=no\nbuild_mac=no\necho \"Host os:  $host_os\"\n\nAC_ARG_ENABLE([debug],  [AS_HELP_STRING([--enable-debug], [Enable debug build])],  [enable_debug=$enableval],  [enable_debug=no])\nAM_CONDITIONAL([DEBUG_BUILD], [test \"$enable_debug\" = \"yes\"])\ncase \"${host_os}\" in\n\tmingw*)\n\t\techo \"Building for mingw32: [$host_cpu-$host_vendor-$host_os]\"\n\t\tbuild_windows=yes\n\t\tAC_SUBST([%s_CFLAGS], [\"-I/usr/x86_64-w64-mingw32/include -static -fstack-protector\"])\n\t\tAC_SUBST([%s_LDFLAGS], [\"-L/usr/x86_64-w64-mingw32/lib\"])\n\t\tAC_SUBST([CCOMP], [\"/usr/bin/x86_64-w64-mingw32-gcc\"])\n\t\tAC_SUBST([OS], [\"w64-mingw32\"])\n\t\tAC_SUBST([TARGET], [\"%s.exe\"])\n\t;;\n\tdarwin*)\n\t\tbuild_mac=yes\n\t\techo \"Building for macos: [$host_cpu-$host_vendor-$host_os]\"\n\t\tAC_SUBST([%s_CFLAGS], [\"-I/opt/homebrew/opt/ncurses/include\"])\n\t\tAC_SUBST([%s_LDFLAGS], [\"-L/opt/homebrew/opt/ncurses/lib\"])\n\t\tAC_SUBST([OS], [\"darwin\"])\n\t\tAC_SUBST([TARGET], [\"%s\"])\n\t;;\n\tlinux*)\n\t\techo \"Building for Linux: [$host_cpu-$host_vendor-$host_os]\"\n\t\tbuild_linux=yes\n\t\tAC_SUBST([%s_CFLAGS], [\"\"])\n\t\tAC_SUBST([%s_LDFLAGS], [\"\"])\n\t\tAC_SUBST([OS], [\"Linux\"])\n\t\tAC_SUBST([TARGET], [\"%s\"])\n\t;;\nesac\n\nAM_CONDITIONAL([DARWIN_BUILD], [test \"$build_mac\" = \"yes\"])\nAM_CONDITIONAL([WINDOWS_BUILD], [test \"$build_windows\" = \"yes\"])\nAM_CONDITIONAL([LINUX_BUILD], [test \"$build_linux\" = \"yes\"])\n\nAC_ARG_VAR([VERSION], [Version number])\nif test -z \"$VERSION\"; then\n  VERSION=\"0.1.0\"\nfi\nAC_DEFINE_UNQUOTED([VERSION], [\"$VERSION\"], [Version number])\nAC_CHECK_PROGS([CCOMP], [gcc clang])\nAC_CHECK_HEADERS([stdio.h])\nAC_CHECK_FUNCS([malloc calloc])\nAC_CONFIG_FILES([Makefile])\nAC_OUTPUT\n", CANVIL_API_VERSION_STRING, target_name_nodashes, target_name_upper, target_name_upper, target_name_nodashes, target_name_upper, target_name_upper, target_name_nodashes, target_name_upper, target_name_upper, target_name_nodashes);

        fclose(fp);

        char target_makefile_am[FILENAME_MAX] = {0};
#ifndef _WIN32
        sprintf(target_makefile_am, "%s/Makefile.am", target_name);
#else
        sprintf(target_makefile_am, "%s\\Makefile.am", target_name);
#endif

        fp = fopen(target_makefile_am, "w");

        if (!fp) {
            spr_logf_to(logger, SPR_ERROR, "Failed opening {%s}", target_makefile_am);
            return false;
        }

        fprintf(fp, "#Generated by canvil v%s\nAUTOMAKE_OPTIONS = foreign\nCFLAGS = @CFLAGS@\nSHELL := /bin/bash\n.ONESHELL:\nMACHINE := $$(uname -m)\nPACK_NAME = $(TARGET)-$(VERSION)-$(OS)-$(MACHINE)\n%s_SOURCES = src/main.c\nLDADD = $(%s_LDFLAGS)\nAM_LDFLAGS = -O2\nAM_CFLAGS = $(%s_CFLAGS) -O2 -Werror -Wpedantic -Wall\nif DEBUG_BUILD\nAM_LDFLAGS += -ggdb -O0\nAM_CFLAGS += \nelse\nAM_LDFLAGS += -s\nendif\n\n%%.o: %%.c\n\t$(CCOMP) -c $(CFLAGS) $(AM_CFLAGS) $< -o $@\n\n$(TARGET): $(%s_SOURCES:.c=.o)\n\t@echo -e \"    CFLAGS: [ $(CFLAGS) ]\"\n\t@echo -e \"    AM_CFLAGS: [ $(AM_CFLAGS) ]\"\n\t@echo -e \"    LDADD: [ $(LDADD) ]\"\n\t$(CCOMP) $(CFLAGS) $(AM_CFLAGS) $(%s_SOURCES:.c=.o) -o $@ $(LDADD) $(AM_LDFLAGS)\n\nclean:\n\t@echo -en \"Cleaning build artifacts:  \"\n\t-rm $(TARGET)\n\t-rm src/*.o\n\t-rm static/*.o\n\t@echo -e \"Done.\"\n\ncleanob:\n\t@echo -en \"Cleaning object build artifacts:  \"\n\t-rm src/*.o\n\t-rm static/*.o\n\t@echo -e \"Done.\"\n\nanviltest:\n\t@echo -en \\\"Running anvil tests.\"\n\t./anvil -tX\n\t@echo -e \"Done.\"\n\nall: $(TARGET)\nrebuild: clean all\n.DEFAULT_GOAL := all\n", CANVIL_API_VERSION_STRING, target_name_nodashes, target_name_upper, target_name_upper, target_name_nodashes, target_name_nodashes);

        fclose(fp);

    } else if (!strcmp(anvil_kern, "anvilPy")) {
        char target_stego_lock[FILENAME_MAX] = {0};
#ifndef _WIN32
        sprintf(target_stego_lock, "%s/stego.lock", target_name);
#else
        sprintf(target_stego_lock, "%s\\stego.lock", target_name);
#endif

        FILE* fp = fopen(target_stego_lock, "w");

        if (!fp) {
            spr_logf_to(logger, SPR_ERROR, "Failed opening {%s}", target_stego_lock);
            return false;
        }

        fprintf(fp, "[anvil]\nkern = \"anvilPy\"\nversion = \"%s\"\n[build]\nsource = \"main.py\"\nbin = \"%s\"\nmakevers = \"0.1.0\"\nautomakevers = \"0.1.0\"\ntests = \"tests\"\n[tests]\ntestsdir = \"ok\"\nerrortestsdir = \"errors\"\n[versions]\n\"0.1.0\" = \"%s\"\n", EXPECTED_AMBOSO_API_LEVEL, target_name_nodashes, target_name_nodashes);
        fclose(fp);

        char target_main_py[FILENAME_MAX] = {0};
#ifndef _WIN32
        sprintf(target_main_py, "%s/%s/main.py", target_name, target_name_nodashes);
#else
        sprintf(target_main_py, "%s\\%s\\main.py", target_name, target_name_nodashes);
#endif
        fp = fopen(target_main_py, "w");

        if (!fp) {
            spr_logf_to(logger, SPR_ERROR, "Failed opening {%s}", target_main_py);
            return false;
        }

        fprintf(fp, "#!/bin/python3\ndef main():\n    print(\"Hello, World!\");\n");
        fclose(fp);

        char target_gitignore[FILENAME_MAX] = {0};
#ifndef _WIN32
        sprintf(target_gitignore, "%s/.gitignore", target_name);
#else
        sprintf(target_gitignore, "%s\\.gitignore", target_name);
#endif

        fp = fopen(target_gitignore, "w");

        if (!fp) {
            spr_logf_to(logger, SPR_ERROR, "Failed opening {%s}", target_gitignore);
            return false;
        }

        fprintf(fp, "#Generated by amboso v%s\n# ignore dist dir\ndist\n# ignore __pycache__\n__pycache__\n# ignore build dir\nbuild\n# ignore egg info dir\n%s.egg-info\n",
                EXPECTED_AMBOSO_API_LEVEL, target_name_nodashes);
        fclose(fp);

        char target_pyproj_toml[FILENAME_MAX] = {0};
#ifndef _WIN32
        sprintf(target_pyproj_toml, "%s/pyproject.toml", target_name);
#else
        sprintf(target_pyproj_toml, "%s\\pyproject.toml", target_name);
#endif
        fp = fopen(target_pyproj_toml, "w");

        fprintf(fp, "[project]\nname = \"%s\"\nversion = \"0.1.0\"\n[project.scripts]\n%s = \"%s.main:main\"\n[build-system]\nrequires = [\"setuptools>=61.0\"]\nbuild-backend = \"setuptools.build_meta\"",
                target_name_nodashes, target_name_nodashes, target_name_nodashes);
        fclose(fp);
    }

    return false;
}
