// jgabaut @ github.com/jgabaut
// SPDX-License-Identifier: GPL-3.0-only
/*
    Copyright (C) 2024 jgabaut

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
#ifndef CANVIL_PY_ENV_H_
#define CANVIL_PY_ENV_H_
#include "canvil_log.h"

typedef struct Author {
    char *name;
    char *email;
} Author;

typedef struct UrlEntry {
    char *name;
    char *link;
} UrlEntry;

typedef struct ScriptEntry {
    char *name;
    char *entrypoint;
} ScriptEntry;

typedef struct BuildSystem {
    char **reqs;
    size_t reqs_len;
    char *backend;
} BuildSystem;

typedef struct AnvilPy_Env {
    char *proj_name;

    char *version;

    Author **authors;
    size_t authors_len;

    char *description;

    char *readme_path;

    char *python_version_req;

    char **classifiers;
    size_t classifiers_len;

    ScriptEntry **scripts;
    size_t scripts_len;

    UrlEntry **urls;
    size_t urls_len;

    BuildSystem build_system;
} AnvilPy_Env;

void print_anvilpy_env(AnvilPy_Env env, Spuro logger);

#endif // CANVIL_PY_ENV_H_
