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
#include "canvil_toml.h"
#include <dirent.h>

bool try_check_key_strvalue(const char* key, toml_datum_t table, const char* table_name, toml_datum_t* buf)
{
     //Remember to free *buf.u.s after this returns, if it is accessed
     toml_datum_t value = toml_get(table, key);
     if (value.type != TOML_STRING) {
         spr_ltracef(SPR_ERROR, "Can't get string for {%s} key {%s}", table_name, key);
         return false;
     }

     /*
     char* upper_key_buf = calloc(strlen(key)+1, sizeof(char));
     //TODO: check mem fail?
     for(size_t i = 0; i < strlen(key); i++) {
         upper_key_buf[i] = toupper(key[i]);
     }
     upper_key_buf[strlen(key)] = '\0';

     char* upper_table_buf = calloc(strlen(table_name)+1, sizeof(char));
     //TODO: check mem fail?
     for(size_t i = 0; i < strlen(table_name); i++) {
         upper_table_buf[i] = toupper(table_name[i]);
     }
     upper_table_buf[strlen(table_name)] = '\0';

     spr_ltracef(SPR_INFO, "%s_%s: {%s}", upper_table_buf, upper_key_buf, value.u.s);
     free(upper_key_buf);
     free(upper_table_buf);
     */

     *buf = value;
     //Remember to free *buf.u.s after this returns, if it is accessed
     return true;
}

void check_table_innerkey(const char* target_key, toml_datum_t table, const char* current_key, const char* table_name, Spuro logger)
{
    toml_datum_t value = {0};
    if (strcmp(current_key, target_key) == 0) {
        if (!try_check_key_strvalue(current_key, table, table_name, &value)) {
            spr_logf_to(logger, SPR_ERROR, "Bad try for {%s}", current_key);
        } else {
            const char* val = value.u.s;
            spr_logf_to(logger, SPR_TRACE, "Found {%s} {%s} value: {%s}", table_name, current_key, val);
            //free(val);
        }
    }
}

const char* get_table_stringval(toml_datum_t table, const char* key, const char* table_name, Spuro logger)
{
    toml_datum_t value = {0};
    const char* res = NULL;
    if (!try_check_key_strvalue(key, table, table_name, &value)) {
        spr_logf_to(logger, SPR_ERROR, "Bad try for {%s}", key);
    } else {
        const char* val = value.u.s;
        spr_logf_to(logger, SPR_TRACE, "Found {%s} {%s} value: {%s}", table_name, key, val);
        res = val;
    }
    return res;
}

bool check_filepath_as_stego(const char* filepath, Spuro logger)
{
    toml_result_t result = toml_parse_file_ex(filepath);

    if (!result.ok) {
      fprintf(stderr, "%s():    %s\n", __func__, result.errmsg);
      return false;
    }

    toml_datum_t tab = result.toptab;
    int tab_len = tab.u.tab.size;
    for (int i = 0; i < tab_len ; i++) {
        const char* key = tab.u.tab.key[i];
        spr_logf_to(logger, SPR_DEBUG, "key %d: %s", i, key);
        bool is_anvil_table = (strcmp(key,"anvil") == 0);
        bool is_build_table = (strcmp(key,"build") == 0);
        bool is_tests_table = (strcmp(key,"tests") == 0);
        bool is_versions_table = (strcmp(key,"versions") == 0);

        toml_datum_t inner_table = toml_get(tab, key);

        if (inner_table.type != TOML_TABLE) {
            //TODO: custom errors for specific erroring inner tables
            spr_logf_to(logger, SPR_ERROR, "can't get inner table for key {%s}", key);
            toml_free(result);
            return false;
        }
        int inner_tab_len = inner_table.u.tab.size;
        for (int j = 0; j < inner_tab_len; j++) {
            const char* inner_key = inner_table.u.tab.key[j];
            spr_logf_to(logger, SPR_DEBUG, "inner_key %d: %s", j, inner_key);
            if (is_anvil_table) {
                check_table_innerkey("version", inner_table, inner_key, "anvil", logger);
                check_table_innerkey("kern", inner_table, inner_key, "anvil", logger);
            }
            if (is_build_table) {
                check_table_innerkey("source", inner_table, inner_key, "build", logger);
                check_table_innerkey("bin", inner_table, inner_key, "build", logger);
                check_table_innerkey("makevers", inner_table, inner_key, "build", logger);
                check_table_innerkey("tests", inner_table, inner_key, "build", logger);
                check_table_innerkey("automakevers", inner_table, inner_key, "build", logger);
                check_table_innerkey("dir", inner_table, inner_key, "build", logger);
            }
            if (is_tests_table) {
                check_table_innerkey("testsdir", inner_table, inner_key, "tests", logger);
                check_table_innerkey("errortestsdir", inner_table, inner_key, "tests", logger);
            }
            if (is_versions_table) {
                bool is_valid_semver = false;
                bool is_base_tag = false;
                char* tag = NULL;
                if (inner_key[0] == 'B') {
                    // This is a base mode tag. Check it without the B
                    is_base_tag = true;
                    tag = (char*) &(inner_key[1]);
                } else {
                    tag = (char*) inner_key;
                }
                is_valid_semver = validateSemVer(tag);
                if (is_valid_semver) {
                    spr_logf_to(logger, SPR_DEBUG, "Found%sversion: {%s}", (is_base_tag ? " base " : " "), tag);
                } else {
                    spr_logf_to(logger, SPR_ERROR, "Failed validation for version: {%s}", tag);
                }
            }
        }
    }

    toml_free(result);
    return true;
}

bool getargs_from_filepath_as_stego(Anvil_Args* canvil_args, const char* filepath, Koliseo* kls, bool collect_anvil_env, Anvil_Env* canvil_env, Spuro logger)
{
    if (collect_anvil_env && canvil_env == NULL) {
        spr_tracef("Passed canvil_env was NULL.\n");
        return false;
    }

    toml_result_t result = toml_parse_file_ex(filepath);

    if (!result.ok) {
        fprintf(stderr, "%s():    %s\n", __func__, result.errmsg);
        return false;
    }

    if (collect_anvil_env) {
        canvil_env->base_tags = Canvil_Tag_List_nullList();
        canvil_env->git_tags = Canvil_Tag_List_nullList();
        canvil_env->tests = Canvil_Test_List_nullList();
        canvil_env->errortests = Canvil_Test_List_nullList();
    }

    toml_datum_t tab = result.toptab;
    int tab_len = tab.u.tab.size;

    for (int i = 0; i < tab_len; i++) {
        const char* key = tab.u.tab.key[i];
        spr_logf_to(logger, SPR_DEBUG, "key %d: %s", i, key);
        bool is_anvil_table = (strcmp(key,"anvil") == 0);
        bool is_build_table = (strcmp(key,"build") == 0);
        bool is_tests_table = (strcmp(key,"tests") == 0);
        bool is_versions_table = (strcmp(key,"versions") == 0);

        toml_datum_t inner_table = toml_get(tab, key);

        if (inner_table.type != TOML_TABLE) {
            //TODO: custom errors for specific erroring inner tables
            spr_logf_to(logger, SPR_ERROR, "can't get inner table for key {%s}", key);
            toml_free(result);
            return false;
        }
        int inner_tab_len = inner_table.u.tab.size;
        for (int j = 0; j < inner_tab_len; j++) {
            const char* inner_key = inner_table.u.tab.key[j];
            spr_logf_to(logger, SPR_DEBUG, "inner_key %d: %s", j, inner_key);
            if (is_anvil_table) {
                bool is_versions_key = (strcmp(inner_key, "version") == 0);
                bool is_kern_key = (strcmp(inner_key, "kern") == 0);
                bool is_custombuilder_key = (strcmp(inner_key, "custombuilder") == 0);
                bool is_recipe_key = (strcmp(inner_key, "recipe") == 0);
                if (is_versions_key) {
                    const char* stego_val = get_table_stringval(inner_table, inner_key, "anvil", logger);
                    int major, minor, patch;
                    if(!parseSemVer(stego_val, &major, &minor, &patch)) {
                        spr_logf_to(logger, SPR_ERROR, "Passed anvil_version is not a valid SemVer: {%s}", stego_val);
                        toml_free(result);
                        return false;
                    }
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
                        spr_logf_to(logger, SPR_ERROR, "Stego-defined anvil_version {%s} is not a supported anvil version", stego_val);
                        toml_free(result);
                        return false;
                    } else {
                        if (canvil_SemVer_cmp(target, MIN_AMBOSO_V_EXTENSIONS) < 0) {
                            spr_logf_to(logger, SPR_DEBUG, "Turning off extensions");
                            canvil_args->strict = 1;
                        }
                    }

                    spr_logf_to(logger, SPR_INFO, "Using anvil version from stego.lock: {%s}", stego_val);
                    canvil_args->anvil_version_optarg = KLS_PUSH_ARR(kls, char, strlen(stego_val)+1);
                    memcpy(canvil_args->anvil_version_optarg, stego_val, strlen(stego_val)+1);
                    canvil_args->passed_anvil_version = 1;
                }
                if (is_kern_key) {
                    const char* stego_val = get_table_stringval(inner_table, inner_key, "anvil", logger);
                    bool matched = false;
                    if (!strcmp(stego_val, "amboso-C")) {
                        matched = true;
                    }
                    if (!strcmp(stego_val, "anvilPy")) {
                        SemVer target = {0};
                        if (!parseSemVer(canvil_args->anvil_version_optarg, &(target.major), &(target.minor), &(target.patch))) {
                            spr_logf_to(logger, SPR_ERROR, "Failed parsing current anvil_version_optarg");
                            toml_free(result);
                            return false;
                        }
                        if (canvil_SemVer_cmp(target, MIN_AMBOSO_V_ANVILPY_KERN) >= 0) {
                            matched = true;
                        }
                    }
                    if (!strcmp(stego_val, "custom")) {
                        SemVer target = {0};
                        if (!parseSemVer(canvil_args->anvil_version_optarg, &(target.major), &(target.minor), &(target.patch))) {
                            spr_logf_to(logger, SPR_ERROR, "Failed parsing current anvil_version_optarg");
                            toml_free(result);
                            return false;
                        }
                        if (canvil_SemVer_cmp(target, MIN_AMBOSO_V_ANVILPY_KERN) >= 0) {
                            matched = true;
                        }
                    }
                    if (matched) {
                        spr_logf_to(logger, SPR_INFO, "Using anvil kern from stego.lock: {%s}", stego_val);
                        canvil_args->anvil_kern_optarg = KLS_PUSH_ARR(kls, char, strlen(stego_val)+1);
                        memcpy(canvil_args->anvil_kern_optarg, stego_val, strlen(stego_val)+1);
                        canvil_args->passed_anvil_kern = 1;
                    } else {
                        spr_logf_to(logger, SPR_ERROR, "Unsupported anvil_kern: {%s}", stego_val);
                        toml_free(result);
                        return false;
                    }
                }
                if (is_custombuilder_key) {
                    const char* stego_val = get_table_stringval(inner_table, inner_key, "anvil", logger);
                    if (!validateCustomBuilder(stego_val)) {
                        spr_logf_to(logger, SPR_ERROR, "Invalid custom builder: {%s}", stego_val);
                        toml_free(result);
                        return false;
                    }
                    spr_logf_to(logger, SPR_DEBUG, "Found custombuilder {%s}", stego_val);
                    canvil_args->anvil_custombuilder = KLS_PUSH_ARR(kls, char, strlen(stego_val)+1);
                    memcpy(canvil_args->anvil_custombuilder, stego_val, strlen(stego_val)+1);
                }
                if (is_recipe_key) {
                    toml_datum_t inner_array = toml_get(inner_table, inner_key);
                    if (inner_array.type != TOML_ARRAY) {
                        spr_logf_to(logger, SPR_ERROR, "can't get array for inner key {%s}", inner_key);
                        toml_free(result);
                        return false;
                    }
                    int array_size = inner_array.u.arr.size;
                    canvil_env->recipes = da_recipes_init(kls);
                    for (int k = 0; k < array_size ; k++) {
                        toml_datum_t recipe_table_val = inner_array.u.arr.elem[k];
                        if (recipe_table_val.type != TOML_TABLE) continue;
                        int recipe_tab_len = recipe_table_val.u.tab.size;
                        da_recipes_push(canvil_env->recipes, KLS_PUSH(kls, Anvil_Recipe));
                        for (int j = 0; j < recipe_tab_len; j++) {
                            const char* recipe_key = recipe_table_val.u.tab.key[j];
                            spr_logf_to(logger, SPR_DEBUG, "recipe_key %d: %s", j, recipe_key);
                            bool is_recipe_build_key = (strcmp(recipe_key, "build") == 0);
                            bool is_recipe_conf_key = (strcmp(recipe_key, "conf") == 0);
                            bool is_recipe_vers_key = (strcmp(recipe_key, "vers") == 0);
                            if (is_recipe_build_key) {
                                const char* stego_val = get_table_stringval(recipe_table_val, recipe_key, "anvil_recipe", logger);
                                canvil_env->recipes->items[k]->build = KLS_PUSH_ARR(kls, char, strlen(stego_val)+1);

                                memcpy(canvil_env->recipes->items[k]->build, stego_val, strlen(stego_val)+1);
                            }
                            if (is_recipe_conf_key) {
                                const char* stego_val = get_table_stringval(recipe_table_val, recipe_key, "anvil_recipe", logger);
                                canvil_env->recipes->items[k]->conf = KLS_PUSH_ARR(kls, char, strlen(stego_val)+1);

                                memcpy(canvil_env->recipes->items[k]->conf, stego_val, strlen(stego_val)+1);
                            }
                            if (is_recipe_vers_key) {
                                SemVer smv = {0};
                                const char* stego_val = get_table_stringval(recipe_table_val, recipe_key, "anvil_recipe", logger);
                                if (!parseSemVer(stego_val, &(smv.major), &(smv.minor), &(smv.patch))) {
                                    spr_logf_to(logger, SPR_ERROR, "Failed parsing SemVer: {%s}", stego_val);
                                } else {
                                    spr_logf_to(logger, SPR_DEBUG, "Semver: {" SemVer_Fmt "}", SemVer_Arg(smv));
                                    canvil_env->recipes->items[k]->vers = KLS_PUSH(kls, SemVer);
                                    *(canvil_env->recipes->items[k]->vers) = smv;
                                }
                            }
                            spr_logf_to(logger, SPR_DEBUG, "Found anvil_recipe[%i]", k);
                        }
                    }
                }
            }
            if (is_build_table) {
                bool is_source_key = (strcmp(inner_key, "source") == 0);
                bool is_bin_key = (strcmp(inner_key, "bin") == 0);
                bool is_makevers_key = (strcmp(inner_key, "makevers") == 0);
                bool is_tests_key = (strcmp(inner_key, "tests") == 0);
                bool is_automakevers_key = (strcmp(inner_key, "automakevers") == 0);
                bool is_builds_dir_key = (strcmp(inner_key, "dir") == 0);
                if (is_source_key) {
                    const char* stego_val = get_table_stringval(inner_table, inner_key, "build", logger);
                    if (canvil_args->source_optarg == NULL) {
                        spr_logf_to(logger, SPR_INFO, "Using source name from stego.lock: {%s}", stego_val);
                        canvil_args->source_optarg = KLS_PUSH_ARR(kls, char, strlen(stego_val)+1);
                        memcpy(canvil_args->source_optarg, stego_val, strlen(stego_val)+1);
                    }
                }
                if (is_bin_key) {
                    const char* stego_val = get_table_stringval(inner_table, inner_key, "build", logger);
                    if (canvil_args->bin_optarg == NULL) {
                        spr_logf_to(logger, SPR_INFO, "Using bin name from stego.lock: {%s}", stego_val);
                        canvil_args->bin_optarg = KLS_PUSH_ARR(kls, char, strlen(stego_val)+1);
                        memcpy(canvil_args->bin_optarg, stego_val, strlen(stego_val)+1);
                    }
                }
                if (is_makevers_key) {
                    const char* stego_val = get_table_stringval(inner_table, inner_key, "build", logger);
                    if (canvil_args->minmake_optarg == NULL) {
                        spr_logf_to(logger, SPR_INFO, "Using min make tag from stego.lock: {%s}", stego_val);
                        canvil_args->minmake_optarg = KLS_PUSH_ARR(kls, char, strlen(stego_val)+1);
                        memcpy(canvil_args->minmake_optarg, stego_val, strlen(stego_val)+1);
                    }
                }
                if (is_tests_key) {
                    const char* stego_val = get_table_stringval(inner_table, inner_key, "build", logger);
                    canvil_args->tests_dir = KLS_PUSH_ARR(kls, char, strlen(stego_val)+1);
                    memcpy(canvil_args->tests_dir, stego_val, strlen(stego_val)+1);
                }
                if (is_automakevers_key) {
                    const char* stego_val = get_table_stringval(inner_table, inner_key, "build", logger);
                    canvil_args->minautomake_version = KLS_PUSH_ARR(kls, char, strlen(stego_val)+1);
                    memcpy(canvil_args->minautomake_version, stego_val, strlen(stego_val)+1);
                }
                if (is_builds_dir_key) {
                    const char* stego_val = get_table_stringval(inner_table, inner_key, "build", logger);
                    spr_logf_to(logger, SPR_INFO, "Using builds_dir from stego.lock: {%s}", stego_val);
                    canvil_args->builds_dir_optarg = KLS_PUSH_ARR(kls, char, strlen(stego_val)+1);
                    memcpy(canvil_args->builds_dir_optarg, stego_val, strlen(stego_val)+1);
                }
            }
            if (is_tests_table) {
                bool is_testsdir_key = (strcmp(inner_key, "testsdir") == 0);
                bool is_errortestsdir_key = (strcmp(inner_key, "errortestsdir") == 0);
                if (is_testsdir_key) {
                    const char* stego_val = get_table_stringval(inner_table, inner_key, "tests", logger);
                    spr_logf_to(logger, SPR_DEBUG, "Found tests dir: {%s}", stego_val);

                    Koliseo* tmp_kls = kls_new(FILENAME_MAX+1);
                    char* testsdir_path = KLS_PUSH_ARR(tmp_kls, char, strlen(canvil_args->tests_dir) + 1 + strlen(stego_val) +1); // +1 for / and /0
                    sprintf(testsdir_path, "%s/%s", canvil_args->tests_dir, stego_val);

                    spr_logf_to(logger, SPR_DEBUG, "Looking into tests dir {%s}", testsdir_path);

                    struct dirent *entry;
                    DIR *dir = opendir(testsdir_path);
                    if (dir == NULL) {
                        spr_logf_to(logger, SPR_ERROR, "Failed opening tests dir {%s}", testsdir_path);
                    } else {
                        while ((entry = readdir(dir)) != NULL) {

                            // Skip . and ..
                            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                                continue;

                            kls_clear(tmp_kls);

                            char* curr_path = KLS_PUSH_ARR(tmp_kls, char, strlen(canvil_args->tests_dir) +1 + strlen(stego_val) +1 + strlen(entry->d_name) +1); // +1 for / and /0
                            sprintf(curr_path, "%s/%s/%s", canvil_args->tests_dir, stego_val, entry->d_name);

                            struct stat st;

                            if (stat(curr_path, &st) == -1) {
                                perror("stat");
                                spr_logf_to(logger, SPR_ERROR, "Failed stat of test {%s}", curr_path);
                                continue;
                            }

                            if (S_ISDIR(st.st_mode)) {
                                spr_logf_to(logger, SPR_TRACE, "Found subdirectory: {%s}", entry->d_name);
                            } else if (S_ISREG(st.st_mode)) {
                                spr_logf_to(logger, SPR_TRACE, "Found file: {%s}", entry->d_name);
                                int is_exec = st.st_mode & S_IXUSR;
                                if (is_exec) {
                                    spr_logf_to(logger, SPR_TRACE, "Found executable file");
                                    int name_len = strlen(entry->d_name);
                                    if (name_len >= 2) {
                                        if (entry->d_name[name_len-1] == 'k' && entry->d_name[name_len-2] == '.') {
                                            Canvil_Test* canvil_test = KLS_PUSH(kls, Canvil_Test);
                                            canvil_test->dir = KLS_PUSH_STR(kls, stego_val);
                                            sprintf((char*)canvil_test->dir, "%s", stego_val);
                                            canvil_test->name = KLS_PUSH_STR(kls, entry->d_name);
                                            sprintf((char*)canvil_test->name, "%s", entry->d_name);
                                            spr_logf_to(logger, SPR_DEBUG, "Found test: {%s/%s}", canvil_test->dir, canvil_test->name);
                                            canvil_env->tests = Canvil_Test_List_cons_kls(kls, canvil_test, canvil_env->tests);
                                        }
                                    }
                                }
                            }
                        }
                        closedir(dir);
                    }
                    kls_free(tmp_kls);
                }
                if (is_errortestsdir_key) {
                    const char* stego_val = get_table_stringval(inner_table, inner_key, "tests", logger);
                    spr_logf_to(logger, SPR_DEBUG, "Found errortests dir: {%s}", stego_val);

                    Koliseo* tmp_kls = kls_new(FILENAME_MAX+1);
                    char* errortestsdir_path = KLS_PUSH_ARR(tmp_kls, char, strlen(canvil_args->tests_dir) + 1 + strlen(stego_val) +1); // +1 for / and /0
                    sprintf(errortestsdir_path, "%s/%s", canvil_args->tests_dir, stego_val);

                    spr_logf_to(logger, SPR_DEBUG, "Looking into errortests dir {%s}", errortestsdir_path);

                    struct dirent *entry;
                    DIR *dir = opendir(errortestsdir_path);
                    if (dir == NULL) {
                        spr_logf_to(logger, SPR_ERROR, "Failed opening errortests dir {%s}", errortestsdir_path);
                    } else {
                        while ((entry = readdir(dir)) != NULL) {

                            // Skip . and ..
                            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                                continue;

                            kls_clear(tmp_kls);

                            char* curr_path = KLS_PUSH_ARR(tmp_kls, char, strlen(canvil_args->tests_dir) +1 + strlen(stego_val) +1 + strlen(entry->d_name) +1); // +1 for / and /0

                            sprintf(curr_path, "%s/%s/%s", canvil_args->tests_dir, stego_val, entry->d_name);

                            struct stat st;

                            if (stat(curr_path, &st) == -1) {
                                perror("stat");
                                spr_logf_to(logger, SPR_ERROR, "Failed stat of test {%s}", curr_path);
                                continue;
                            }

                            if (S_ISDIR(st.st_mode)) {
                                spr_logf_to(logger, SPR_TRACE, "Found subdirectory: {%s}", entry->d_name);
                            } else if (S_ISREG(st.st_mode)) {
                                spr_logf_to(logger, SPR_TRACE, "Found file: {%s}", entry->d_name);
                                int is_exec = st.st_mode & S_IXUSR;
                                if (is_exec) {
                                    spr_logf_to(logger, SPR_TRACE, "Found executable file");
                                    int name_len = strlen(entry->d_name);
                                    if (name_len >= 2) {
                                        if (entry->d_name[name_len-1] == 'k' && entry->d_name[name_len-2] == '.') {
                                            Canvil_Test* canvil_test = KLS_PUSH(kls, Canvil_Test);
                                            canvil_test->dir = KLS_PUSH_STR(kls, stego_val);
                                            sprintf((char*)canvil_test->dir, "%s", stego_val);
                                            canvil_test->name = KLS_PUSH_STR(kls, entry->d_name);
                                            sprintf((char*)canvil_test->name, "%s", entry->d_name);
                                            spr_logf_to(logger, SPR_DEBUG, "Found errortest: {%s/%s}", canvil_test->dir, canvil_test->name);
                                            canvil_env->errortests = Canvil_Test_List_cons_kls(kls, canvil_test, canvil_env->errortests);
                                        }
                                    }
                                }
                            }
                        }
                        closedir(dir);
                    }
                    kls_free(tmp_kls);
                }
            }
            if (is_versions_table) {
                bool is_valid_semver = false;
                bool is_base_tag = false;
                char* tag = NULL;
                if (inner_key[0] == 'B') {
                    // This is a base mode tag. Check it without the B
                    is_base_tag = true;
                    tag = (char*) &(inner_key[1]);
                } else {
                    tag = (char*) inner_key;
                }
                is_valid_semver = validateSemVer(tag);
                if (is_valid_semver) {
                    spr_logf_to(logger, SPR_DEBUG, "Found%sversion: {%s}", (is_base_tag ? " base " : " "), tag);
                    if (collect_anvil_env) {
                        SemVer* smv = KLS_PUSH(kls, SemVer);

                        if (!parseSemVer(tag, &(smv->major), &(smv->minor), &(smv->patch))) {
                            spr_logf_to(logger, SPR_ERROR, "Failed parsing SemVer: {%s}", tag);
                        } else {
                            SemVer smv_val = *smv;
                            spr_logf_to(logger, SPR_DEBUG, "Semver: {" SemVer_Fmt "}", SemVer_Arg(smv_val));

                            Canvil_Tag* canvil_tag = KLS_PUSH(kls, Canvil_Tag);
                            *canvil_tag = (Canvil_Tag) {
                                .type = (is_base_tag ? CANVIL_GIT_TAG : CANVIL_BASE_TAG),
                                .version = smv,
                                .desc = NULL,
                            };
                            // Add the SemVer to the base/git list
                            if (is_base_tag) {
                                canvil_env->base_tags = Canvil_Tag_List_cons_kls(kls, canvil_tag, canvil_env->base_tags);
                            } else {
                                canvil_env->git_tags = Canvil_Tag_List_cons_kls(kls, canvil_tag, canvil_env->git_tags);
                            }
                        }
                    }
                } else {
                    spr_logf_to(logger, SPR_INFO, "Failed validation for version: {%s}", tag);
                }
            }
        }
    }
    toml_free(result);
    return true;
}

bool getargs_from_filepath_as_stego_global(Anvil_Args* canvil_args, const char* filepath, Koliseo* kls, Spuro logger)
{
    toml_result_t result = toml_parse_file_ex(filepath);

    if (!result.ok) {
        fprintf(stderr, "%s():    %s\n", __func__, result.errmsg);
        return false;
    }

    toml_datum_t tab = result.toptab;
    int tab_len = tab.u.tab.size;

    for (int i = 0; i < tab_len; i++) {
        const char* key = tab.u.tab.key[i];
        spr_logf_to(logger, SPR_DEBUG, "key %d: %s", i, key);
        bool is_anvil_table = (strcmp(key,"anvil") == 0);
        toml_datum_t inner_table = toml_get(tab, key);

        if (inner_table.type != TOML_TABLE) {
            //TODO: custom errors for specific erroring inner tables
            spr_logf_to(logger, SPR_ERROR, "can't get inner table for key {%s}", key);
            toml_free(result);
            return false;
        }
        int inner_tab_len = inner_table.u.tab.size;
        for (int j = 0; j < inner_tab_len; j++) {
            const char* inner_key = inner_table.u.tab.key[j];
            spr_logf_to(logger, SPR_DEBUG, "inner_key %d: %s", j, inner_key);
            if (is_anvil_table) {
                bool is_versions_key = (strcmp(inner_key, "version") == 0);
                bool is_kern_key = (strcmp(inner_key, "kern") == 0);
                if (is_versions_key) {
                    const char* stego_val = get_table_stringval(inner_table, inner_key, "anvil", logger);
                    int major, minor, patch;
                    if(!parseSemVer(stego_val, &major, &minor, &patch)) {
                        spr_logf_to(logger, SPR_ERROR, "Passed anvil_version is not a valid SemVer: {%s}", stego_val);
                        toml_free(result);
                        return false;
                    }
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
                        spr_logf_to(logger, SPR_ERROR, "Passed anvil_version {%s} is not a supported anvil version", stego_val);
                        toml_free(result);
                        return false;
                    } else {
                        if (canvil_SemVer_cmp(target, MIN_AMBOSO_V_EXTENSIONS) < 0) {
                            spr_logf_to(logger, SPR_DEBUG, "Turning off extensions");
                            canvil_args->strict = 1;
                        }
                    }

                    spr_logf_to(logger, SPR_INFO, "Using anvil version from stego.lock: {%s}", stego_val);
                    canvil_args->anvil_version_optarg = KLS_PUSH_ARR(kls, char, strlen(stego_val)+1);
                    memcpy(canvil_args->anvil_version_optarg, stego_val, strlen(stego_val)+1);
                }
                if (is_kern_key) {
                    const char* stego_val = get_table_stringval(inner_table, inner_key, "anvil", logger);
                    if (canvil_args->anvil_kern_optarg == NULL) {
                        bool matched = false;
                        if (!strcmp(stego_val, "amboso-C")) {
                            matched = true;
                        }
                        if (matched) {
                            spr_logf_to(logger, SPR_INFO, "Using anvil kern from stego.lock: {%s}", stego_val);
                            canvil_args->anvil_kern_optarg = KLS_PUSH_ARR(kls, char, strlen(stego_val)+1);
                            memcpy(canvil_args->anvil_kern_optarg, stego_val, strlen(stego_val)+1);
                        } else {
                            spr_logf_to(logger, SPR_ERROR, "Unsupported anvil_kern: {%s}", stego_val);
                            toml_free(result);
                            return false;
                        }
                    }
                }
            }
        }
    }
    toml_free(result);
    return true;
}

bool lint_stegopath(const char* stego_path, Canvil_Lint_Mode mode, Spuro logger)
{
    switch (mode) {
        case CANVIL_LINT_FULL_CHECK: {
            return check_filepath_as_stego(stego_path, logger);
        }
        break;
        case CANVIL_LINT_LEX: {
            return lex_filepath_as_stego(stego_path, logger);
        }
        break;
        case CANVIL_LINT_ONLY: {
            toml_result_t result = toml_parse_file_ex(stego_path);

            if (!result.ok) {
                fprintf(stderr, "%s():    %s\n", __func__, result.errmsg);
                return false;
            }
            toml_free(result);
            return true;
        }
        break;
        default: {
            spr_logf_to(logger, SPR_ERROR, "Unexpected mode: {%i}", mode);
            return false;
        }
        break;
    }
}

bool lex_filepath_as_stego(const char* stego_path, Spuro logger)
{
    toml_result_t result = toml_parse_file_ex(stego_path);

    if (!result.ok) {
        fprintf(stderr, "%s():    %s\n", __func__, result.errmsg);
        return false;
    }

    toml_datum_t tab = result.toptab;
    int tab_len = tab.u.tab.size;
    for (int i = 0; i < tab_len; i++) {
        const char* key = tab.u.tab.key[i];
        spr_logf_to(logger, SPR_TRACE, "key %d: %s", i, key);
        printf("Scope: %s\n", key);

        toml_datum_t inner_table = toml_get(tab, key);
        if (inner_table.type != TOML_TABLE) {
            //TODO: custom errors for specific erroring inner tables
            spr_logf_to(logger, SPR_ERROR, "can't get inner table for key {%s}", key);
            toml_free(result);
            return false;
        }
        int inner_tab_len = inner_table.u.tab.size;
        for (int j = 0; j < inner_tab_len; j++) {
            const char* inner_key = inner_table.u.tab.key[j];
            spr_logf_to(logger, SPR_TRACE, "inner_key %d: %s", j, inner_key);
            toml_datum_t inner_val = toml_get(inner_table, inner_key);
            if (inner_val.type == TOML_STRING) {
                const char* stego_val = get_table_stringval(inner_table, inner_key, key, logger);

                if (stego_val != NULL) {
                    spr_logf_to(logger, SPR_TRACE, "value: %s", stego_val);
                    printf("Variable: %s_%s, Value: %s\n", key, inner_key, stego_val);
                } else {
                    spr_logf_to(logger, SPR_ERROR, "can't get value for inner key {%s} in table {%s}", inner_key, key);
                }
            } else if (inner_val.type == TOML_ARRAY) {
                printf("Array: %s_%s, name: %s\n", key, inner_key, inner_key);
                int inner_inner_array_len = inner_val.u.arr.size;
                for (int k = 0; k < inner_inner_array_len; k++) {
                    toml_datum_t inner_inner_val = inner_val.u.arr.elem[k];
                    if (inner_inner_val.type == TOML_TABLE) {
                        int inner_inner_table_len = inner_inner_val.u.tab.size;
                        for (int y = 0; y < inner_inner_table_len; y++) {
                            const char* inner_inner_key = inner_inner_val.u.tab.key[y];
                            toml_datum_t inner_inner_inner_val = toml_get(inner_inner_val, inner_inner_key);
                            if (inner_inner_inner_val.type == TOML_STRING) {
                                const char* stego_val = inner_inner_inner_val.u.s;
                                if (stego_val != NULL) {
                                    spr_logf_to(logger, SPR_TRACE, "value: %s", stego_val);
                                    printf("In-Arr Structvalue: %s_%s_%i[%s], Value: %s\n", key, inner_key, k, inner_inner_key, stego_val);
                                } else {
                                    spr_logf_to(logger, SPR_ERROR, "can't get value for inner_inner key {%s[%i]} in table {%s_%s}", inner_inner_key, k, key, inner_key);
                                }
                            }
                        }
                    } else if (inner_inner_val.type == TOML_STRING) {
                        const char* stego_val = inner_inner_val.u.s;
                        if (stego_val != NULL) {
                            spr_logf_to(logger, SPR_TRACE, "value: %s", stego_val);
                            printf("Variable: %s_%s[%i], Value: %s\n", key, inner_key, k, stego_val);
                        } else {
                            spr_logf_to(logger, SPR_ERROR, "can't get value for inner key {%s[%i]} in table {%s}", inner_key, k, key);
                        }
                    }
                }
            }
        }
        printf("------------------------\n");
    }

    toml_free(result);
    return true;
}

bool anvilpy_getenv_from_filepath(const char* filepath, AnvilPy_Env* canvil_py_env, Koliseo* kls, Spuro logger) {

    toml_result_t result = toml_parse_file_ex(filepath);

    if (!result.ok) {
        fprintf(stderr, "%s():    %s\n", __func__, result.errmsg);
        return false;
    }

    toml_datum_t tab = result.toptab;
    int tab_len = tab.u.tab.size;
    for (int i = 0; i < tab_len; i++) {
        const char* key = tab.u.tab.key[i];
        spr_logf_to(logger, SPR_TRACE, "key %d: %s", i, key);

        toml_datum_t inner_table = toml_get(tab, key);
        if (inner_table.type != TOML_TABLE) {
            //TODO: custom errors for specific erroring inner tables
            spr_logf_to(logger, SPR_ERROR, "can't get inner table for key {%s}", key);
            toml_free(result);
            return false;
        }
        int inner_tab_len = inner_table.u.tab.size;
        for (int j = 0; j < inner_tab_len; j++) {
            const char* inner_key = inner_table.u.tab.key[j];
            spr_logf_to(logger, SPR_TRACE, "inner_key %d: %s", j, inner_key);

            bool is_authors_table = (strcmp(inner_key,"authors") == 0);
            bool is_classifiers_table = (strcmp(inner_key,"classifiers") == 0);
            bool is_scripts_table = (strcmp(inner_key,"scripts") == 0);
            bool is_urls_table = (strcmp(inner_key,"urls") == 0);
            bool is_requires_table = (strcmp(inner_key,"requires") == 0);

            if (is_authors_table) {
                toml_datum_t inner_array = toml_get(inner_table, inner_key);
                if (inner_array.type != TOML_ARRAY) {
                    spr_logf_to(logger, SPR_ERROR, "can't get inner array for key {%s}", inner_key);
                    toml_free(result);
                    return false;
                }
                int array_size = inner_array.u.arr.size;
                canvil_py_env->authors = KLS_PUSH_ARR(kls, Author*, array_size);
                canvil_py_env->authors_len = array_size;
                for (int k = 0; k < array_size ; k++) {
                    toml_datum_t inner_inner_table = inner_array.u.arr.elem[k];
                    if (inner_inner_table.type != TOML_TABLE) {
                        spr_logf_to(logger, SPR_ERROR, "can't get inner table #%i for key {%s}", k, inner_key);
                        toml_free(result);
                        return false;
                    }
                    canvil_py_env->authors[k] = KLS_PUSH(kls, Author);
                    int inner_inner_table_len = inner_inner_table.u.tab.size;
                    for (int x = 0; x < inner_inner_table_len; x++) {
                        const char* inner_inner_key = inner_inner_table.u.tab.key[x];
                        spr_logf_to(logger, SPR_TRACE, "inner_inner_key %d: %s", x, inner_inner_key);
                        const char* stego_val = get_table_stringval(inner_inner_table, inner_inner_key, inner_key, logger);
                        if (!strcmp(inner_inner_key, "name")) {
                            canvil_py_env->authors[k]->name = KLS_PUSH_ARR(kls, char, strlen(stego_val)+1);
                            memcpy(canvil_py_env->authors[k]->name, stego_val, strlen(stego_val)+1);
                        } else if (!strcmp(inner_inner_key, "email")) {
                            canvil_py_env->authors[k]->email = KLS_PUSH_ARR(kls, char, strlen(stego_val)+1);
                            memcpy(canvil_py_env->authors[k]->email, stego_val, strlen(stego_val)+1);
                        }

                        if (stego_val != NULL) {
                            spr_logf_to(logger, SPR_TRACE, "value: %s", stego_val);
                            spr_logf_to(logger, SPR_DEBUG, "Variable: %s_%s_%s[%d], Value: %s", key, inner_key, inner_inner_key, k, stego_val);
                        } else {
                            spr_logf_to(logger, SPR_ERROR, "can't get value for inner inner key {%s} in table {%s} in table {%s}", inner_inner_key, inner_key, key);
                        }
                    }
                }
            } else if (is_classifiers_table) {
                toml_datum_t inner_array = toml_get(inner_table, inner_key);
                if (inner_array.type != TOML_ARRAY) {
                    spr_logf_to(logger, SPR_ERROR, "can't get array for inner key {%s}", inner_key);
                    toml_free(result);
                    return false;
                }
                int array_size = inner_array.u.arr.size;
                canvil_py_env->classifiers = KLS_PUSH_ARR(kls, char*, array_size);
                canvil_py_env->classifiers_len = array_size;
                for (int k = 0; k < array_size ; k++) {
                    toml_datum_t string_val = inner_array.u.arr.elem[k];
                    if (string_val.type != TOML_STRING) continue;
                    const char* stego_val = string_val.u.s;
                    canvil_py_env->classifiers[k] = KLS_PUSH_ARR(kls, char, strlen(stego_val)+1);

                    memcpy(canvil_py_env->classifiers[k], stego_val, strlen(stego_val)+1);

                    if (stego_val != NULL) {
                        spr_logf_to(logger, SPR_TRACE, "value: %s", stego_val);
                        spr_logf_to(logger, SPR_DEBUG, "Variable: %s_%s[%d], Value: %s", key, inner_key, k, stego_val);
                    } else {
                        spr_logf_to(logger, SPR_ERROR, "can't get value for inner key {%s} in table {%s}", inner_key, key);
                    }
                }
            } else if (is_scripts_table) {
                toml_datum_t inner_inner_table = toml_get(inner_table, inner_key);
                if (inner_inner_table.type != TOML_TABLE) {
                    spr_logf_to(logger, SPR_ERROR, "can't get inner table for key {%s}", inner_key);
                    toml_free(result);
                    return false;
                }
                int tot_scripts = inner_inner_table.u.tab.size;
                canvil_py_env->scripts_len = tot_scripts;
                canvil_py_env->scripts = KLS_PUSH_ARR(kls, ScriptEntry*, tot_scripts);
                for (int k=0; k < tot_scripts; k++) {
                    const char* inner_inner_key = inner_inner_table.u.tab.key[k];
                    const char* stego_val = get_table_stringval(inner_inner_table, inner_inner_key, inner_key, logger);
                    canvil_py_env->scripts[k] = KLS_PUSH(kls, ScriptEntry);
                    canvil_py_env->scripts[k]->name = KLS_PUSH_ARR(kls, char, strlen(inner_inner_key)+1);
                    memcpy(canvil_py_env->scripts[k]->name, inner_inner_key, strlen(inner_inner_key)+1);
                    canvil_py_env->scripts[k]->entrypoint = KLS_PUSH_ARR(kls, char, strlen(stego_val)+1);
                    memcpy(canvil_py_env->scripts[k]->entrypoint, stego_val, strlen(stego_val)+1);
                }
            } else if (is_urls_table) {
                toml_datum_t inner_inner_table = toml_get(inner_table, inner_key);
                if (inner_inner_table.type != TOML_TABLE) {
                    spr_logf_to(logger, SPR_ERROR, "can't get inner table for key {%s}", inner_key);
                    toml_free(result);
                    return false;
                }
                size_t tot_urls = inner_inner_table.u.tab.size;
                canvil_py_env->urls_len = tot_urls;
                canvil_py_env->urls = KLS_PUSH_ARR(kls, UrlEntry*, tot_urls);
                for (int k=0; k < tot_urls; k++) {
                    const char* inner_inner_key = inner_inner_table.u.tab.key[k];
                    const char* stego_val = get_table_stringval(inner_inner_table, inner_inner_key, inner_key, logger);
                    canvil_py_env->urls[k] = KLS_PUSH(kls, UrlEntry);
                    canvil_py_env->urls[k]->name = KLS_PUSH_ARR(kls, char, strlen(inner_inner_key)+1);
                    memcpy(canvil_py_env->urls[k]->name, inner_inner_key, strlen(inner_inner_key)+1);
                    canvil_py_env->urls[k]->link = KLS_PUSH_ARR(kls, char, strlen(stego_val)+1);
                    memcpy(canvil_py_env->urls[k]->link, stego_val, strlen(stego_val)+1);
                }
            } else if (is_requires_table) {
                toml_datum_t inner_array = toml_get(inner_table, inner_key);
                if (inner_array.type != TOML_ARRAY) {
                    spr_logf_to(logger, SPR_ERROR, "can't get inner array for key {%s}", inner_key);
                    toml_free(result);
                    return false;
                }
                int array_size = inner_array.u.arr.size;
                canvil_py_env->build_system.reqs_len = array_size;
                canvil_py_env->build_system.reqs = KLS_PUSH_ARR(kls, char*, array_size);
                for (int k = 0; k < array_size ; k++) {
                    toml_datum_t string_val = inner_array.u.arr.elem[k];
                    if (string_val.type != TOML_STRING) continue;
                    const char* stego_val = string_val.u.s;
                    canvil_py_env->build_system.reqs[k] = KLS_PUSH_ARR(kls, char, strlen(stego_val)+1);
                    memcpy(canvil_py_env->build_system.reqs[k], stego_val, strlen(stego_val)+1);

                    if (stego_val != NULL) {
                        spr_logf_to(logger, SPR_TRACE, "value: %s", stego_val);
                        spr_logf_to(logger, SPR_DEBUG, "Variable: %s_%s[%d], Value: %s", key, inner_key, k, stego_val);
                    } else {
                        spr_logf_to(logger, SPR_ERROR, "can't get value for inner key {%s} in table {%s}", inner_key, key);
                    }
                }
            } else {
                const char* stego_val = get_table_stringval(inner_table, inner_key, key, logger);

                if (!strcmp(inner_key, "name")) {
                    canvil_py_env->proj_name = KLS_PUSH_ARR(kls, char, strlen(stego_val)+1);
                    memcpy(canvil_py_env->proj_name, stego_val, strlen(stego_val)+1);
                } else if (!strcmp(inner_key, "version")) {
                    canvil_py_env->version = KLS_PUSH_ARR(kls, char, strlen(stego_val)+1);
                    memcpy(canvil_py_env->version, stego_val, strlen(stego_val)+1);
                } else if (!strcmp(inner_key, "description")) {
                    canvil_py_env->description = KLS_PUSH_ARR(kls, char, strlen(stego_val)+1);
                    memcpy(canvil_py_env->description, stego_val, strlen(stego_val)+1);
                } else if (!strcmp(inner_key, "readme")) {
                    canvil_py_env->readme_path = KLS_PUSH_ARR(kls, char, strlen(stego_val)+1);
                    memcpy(canvil_py_env->readme_path, stego_val, strlen(stego_val)+1);
                } else if (!strcmp(inner_key, "requires-python")) {
                    canvil_py_env->python_version_req = KLS_PUSH_ARR(kls, char, strlen(stego_val)+1);
                    memcpy(canvil_py_env->python_version_req, stego_val, strlen(stego_val)+1);
                } else if (!strcmp(inner_key, "build-backend")) {
                    canvil_py_env->build_system.backend = KLS_PUSH_ARR(kls, char, strlen(stego_val)+1);
                    memcpy(canvil_py_env->build_system.backend, stego_val, strlen(stego_val)+1);
                }

                if (stego_val != NULL) {
                    spr_logf_to(logger, SPR_TRACE, "value: %s", stego_val);
                    spr_logf_to(logger, SPR_DEBUG, "Variable: %s_%s, Value: %s", key, inner_key, stego_val);
                } else {
                    spr_logf_to(logger, SPR_ERROR, "can't get value for inner key {%s} in table {%s}", inner_key, key);
                }
            }
        }
    }

    toml_free(result);
    return true;
}

bool lex_anvilpy_from_filepath(const char* filepath, Spuro logger) {

    toml_result_t result = toml_parse_file_ex(filepath);

    if (!result.ok) {
        fprintf(stderr, "%s():    %s\n", __func__, result.errmsg);
        return false;
    }

    toml_datum_t tab = result.toptab;
    int tab_len = tab.u.tab.size;
    for (int i = 0; tab_len; i++) {
        const char* key = tab.u.tab.key[i];
        spr_logf_to(logger, SPR_TRACE, "key %d: %s", i, key);
        printf("Scope: %s\n", key);

        toml_datum_t inner_table = toml_get(tab, key);
        if (inner_table.type != TOML_TABLE) {
            spr_logf_to(logger, SPR_ERROR, "can't get inner table for key {%s}", key);
            toml_free(result);
            return false;
        }
        int inner_tab_len = inner_table.u.tab.size;
        for (int j = 0; inner_tab_len; j++) {
            const char* inner_key = inner_table.u.tab.key[j];
            spr_logf_to(logger, SPR_TRACE, "inner_key %d: %s", j, inner_key);

            bool is_authors_table = (strcmp(inner_key,"authors") == 0);
            bool is_classifiers_table = (strcmp(inner_key,"classifiers") == 0);
            bool is_scripts_table = (strcmp(inner_key,"scripts") == 0);
            bool is_urls_table = (strcmp(inner_key,"urls") == 0);
            bool is_requires_table = (strcmp(inner_key,"requires") == 0);

            if (is_authors_table) {
                toml_datum_t inner_array = toml_get(inner_table, inner_key);
                if (inner_array.type != TOML_ARRAY) {
                    spr_logf_to(logger, SPR_ERROR, "can't get inner array for inner_key {%s}", inner_key);
                    toml_free(result);
                    return false;
                }
                int array_size = inner_array.u.arr.size;
                for (int k = 0; k < array_size ; k++) {
                    toml_datum_t inner_inner_table = inner_array.u.arr.elem[k];
                    if (inner_inner_table.type != TOML_TABLE) {
                        spr_logf_to(logger, SPR_ERROR, "can't get inner table #%i for inner_key {%s}", k, inner_key);
                        toml_free(result);
                        return false;
                    }
                    int inner_inner_tab_len = inner_inner_table.u.tab.size;
                    for (int x = 0; x < inner_inner_tab_len; x++) {
                        const char* inner_inner_key = inner_inner_table.u.tab.key[x];
                        spr_logf_to(logger, SPR_TRACE, "inner_inner_key %d: %s", x, inner_inner_key);
                        const char* stego_val = get_table_stringval(inner_inner_table, inner_inner_key, inner_key, logger);
                        if (stego_val != NULL) {
                            spr_logf_to(logger, SPR_TRACE, "value: %s", stego_val);
                            printf("Variable: %s_%s_%s[%d], Value: %s\n", key, inner_key, inner_inner_key, k, stego_val);
                        } else {
                            spr_logf_to(logger, SPR_ERROR, "can't get value for inner inner key {%s} in table {%s} in table {%s}", inner_inner_key, inner_key, key);
                        }
                    }
                }
            } else if (is_classifiers_table) {
                toml_datum_t inner_array = toml_get(inner_table, inner_key);
                if (inner_array.type != TOML_ARRAY) {
                    spr_logf_to(logger, SPR_ERROR, "can't get inner array for inner_key {%s}", inner_key);
                    toml_free(result);
                    return false;
                }
                int array_size = inner_array.u.arr.size;
                for (int k = 0; k < array_size ; k++) {
                    toml_datum_t string_val = inner_array.u.arr.elem[k];
                    if (string_val.type != TOML_STRING) continue;
                    const char* stego_val = string_val.u.s;

                    if (stego_val != NULL) {
                        spr_logf_to(logger, SPR_TRACE, "value: %s", stego_val);
                        printf("Variable: %s_%s[%d], Value: %s\n", key, inner_key, k, stego_val);
                    } else {
                        spr_logf_to(logger, SPR_ERROR, "can't get value for inner key {%s} in table {%s}", inner_key, key);
                    }
                }
            } else if (is_scripts_table) {
                toml_datum_t inner_inner_table = toml_get(inner_table, inner_key);
                if (inner_inner_table.type != TOML_TABLE) {
                    spr_logf_to(logger, SPR_ERROR, "can't get inner table for inner_key {%s}", inner_key);
                    toml_free(result);
                    return false;
                }
                size_t tot_scripts = inner_inner_table.u.tab.size;
                for (int k = 0; k < tot_scripts; k++) {
                    const char* inner_inner_key = inner_inner_table.u.tab.key[k];
                    spr_logf_to(logger, SPR_TRACE, "inner_inner_key %d: %s", k, inner_inner_key);
                    const char* stego_val = get_table_stringval(inner_inner_table, inner_inner_key, inner_key, logger);

                    if (stego_val != NULL) {
                        spr_logf_to(logger, SPR_TRACE, "value: %s", stego_val);
                        printf("Variable: %s_%s_%s, Value: %s\n", key, inner_key, inner_inner_key, stego_val);
                    } else {
                        spr_logf_to(logger, SPR_ERROR, "can't get value for inner inner key {%s} in table {%s} in table {%s}", inner_inner_key, inner_key, key);
                    }
                }
            } else if (is_urls_table) {
                toml_datum_t inner_inner_table = toml_get(inner_table, inner_key);
                if (inner_inner_table.type != TOML_TABLE) {
                    spr_logf_to(logger, SPR_ERROR, "can't get inner table for inner_key {%s}", inner_key);
                    toml_free(result);
                    return false;
                }
                size_t tot_urls = inner_inner_table.u.tab.size;
                for (int k = 0; k < tot_urls; k++) {
                    const char* inner_inner_key = inner_inner_table.u.tab.key[k];
                    spr_logf_to(logger, SPR_TRACE, "inner_inner_key %d: %s", k, inner_inner_key);
                    const char* stego_val = get_table_stringval(inner_inner_table, inner_inner_key, inner_key, logger);

                    if (stego_val != NULL) {
                        spr_logf_to(logger, SPR_TRACE, "value: %s", stego_val);
                        printf("Variable: %s_%s_%s, Value: %s\n", key, inner_key, inner_inner_key, stego_val);
                    } else {
                        spr_logf_to(logger, SPR_ERROR, "can't get value for inner inner key {%s} in table {%s} in table {%s}", inner_inner_key, inner_key, key);
                    }
                }
            } else if (is_requires_table) {
                toml_datum_t inner_array = toml_get(inner_table, inner_key);
                if (inner_array.type != TOML_ARRAY) {
                    spr_logf_to(logger, SPR_ERROR, "can't get inner array for inner_key {%s}", inner_key);
                    toml_free(result);
                    return false;
                }
                int array_size = inner_array.u.arr.size;
                for (int k = 0; k < array_size ; k++) {
                    toml_datum_t string_val = inner_array.u.arr.elem[k];
                    if (string_val.type != TOML_STRING) continue;
                    const char* stego_val = string_val.u.s;

                    if (stego_val != NULL) {
                        spr_logf_to(logger, SPR_TRACE, "value: %s", stego_val);
                        printf("Variable: %s_%s[%d], Value: %s\n", key, inner_key, k, stego_val);
                    } else {
                        spr_logf_to(logger, SPR_ERROR, "can't get value for inner key {%s} in table {%s}", inner_key, key);
                    }
                }
            } else {
                const char* stego_val = get_table_stringval(inner_table, inner_key, key, logger);

                if (stego_val != NULL) {
                    spr_logf_to(logger, SPR_TRACE, "value: %s", stego_val);
                    printf("Variable: %s_%s, Value: %s\n", key, inner_key, stego_val);
                } else {
                    spr_logf_to(logger, SPR_ERROR, "can't get value for inner key {%s} in table {%s}", inner_key, key);
                }
            }
        }
        printf("------------------------\n");
    }

    toml_free(result);
    return true;
}
