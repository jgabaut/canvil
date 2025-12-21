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

#include <stddef.h>
#include "canvil_py_env.h"

void print_anvilpy_env(AnvilPy_Env env, Spuro logger) {
    SpuroLevel lvl = SPR_DEBUG;

    spr_logf_to(logger, lvl, "Project name: {%s}", env.proj_name);
    spr_logf_to(logger, lvl, "Project version: {%s}", env.version);
    spr_logf_to(logger, lvl, "Project description: {%s}", env.description);
    spr_logf_to(logger, lvl, "Readme path: {%s}", env.readme_path);
    spr_logf_to(logger, lvl, "Python version requirement: {%s}", env.python_version_req);

    for (size_t i=0; i < env.authors_len; i++) {
        spr_logf_to(logger, lvl, "Author [%zu]: {name: %s, email: %s}", i, env.authors[i]->name, env.authors[i]->email);
    }

    for (size_t i=0; i < env.classifiers_len; i++) {
        spr_logf_to(logger, lvl, "Classifier [%zu]: {%s}", i, env.classifiers[i]);
    }

    for (size_t i=0; i < env.scripts_len; i++) {
        spr_logf_to(logger, lvl, "Script [%zu]: {name: %s, entrypoint: %s}", i, env.scripts[i]->name, env.scripts[i]->entrypoint);
    }

    for (size_t i=0; i < env.urls_len; i++) {
        spr_logf_to(logger, lvl, "Url [%zu]: {name: %s, link: %s}", i, env.urls[i]->name, env.urls[i]->link);
    }

    spr_logf_to(logger, lvl, "Build system: {backend: %s}", env.build_system.backend);

    for (size_t i=0; i < env.build_system.reqs_len; i++) {
        spr_logf_to(logger, lvl, "Build system req [%zu]: {%s}", i, env.build_system.reqs[i]);
    }
}
