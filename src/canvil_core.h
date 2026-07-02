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
#ifndef CANVIL_CORE_H_
#define CANVIL_CORE_H_

#define CANVIL_MAJOR 0
#define CANVIL_MINOR 1
#define CANVIL_PATCH 2

#define EXPECTED_AMBOSO_API_LEVEL "2.1.3"

#ifndef _WIN32
#include <regex.h>
#else
#include <pcre2posix.h>
#endif // _WIN32
#include "../koliseo/src/koliseo.h"
#include "canvil_log.h"
#include <inttypes.h>
#include <sys/stat.h>

typedef struct SemVer {
    int32_t major;
    int32_t minor;
    int32_t patch;
} SemVer;

#define SemVer_Fmt "%" PRId32 ".%" PRId32 ".%" PRId32

#define SemVer_Arg(s) s.major, s.minor, s.patch

extern const SemVer MIN_AMBOSO_V_EXTENSIONS;
extern const SemVer MIN_AMBOSO_V_KERN;
extern const SemVer MIN_AMBOSO_V_TREEGEN;
extern const SemVer MIN_AMBOSO_V_ANVILPY_KERN;
extern const SemVer MIN_AMBOSO_V_REFUSE_TI;
extern SemVer supported_anvil_versions[17];

typedef enum Canvil_Tag_Type {
    CANVIL_BASE_TAG = 0,
    CANVIL_GIT_TAG,
} Canvil_Tag_Type;

typedef struct Canvil_Tag {
    Canvil_Tag_Type type;
    SemVer* version;
    const char* desc;
} Canvil_Tag;

typedef struct Canvil_Test {
    const char* dir;
    const char* name;
} Canvil_Test;

typedef enum Canvil_Lint_Mode {
    CANVIL_LINT_FULL_CHECK = 0,
    CANVIL_LINT_ONLY,
    CANVIL_LINT_LEX,
} Canvil_Lint_Mode;

typedef struct Anvil_Args {
    int version;
    int warranty;
    int help;
    int linter;
    char* linter_optarg;
    int base_mode;
    int git_mode;
    int passed_stego_dir;
    char* stegodir_optarg;
    int passed_builds_dir;
    char* builds_dir_optarg;
    int passed_target_dir;
    char* targetdir_optarg;
    int passed_source_name;
    char* source_optarg;
    int passed_bin_name;
    char* bin_optarg;
    int passed_minmake_tag;
    char* minmake_optarg;
    int passed_kazoj_dir;
    char* kazojdir_optarg;
    int ignore_gitcheck;
    int gen_header;
    char* passed_gen_header_dir;
    int passed_config_arg;
    char* config_optarg;
    int list;
    int list_all;
    int do_init;
    int do_purge;
    int do_delete;
    int do_build;
    int do_run;
    char* tests_dir;
    int do_test;
    char* passed_test_name;
    int do_test_macro;
    int force_build;
    int no_rebuild;
    int no_color;
    int logged;
    int strict;
    int passed_anvil_version;
    char* anvil_version_optarg;
    int passed_anvil_kern;
    char* anvil_kern_optarg;
    char* anvil_custombuilder;
    int passed_cflags;
    char* cflags_optarg;
    char* minautomake_version;
    int watch;
    char** extra_args;
    size_t extra_args_len;
} Anvil_Args;

/**
 * Defines current API version number from CANVIL_MAJOR, CANVIL_MINOR and CANVIL_PATCH.
 */
static const int CANVIL_API_VERSION_INT =
    (CANVIL_MAJOR * 1000000 + CANVIL_MINOR * 10000 + CANVIL_PATCH * 100);
/**< Represents current version with numeric format.*/

/**
 * Defines current API version string.
 */
static const char CANVIL_API_VERSION_STRING[] = "0.1.2"; /**< Represents current version with MAJOR.MINOR.PATCH format.*/

/**
 * Defines current OS string.
 */
#ifdef _WIN32
static const char CANVIL_OS_STRING[] = "Windows"; /**< Represents current OS used.*/
#elif __APPLE__
static const char CANVIL_OS_STRING[] = "macOS"; /**< Represents current OS used.*/
#elif __linux__
static const char CANVIL_OS_STRING[] = "linux"; /**< Represents current OS used.*/
#else
static const char CANVIL_OS_STRING[] = "unknown"; /**< Represents current OS used.*/
#endif // _WIN32

extern const char *strictSemVerRegex;
extern const char *extendedSemVerRegex;
bool validateSemVer(const char *input);
bool parseSemVer(const char* input, int* major, int* minor, int* patch);
bool validateCustomBuilder(const char* input);
bool canvil_filepath_exists(const char* file_path);
bool canvil_check_dir(const char* dir_path);
bool canvil_check_dir_create(const char* dir_path);
int canvil_SemVer_cmp(SemVer a, SemVer b);
bool canvil_tag_cmp(const Canvil_Tag *a, const Canvil_Tag *b);
bool canvil_test_cmp(const Canvil_Test *a, const Canvil_Test *b);
#endif // CANVIL_CORE_H_
