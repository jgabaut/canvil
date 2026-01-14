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
#ifndef CANVIL_H_
#define CANVIL_H_
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>

#ifndef _WIN32
#ifndef CANVIL_NOGIT2
#include <git2.h>
#endif // CANVIL_NOGIT2
#endif

#include "canvil_core.h"
#include "canvil_op.h"
#include "canvil_toml.h"

int canvil_main(int argc, char** argv, Koliseo* default_kls);
int canvil_check_passed_args(Anvil_Args* canvil_args, Anvil_Env* canvil_env, AnvilPy_Env canvil_py_env, char** argv, size_t argc, Spuro logger, Koliseo* kls);
bool canvil_gen_header(const char* target_dir, const char* anvil_kern, const char* tag, const char* bin_name, Spuro logger, Koliseo* kls);
bool canvil_init_project(const char* target_name, const char* anvil_kern, const char* template_name, Spuro logger, Koliseo* kls);
void to_uppercase_copy(const char *src, char *dest, size_t dest_size);
#endif // CANVIL_H_
