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
#include "canvil_op.h"

static bool canvil_check_tagpath(const char* targetdir_optarg, const char* tagname, Koliseo_Temp* t_kls)
{
    assert(t_kls != NULL);
    size_t target_tag_path_len = (FILENAME_MAX * 2) + 2 + 1 + 1; // PATH/vPATH/\0

    char* target_tag_path = KLS_PUSH_ARR_T(t_kls, char, target_tag_path_len);

#ifndef _WIN32
    snprintf(target_tag_path, target_tag_path_len, "%s/v%s/", targetdir_optarg, tagname);
#else
    snprintf(target_tag_path, "%s\\v%s\\", targetdir_optarg, tagname);
#endif // _WIN32

    return canvil_check_dir(target_tag_path);
}

static bool canvil_check_tagpath_create(const char* targetdir_optarg, const char* tagname, Koliseo_Temp* t_kls)
{
    assert(t_kls != NULL);
    size_t target_tag_path_len = (FILENAME_MAX * 2) + 2 + 1 + 1; // PATH/vPATH/\0

    char* target_tag_path = KLS_PUSH_ARR_T(t_kls, char, target_tag_path_len);

#ifndef _WIN32
    snprintf(target_tag_path, target_tag_path_len, "%s/v%s/", targetdir_optarg, tagname);
#else
    snprintf(target_tag_path, "%s\\v%s\\", targetdir_optarg, tagname);
#endif // _WIN32

    return canvil_check_dir_create(target_tag_path);
}

static char* canvil_fmt_targetpath(const char* targetdir_optarg, const char* tagname, const char* bin_optarg, Koliseo_Temp* t_kls)
{
    assert(t_kls != NULL);
    size_t target_file_path_len = (FILENAME_MAX * 3) + 2 + 1 + 1; // PATH/vPATH/PATH\0

    char* target_file_path = KLS_PUSH_ARR_T(t_kls, char, target_file_path_len);

#ifndef _WIN32
    snprintf(target_file_path, target_file_path_len, "%s/v%s/%s", targetdir_optarg, tagname, bin_optarg);
#else
    snprintf(target_file_path, "%s\\v%s\\%s", targetdir_optarg, tagname, bin_optarg);
#endif // _WIN32

    return target_file_path;
}

#ifndef _WIN32

#ifndef CANVIL_NOGIT2
int canvil_submodule_callback(git_submodule *submodule, const char *name, void *payload) {
    git_repository *repo = (git_repository *)payload;
    (void)repo;
    //printf("Initializing submodule: %s\n", name);

    if (git_submodule_update(submodule, 1, NULL) != 0) {
        // If submodule update fails, print an error but do not stop the process.
        // const git_error *err = git_error_last();
        // fprintf(stderr, "Error: Failed to update submodule '%s'. Error: %s\n", name, err ? err->message : "Unknown error");
        // Continue to the next submodule instead of failing.
        return 0;
    }

    return 0;
}

/**
 * Checkout a specific commit and return the previous HEAD reference.
 * If an error occurs, returns NULL.
 */
static inline git_reference *canvil_checkout_tag(git_repository *repo, const char *tag_name) {
    git_object *commit = NULL;
    git_reference *prev_head = NULL;
    git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;

    // Store previous HEAD
    if (git_repository_head(&prev_head, repo) != 0) {
        prev_head = NULL;
        printf("Warning: Unable to determine previous HEAD.\n");
    }

    // Directly resolve the tag name to a commit (handles lightweight tags)
    int error = git_revparse_single(&commit, repo, tag_name);
    if (error != 0) {
        const git_error *e = git_error_last();
        fprintf(stderr, "Error: git_revparse_single failed for '%s': %s\n", tag_name, e ? e->message : "Unknown error");
        return NULL;
    }

    // Checkout the commit
    if (git_checkout_tree(repo, commit, &checkout_opts) != 0) {
        fprintf(stderr, "Error: Failed to checkout tree for tag '%s'.\n", tag_name);
        git_object_free(commit);
        return NULL;
    }

    // Set HEAD to detached mode
    if (git_repository_set_head_detached(repo, git_object_id(commit)) != 0) {
        fprintf(stderr, "Failed to set HEAD to commit.\n");
        git_object_free(commit);
        return NULL;
    }

    // Initialize and update submodules
    if (git_submodule_foreach(repo, canvil_submodule_callback, repo) != 0) {
        fprintf(stderr, "Warning: Some submodules failed to initialize.\n");
    }

    git_object_free(commit);
    printf("Checked out tag '%s'\n", tag_name);
    return prev_head;
}

static inline bool canvil_restore_previous_branch(git_repository *repo, git_reference *prev_head) {
    if (!prev_head) {
        printf("No previous HEAD to switch back to.\n");
        return false;
    }

    const git_oid *prev_commit_id = git_reference_target(prev_head);
    if (!prev_commit_id) {
        fprintf(stderr, "Error: Failed to get previous commit ID.\n");
        return false;
    }

    // Checkout the previous commit
    if (git_repository_set_head(repo, git_reference_name(prev_head)) != 0) {
        fprintf(stderr, "Error: Failed to switch back to previous HEAD.\n");
        return false;
    }

    git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;
    checkout_opts.checkout_strategy = GIT_CHECKOUT_FORCE;  // Forces overwrite of local changes
    git_object *prev_commit = NULL;

    // Ensure working tree is cleaned before checkout
    if (git_object_lookup(&prev_commit, repo, prev_commit_id, GIT_OBJECT_COMMIT) != 0) {
        fprintf(stderr, "Error: Could not lookup previous commit.\n");
        return false;
    }

    if (git_checkout_tree(repo, prev_commit, &checkout_opts) != 0) {
        fprintf(stderr, "Error: Failed to checkout previous tree.\n");
        git_object_free(prev_commit);
        return false;
    }

    // Free previous commit object
    git_object_free(prev_commit);

    // Ensure submodules are initialized and updated properly
    if (git_submodule_foreach(repo, canvil_submodule_callback, repo) != 0) {
        fprintf(stderr, "Warning: Some submodules failed to reinitialize.\n");
    }

    printf("Switched back to previous commit.\n");
    return true;
}
#else
static inline bool canvil_checkout_tag(const char *tag_name) {
    const char* cmd_args[4] = {
        [0] = "git",
        [1] = "checkout",
        [2] = tag_name,
        [3] = NULL,
    };
    Koliseo* k = kls_new(KLS_DEFAULT_SIZE);
    Koliseo_Temp* kls_t = kls_temp_start(k);
    Komando cmd = new_command_kls_t(3, cmd_args, kls_t);
    bool run_res = run_command(cmd);
    kls_free(k);
    return run_res;
}
static inline bool canvil_restore_previous_branch(void) {
    const char* cmd_args[2] = {
        [0] = "git switch -",
        [1] = NULL,
    };
    Koliseo* k = kls_new(KLS_DEFAULT_SIZE);
    Koliseo_Temp* kls_t = kls_temp_start(k);
    Komando cmd = new_shell_command_kls_t(1, cmd_args, kls_t);
    bool run_res = run_command(cmd);
    kls_free(k);
    return run_res;
}
#endif // CANVIL_NOGIT2
#endif // _WIN32

int canvil_handle_singlefile_build(const char* targetdir_optarg, const char* builds_dir_optarg, const char* bin_optarg, const char* source_optarg, const char* tag, const char* cflags_optarg, Spuro logger, Koliseo_Temp* kls_t)
{

    char cmd_str[FILENAME_MAX*8] = {0};
    if (cflags_optarg) {
        sprintf(cmd_str, "cc %s/v%s/%s -o %s/v%s/%s -lm %s", targetdir_optarg, tag, source_optarg, targetdir_optarg, tag, bin_optarg, cflags_optarg);
    } else {
        sprintf(cmd_str, "cc %s/v%s/%s -o %s/v%s/%s -lm", targetdir_optarg, tag, source_optarg, targetdir_optarg, tag, bin_optarg);
    }
    const char* cmd_args[2] = {
        [0] = cmd_str,
        [1] = NULL,
    };
    Komando cmd = new_shell_command_kls_t(1, cmd_args, kls_t);
    bool run_res = run_command(cmd);
    if (!run_res) {
        spr_clogf_to(logger, SPR_RED, SPR_ERROR, "%s", "Failed running single file build cmd");
        return run_res;
    }
    return 0;
}

int canvil_handle_make_call(bool use_autoconf, bool no_rebuild, bool use_configure_arg, const char* configure_arg, char** extra_args, size_t extra_args_len, Spuro logger, Koliseo_Temp* kls_t)
{
    bool need_autoreconf = true;
#ifndef _WIN32
    const char* makefile_path = "./Makefile";
#else
    const char* makefile_path = ".\\Makefile";
#endif
    if (canvil_filepath_exists(makefile_path) || !use_autoconf) {
        need_autoreconf = false;
    }
#ifndef _WIN32
    const char* configure_ac_path = "./configure.ac";
#else
    const char* configure_ac_path = ".\\configure.ac";
#endif
#ifndef _WIN32
    const char* makefile_am_path = "./Makefile.am";
#else
    const char* makefile_am_path = ".\\Makefile.am";
#endif
    if (need_autoreconf && ! (canvil_filepath_exists(configure_ac_path) && canvil_filepath_exists(makefile_am_path))) {
        spr_clogf_to(logger, SPR_RED, SPR_ERROR, "Can't find both {%s} and {%s}", configure_ac_path, makefile_am_path);
        return 1;
    }

    if (need_autoreconf) {

        const char* autoreconf_prep_cmd_str = "aclocal; autoconf; automake --add-missing; ./configure";
        char autoreconf_prep_actual_str[1024] = {0};
        if (use_configure_arg) {
            spr_clogf_to(logger, SPR_CYAN, SPR_INFO, "Using configure argument {%s}", configure_arg);
            sprintf(autoreconf_prep_actual_str, "%s %s", autoreconf_prep_cmd_str, configure_arg);
        } else {
            sprintf(autoreconf_prep_actual_str, "%s", autoreconf_prep_cmd_str);
        }
        const char* autoreconf_prep_cmd_args[2] = {
            [0] = autoreconf_prep_actual_str,
            [1] = NULL,
        };
        Komando autoreconf_prep_cmd = new_shell_command_kls_t(1, autoreconf_prep_cmd_args, kls_t);
        bool run_res = run_command(autoreconf_prep_cmd);
        if (!run_res) {
            spr_clogf_to(logger, SPR_RED, SPR_ERROR, "%s", "Failed running autoreconf prep cmd");
            return run_res;
        }
        spr_clogf_to(logger, SPR_GREEN, SPR_ERROR, "%s", "Success running autoreconf prep cmd");

        if (!canvil_filepath_exists(makefile_path)) {
            spr_clogf_to(logger, SPR_RED, SPR_ERROR, "Failed autotool prep for Makefile at {%s}", makefile_path);
            return 1;
        }
    }

    char* actual_cmd = "make";

    const char** make_cmd_args = KLS_PUSH_ARR_T(kls_t, const char*, 3+extra_args_len);
    if (extra_args_len > 0) {
        spr_logf_to(logger, SPR_INFO, "Extra args:");
        for (size_t i=0; i < extra_args_len; i++) {
            spr_logf_to(logger, SPR_INFO, "{%s}", extra_args[i]);
        }
    }
    make_cmd_args[0] = actual_cmd;
    for (size_t i=1; i<=extra_args_len; i++) make_cmd_args[i] = extra_args[i-1];
    if (no_rebuild) {
        make_cmd_args[extra_args_len+1] = NULL;
    } else {
        make_cmd_args[extra_args_len+1] = "rebuild";
        make_cmd_args[extra_args_len+2] = NULL;
    }

    spr_logf_to(logger, SPR_DEBUG, "Running:");
    for (size_t i=0; i < 3+extra_args_len; i++) {
        spr_logf_to(logger, SPR_DEBUG, "%s", make_cmd_args[i]);
    }

    Komando make_cmd = new_shell_command_kls_t(1, make_cmd_args, kls_t);
    bool run_res = run_command(make_cmd);
    return (run_res ? 0 : 1);
}

bool canvil_op_delete(const char* targetdir_optarg, const char* tagname, const char* bin_optarg, Spuro logger, Koliseo* kls)
{
    assert(kls != NULL);

    Koliseo_Temp* k_tmp_check = kls_temp_start(kls);

    if (!canvil_check_tagpath(targetdir_optarg, tagname, k_tmp_check)) {
        spr_logf_to(logger, SPR_ERROR, "Could not find tag dir {%s/v%s}", targetdir_optarg, tagname);
        kls_temp_end(k_tmp_check);
        return false;
    }
    kls_temp_end(k_tmp_check);

    Koliseo_Temp* k_tmp = kls_temp_start(kls);

    char* target_file_path = canvil_fmt_targetpath(targetdir_optarg, tagname, bin_optarg, k_tmp);

    bool res = false;
    if (canvil_filepath_exists(target_file_path)) {
        int remove_res = remove(target_file_path);

        if (remove_res == 0) {
            spr_logf_to(logger, SPR_INFO, "Deleted {%s}", target_file_path);
            res = true;
        } else {
            spr_logf_to(logger, SPR_INFO, "Failed delete() for {%s}", target_file_path);
        }
    } else {
        spr_logf_to(logger, SPR_ERROR, "Could not find {%s}", target_file_path);
    }

    kls_temp_end(k_tmp);
    return res;
}

bool canvil_op_build(bool git_mode, bool force, bool no_rebuild, bool use_config_arg, const char* config_optarg, const char* minmake_optarg, const char* minautomake_version, const char* cflags_optarg, const char* targetdir_optarg, const char* builds_dir_optarg, const char* tagname, const char* bin_optarg, const char* source_optarg, const char* kern, AnvilPy_Env anvilpy_env, const char* custom_builder, char** extra_args, size_t extra_args_len, Spuro logger, Koliseo* kls)
{
    assert(kls != NULL);

    Koliseo_Temp* k_tmp_check = kls_temp_start(kls);

    if (!canvil_check_tagpath_create(targetdir_optarg, tagname, k_tmp_check)) {
        spr_logf_to(logger, SPR_ERROR, "Could not create tag dir {%s/v%s}", targetdir_optarg, tagname);
        kls_temp_end(k_tmp_check);
        return false;
    }
    kls_temp_end(k_tmp_check);

    Koliseo_Temp* k_tmp = kls_temp_start(kls);

    char* target_file_path = canvil_fmt_targetpath(targetdir_optarg, tagname, bin_optarg, k_tmp);

    if (!strcmp(kern, "anvilPy")) {
        char *build_system_backend = anvilpy_env.build_system.backend;
        spr_logf_to(logger, SPR_INFO, "%s", build_system_backend);
        if (!strcmp(build_system_backend, "setuptools.build_meta")) {
            // ok
        } else {
            spr_logf_to(logger, SPR_ERROR, "Unexpected build system: {%s}", build_system_backend);
            return false;
        }
    }

    bool res = true;
    if (!canvil_filepath_exists(target_file_path) || force) {

        if (force) {
            spr_logf_to(logger, SPR_INFO, "Forcing build of {%s}", tagname);
        }
        spr_logf_to(logger, SPR_INFO, "Building {%s}", target_file_path);

#ifndef CANVIL_NOGIT2
        git_repository *repo = NULL;
        git_reference *previous_head = NULL;
#endif // CANVIL_NOGIT2
        if (git_mode) {
            spr_logf_to(logger, SPR_INFO, "Switching to {%s}", tagname);
#ifndef CANVIL_NOGIT2
            git_libgit2_init(); // Initialize libgit2

            const char* repo_path = ".";
            int error = git_repository_open_ext(&repo, repo_path, 0, NULL);
            if (error != 0) {
                spr_logf_to(logger, SPR_ERROR, "Failed to open repository at '%s'.\n", repo_path);
                git_libgit2_shutdown(); // Shutdown libgit2
                return res;
            }
            repo_path = git_repository_path(repo);
            spr_tlogf_to(logger, SPR_INFO, "Repository root path: %s", repo_path);

            previous_head = canvil_checkout_tag(repo, tagname);
            if (!previous_head) {
                spr_logf_to(logger, SPR_ERROR, "Failed checkout of {%s}", tagname);
                kls_temp_end(k_tmp);
                git_repository_free(repo);
                git_libgit2_shutdown(); // Shutdown libgit2
                return res;
            }
#else
            if (!canvil_checkout_tag(tagname)) {
                return false;
            }
#endif // CANVIL_NOGIT2
        }

        int make_res = -1;
        if (!strcmp(kern, "amboso-C")) {
            SemVer target = {0};
            SemVer minmake = {0};
            SemVer minautomake = {0};
            parseSemVer(tagname, &(target.major), &(target.minor), &(target.patch));
            parseSemVer(minmake_optarg, &(minmake.major), &(minmake.minor), &(minmake.patch));
            parseSemVer(minautomake_version, &(minautomake.major), &(minautomake.minor), &(minautomake.patch));
            if (canvil_SemVer_cmp(target, minmake) >= 0) {
                bool use_autoconf = false;
                if (canvil_SemVer_cmp(target, minautomake) >= 0) {
                    use_autoconf = true;
                }
                make_res = canvil_handle_make_call(use_autoconf, no_rebuild, use_config_arg, config_optarg, extra_args, extra_args_len, logger, k_tmp);
            } else {
                make_res = canvil_handle_singlefile_build(targetdir_optarg, builds_dir_optarg, bin_optarg, source_optarg, tagname, cflags_optarg, logger, k_tmp);
            }
        } else if (!strcmp(kern, "anvilPy")) {
            make_res = canvil_py_handle_build(logger, builds_dir_optarg, k_tmp);
        } else if (!strcmp(kern, "custom")) {
            if (custom_builder != NULL) {
                make_res = canvil_custom_handle_build(custom_builder, targetdir_optarg, builds_dir_optarg, bin_optarg, tagname, extra_args, extra_args_len, logger, k_tmp);
            } else {
                spr_logf_to(logger, SPR_ERROR, "Missing custombuilder definition");
            }
        }
        if (make_res == 0) {
            int rename_res = -1;
            if (!strcmp(kern, "amboso-C") || !strcmp(kern, "custom")) {
                SemVer target = {0};
                SemVer minmake = {0};
                parseSemVer(minmake_optarg, &(minmake.major), &(minmake.minor), &(minmake.patch));
                parseSemVer(tagname, &(target.major), &(target.minor), &(target.patch));
                if (canvil_SemVer_cmp(target, minmake) >= 0) {
                    char build_path[FILENAME_MAX] = {0};
#ifndef _WIN32
                    sprintf(build_path, "%s/%s", builds_dir_optarg, bin_optarg);
#else
                    sprintf(build_path, "%s\\%s", builds_dir_optarg, bin_optarg);
#endif
                    rename_res = rename(build_path, target_file_path);
                    if (rename_res != 0) {
                        spr_logf_to(logger, SPR_ERROR, "Failed mv {%s} -> {%s}", build_path, target_file_path);
                        res = false;
                    }
                }
            } else if (!strcmp(kern, "anvilPy")) {
                res = canvil_py_handle_postbuild(anvilpy_env, targetdir_optarg, tagname, logger);
            }
        } else {
            spr_logf_to(logger, SPR_ERROR, "Build command result: {%i}", make_res);
        }


        if (git_mode) spr_logf_to(logger, SPR_INFO, "Switching back");
#ifndef CANVIL_NOGIT2
        if (git_mode && !canvil_restore_previous_branch(repo, previous_head)) {
            spr_logf_to(logger, SPR_ERROR, "Failed switchback from {%s}", tagname);
            kls_temp_end(k_tmp);
            git_repository_free(repo);
            git_libgit2_shutdown(); // Shutdown libgit2
            return res;
        }
        if (git_mode) git_repository_free(repo);
#else
        if(git_mode && !canvil_restore_previous_branch()) {
            spr_logf_to(logger, SPR_ERROR, "Failed switchback from {%s}", tagname);
            kls_temp_end(k_tmp);
            return res;
        }
#endif // CANVIL_NOGIT2
    } else {
        spr_logf_to(logger, SPR_ERROR, "{%s} exists already.", target_file_path);
    }

    kls_temp_end(k_tmp);
#ifndef CANVIL_NOGIT2
    if (git_mode) git_libgit2_shutdown(); // Shutdown libgit2
#endif // CANVIL_NOGIT2
    return res;
}

bool canvil_op_purge(const char* targetdir_optarg, Canvil_Tag_List tag_list, const char* bin_optarg, Spuro logger, Koliseo* kls)
{
    assert(kls != NULL);
    spr_logf_to(logger, SPR_INFO, "Doing purge for {%s}, bin is {%s}", targetdir_optarg, bin_optarg);
    bool res = true;
    while (! Canvil_Tag_List_isEmpty(tag_list)) {
        Canvil_Tag* node_pt = Canvil_Tag_List_head(tag_list);
        SemVer node_value = *(node_pt->version);
        char tag_repr[200] = {0};
        snprintf(tag_repr, 200, SemVer_Fmt, SemVer_Arg(node_value));
        tag_repr[199] = '\0';
        bool delete_res = canvil_op_delete(targetdir_optarg, tag_repr, bin_optarg, logger, kls);
        if (delete_res) {
            spr_logtf_to(logger, SPR_INFO, "Success deleting {%s/v%s/%s}", targetdir_optarg, tag_repr, bin_optarg);
        } else {
            spr_logtf_to(logger, SPR_ERROR, "Failed deleting {%s/v%s/%s}", targetdir_optarg, tag_repr, bin_optarg);
            res = false; // Reporting any number of failures as false
                         // TODO: better handling of cases
        }
        tag_list = Canvil_Tag_List_tail(tag_list);
    }
    return res;
}

bool canvil_op_init(bool git_mode, bool force, bool no_rebuild, bool use_config_arg, const char* config_optarg, const char* minmake_optarg, const char* minautomake_version, const char* cflags_optarg, const char* targetdir_optarg, Canvil_Tag_List tag_list, const char* builds_dir_optarg, const char* bin_optarg, const char* source_optarg, const char* kern, AnvilPy_Env anvilpy_env, const char* custom_builder, char** extra_args, size_t extra_args_len, Spuro logger, Koliseo* kls)
{
    assert(kls != NULL);
    spr_logf_to(logger, SPR_INFO, "Doing init for {%s}, bin is {%s}", targetdir_optarg, bin_optarg);
    bool res = true;
    int successes = 0;
    size_t list_len = Canvil_Tag_List_length(tag_list);
    while (! Canvil_Tag_List_isEmpty(tag_list)) {
        Canvil_Tag* node_pt = Canvil_Tag_List_head(tag_list);
        SemVer node_value = *(node_pt->version);
        char tag_repr[200] = {0};
        snprintf(tag_repr, 200, SemVer_Fmt, SemVer_Arg(node_value));
        tag_repr[199] = '\0';
        bool build_res = canvil_op_build(git_mode, force, no_rebuild, use_config_arg, config_optarg, minmake_optarg, minautomake_version, cflags_optarg, targetdir_optarg, builds_dir_optarg, tag_repr, bin_optarg, source_optarg, kern, anvilpy_env, custom_builder, extra_args, extra_args_len, logger, kls);
        if (build_res) {
            spr_logtf_to(logger, SPR_INFO, "Success building {%s/v%s/%s}", targetdir_optarg, tag_repr, bin_optarg);
            successes++;
        } else {
            spr_logtf_to(logger, SPR_ERROR, "Failed building {%s/v%s/%s}", targetdir_optarg, tag_repr, bin_optarg);
            res = false; // Reporting any number of failures as false
                         // TODO: better handling of cases
        }
        tag_list = Canvil_Tag_List_tail(tag_list);
    }
    SpuroLevel lvl = SPR_INFO;
    if (successes < list_len) lvl = SPR_WARN;
    spr_logf_to(logger, lvl, "Successes: {%i/%zu}", successes, list_len);
    return res;
}

bool canvil_op_run(const char* targetdir_optarg, const char* tagname, const char* bin_optarg, Spuro logger, Koliseo* kls)
{
    char target[FILENAME_MAX] = {0};
    sprintf(target, "%s/v%s/%s", targetdir_optarg, tagname, bin_optarg);

    spr_logtf_to(logger, SPR_INFO, "Running {%s}", target);

    int res = execlp(target, target, (char*) NULL);
    if (res != 0) return false;
    return true;
}

bool canvil_op_test(Anvil_Args* canvil_args, Canvil_Test query, Spuro logger, Koliseo* kls)
{
    char test_name[FILENAME_MAX+1] = {0};
    sprintf(test_name, "./%s/%s/%s", canvil_args->tests_dir, query.dir, query.name);

    spr_logf_to(logger, SPR_INFO, "Running test: {%s}", test_name);

    char stdout_name[FILENAME_MAX+1] = {0};
    sprintf(stdout_name, "%s%s", test_name, ".stdout");

    char stderr_name[FILENAME_MAX+1] = {0};
    sprintf(stderr_name, "%s%s", test_name, ".stderr");

    spr_logtf_to(logger, SPR_DEBUG, "Checking test stdout: {%s}", stdout_name);
    spr_logtf_to(logger, SPR_DEBUG, "Checking test stderr: {%s}", stderr_name);

    bool matched = false;
    const char* args[] = {
        test_name,
        NULL,
    };
    Koliseo_Temp* kls_t = kls_temp_start(kls);
    Komando kmd = new_command_kls_t(1, args, kls_t);
    bool res = run_command_checked(kmd, &matched, (canvil_args->do_build == 1), stdout_name, stderr_name);
    kls_temp_end(kls_t);
    if (!res) spr_logtf_to(logger, SPR_DEBUG, "Failure on run_command_checked()");
    return (matched);
}
int canvil_op_test_macro(Anvil_Args* canvil_args, Canvil_Test_List test_list, Canvil_Test_List errortest_list, Spuro logger, Koliseo* kls)
{
    int result = 0;

    int tests_ran = 0;

    while (! Canvil_Test_List_isEmpty(test_list)) {
        Canvil_Test* node_pt = Canvil_Test_List_head(test_list);

        bool test_res = canvil_op_test(canvil_args, *node_pt, logger, kls);

        if (!test_res) result++;

        tests_ran++;
        test_list = Canvil_Test_List_tail(test_list);
    }

    while (! Canvil_Test_List_isEmpty(errortest_list)) {
        Canvil_Test* node_pt = Canvil_Test_List_head(errortest_list);

        bool test_res = canvil_op_test(canvil_args, *node_pt, logger, kls);

        if (!test_res) result++;

        tests_ran++;
        errortest_list = Canvil_Test_List_tail(errortest_list);
    }

    spr_tclogf_to(logger, SPR_GREEN, SPR_INFO, "Successes: {%i}", tests_ran - result);
    spr_tclogf_to(logger, SPR_RED, SPR_INFO, "Failures: {%i}", result);

    return result;
}
