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
#include "canvil_core.h"
const char *strictSemVerRegex = "^(0|[1-9][0-9]*)\\.(0|[1-9][0-9]*)\\.(0|[1-9][0-9]*)$";
const char *extendedSemVerRegex = "^(0|[1-9][0-9]*)\\.(0|[1-9][0-9]*)\\.(0|[1-9][0-9]*)(-(0|[1-9][0-9]*|[0-9]*[a-zA-Z-][0-9a-zA-Z-]*)(\\.(0|[1-9][0-9]*|[0-9]*[a-zA-Z-][0-9a-zA-Z-]*))*)?$";
const char *customBuilderRegex = "[][$`\";|&><*()#{}]";

const SemVer MIN_AMBOSO_V_EXTENSIONS = {
    .major = 2, .minor = 0, .patch = 1,
};

const SemVer MIN_AMBOSO_V_KERN = {
    .major = 2, .minor = 0, .patch = 2,
};

const SemVer MIN_AMBOSO_V_TREEGEN = {
    .major = 2, .minor = 0, .patch = 4,
};

const SemVer MIN_AMBOSO_V_ANVILPY_KERN = {
    .major = 2, .minor = 1, .patch = 0,
};

const SemVer MIN_AMBOSO_V_REFUSE_TI = {
    .major = 2, .minor = 0, .patch = 11,
};

SemVer supported_anvil_versions[17] = {
    { .major = 2, .minor = 0, .patch = 0 },
    MIN_AMBOSO_V_EXTENSIONS,
    MIN_AMBOSO_V_KERN,
    { .major = 2, .minor = 0, .patch = 3 },
    MIN_AMBOSO_V_TREEGEN,
    { .major = 2, .minor = 0, .patch = 5 },
    { .major = 2, .minor = 0, .patch = 6 },
    { .major = 2, .minor = 0, .patch = 7 },
    { .major = 2, .minor = 0, .patch = 8 },
    { .major = 2, .minor = 0, .patch = 9 },
    { .major = 2, .minor = 0, .patch = 10 },
    MIN_AMBOSO_V_REFUSE_TI,
    { .major = 2, .minor = 0, .patch = 12 },
    MIN_AMBOSO_V_ANVILPY_KERN,
    { .major = 2, .minor = 1, .patch = 1 },
    { .major = 2, .minor = 1, .patch = 2 },
    { .major = 2, .minor = 1, .patch = 3 },
};

// Function to validate SemVer string
bool validateSemVer(const char *input) {
    regex_t re_strict, re_extended;
    int ret;
    char errbuf[100];

    // Compile regular expressions
    if ((ret = regcomp(&re_strict, strictSemVerRegex, REG_EXTENDED)) != 0) {
        regerror(ret, &re_strict, errbuf, sizeof(errbuf));
        fprintf(stderr, "Error compiling regex for strict SemVer: %s\n", errbuf);
        return false;
    }
    if ((ret = regcomp(&re_extended, extendedSemVerRegex, REG_EXTENDED)) != 0) {
        regerror(ret, &re_extended, errbuf, sizeof(errbuf));
        fprintf(stderr, "Error compiling regex for extended SemVer: %s\n", errbuf);
        regfree(&re_strict);
        return false;
    }

    // Check against strict SemVer regex
    if (regexec(&re_strict, input, 0, NULL, 0) == 0) {
        // Pass: Strict SemVer match
        regfree(&re_strict);
        regfree(&re_extended);
        return true;
    }

    // If strict SemVer fails, try extended SemVer
    if (regexec(&re_extended, input, 0, NULL, 0) == 0) {
        // Fail: Extended SemVer match
        //printf("Extended SemVer validation passed but only strict SemVer is supported.\n");
        regfree(&re_strict);
        regfree(&re_extended);
        return false;
    }

    // Neither strict nor extended SemVer match
    printf("SemVer validation failed.\n");
    regfree(&re_strict);
    regfree(&re_extended);
    return false;
}

/**
 * Function to tokenize the SemVer string and extract major, minor, and patch versions
 */
bool parseSemVer(const char *input, int *major, int *minor, int *patch) {
    char *token;
    char *semver_copy = strdup(input); // Create a copy of the input string to avoid modifying the original
    if (semver_copy == NULL) {
        fprintf(stderr, "%s():    Memory allocation failed.\n", __func__);
        return false;
    }
    // Tokenize the SemVer string using dot '.' as the delimiter
    token = strtok(semver_copy, ".");
    if (token != NULL) {
        *major = atoi(token); // Convert token to integer and assign it to major version
        token = strtok(NULL, ".");
        if (token != NULL) {
            *minor = atoi(token); // Convert token to integer and assign it to minor version
            token = strtok(NULL, ".");
            if (token != NULL) {
                *patch = atoi(token); // Convert token to integer and assign it to patch version
            }
        }
    }
    free(semver_copy); // Free the allocated memory for the copied string
    return true;
}

bool validateCustomBuilder(const char* input)
{
    regex_t re;
    int ret;
    char errbuf[100];

    // Compile regular expressions
    if ((ret = regcomp(&re, customBuilderRegex, REG_EXTENDED)) != 0) {
        regerror(ret, &re, errbuf, sizeof(errbuf));
        fprintf(stderr, "Error compiling regex for custom builder: %s\n", errbuf);
        return false;
    }

    // Check against strict SemVer regex
    if (regexec(&re, input, 0, NULL, 0) == 0) {
        // Pass: Strict SemVer match
        regfree(&re);
        return false;
    }
    return true;
}

bool canvil_filepath_exists(const char* file_path)
{
    FILE* fp = fopen(file_path, "r");
    if (fp != NULL) {
        fclose(fp);
        return true;
    } else {
        return false;
    }
}

bool canvil_check_dir(const char* dir_path)
{
    struct stat sb;

    if (stat(dir_path, &sb) == 0 && S_ISDIR(sb.st_mode)) {
        // Dir exists
        return true;
    } else {
        return false;
    }
}

bool canvil_check_dir_create(const char* dir_path)
{
    if (canvil_check_dir(dir_path)) {
        // Dir exists
        return true;
    } else {
        // Dir doesn't exist, try creating it
#ifndef _WIN32
        int mkdir_res = mkdir(dir_path, 0777);
#else
        int mkdir_res = mkdir(dir_path);
#endif
        if (mkdir_res != 0) {
            spr_tracef("Failed creating dir {%s}", dir_path);
            return false;
        } else {
            spr_tracef("Could not find {%s} at first, so it was created.\n", dir_path);
            return true;
        }
    }
}

int canvil_SemVer_cmp(SemVer a, SemVer b)
{
    if (a.major != b.major) return (a.major > b.major) ? 1 : -1;

    if (a.minor != b.minor) return (a.minor > b.minor) ? 1 : -1;

    if (a.patch != b.patch) return (a.patch > b.patch) ? 1 : -1;

    return 0; // equal
}

bool canvil_tag_cmp(const Canvil_Tag *a, const Canvil_Tag *b)
{
    int res = canvil_SemVer_cmp(*(a->version), *(b->version));
    return (res == 0);
}

bool canvil_test_cmp(const Canvil_Test *a, const Canvil_Test *b)
{
    int res = strcmp(a->name, b->name);
    return (res == 0);
}
