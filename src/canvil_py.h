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
#ifndef CANVIL_PY_H_
#define CANVIL_PY_H_
#include <archive.h>
#include <archive_entry.h>
#include "canvil_log.h"
#include "canvil_core.h"
#include "command.h"
#include "canvil_py_env.h"

void canvil_py_subst_dashes(const char* src, char* dest);
int canvil_py_handle_build(Spuro logger, const char* builds_dir_optarg, Koliseo_Temp* kls_t);
int canvil_py_handle_unpack(const char* srcdist_path, const char* bindir_optarg, const char* tagname, Spuro logger);
int canvil_py_gen_shim(const char* shim_path, const char* module_path, const char* entrypoint, Spuro logger);
bool canvil_py_handle_postbuild(AnvilPy_Env anvilpy_env, const char* targetdir_optarg, const char* tagname, Spuro logger);
#endif // CANVIL_PY_H_
