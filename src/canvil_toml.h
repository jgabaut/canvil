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
#ifndef CANVIL_TOML_H_
#define CANVIL_TOML_H_
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../tomlc17/src/tomlc17.h"
#include "../spuro/src/spuro.h"
#include "canvil_core.h"
#include "canvil_env.h"
#include "canvil_py_env.h"

bool try_check_key_strvalue(const char* key, toml_datum_t table, const char* table_name, toml_datum_t* buf);
void check_table_innerkey(const char* target_key, toml_datum_t table, const char* current_key, const char* table_name, Spuro logger);
const char* get_table_stringval(toml_datum_t table, const char* key, const char* table_name, Spuro logger);
bool check_filepath_as_stego(const char* filepath, Spuro logger);
bool lex_filepath_as_stego(const char* stego_path, Spuro logger);
bool getargs_from_filepath_as_stego(Anvil_Args* canvil_args, const char* filepath, Koliseo* kls, bool collect_anvil_env, Anvil_Env* canvil_env, Spuro logger);
bool getargs_from_filepath_as_stego_global(Anvil_Args* canvil_args, const char* filepath, Koliseo* kls, Spuro logger);
bool lint_stegopath(const char* stego_path, Canvil_Lint_Mode mode, Spuro logger);
bool anvilpy_getenv_from_filepath(const char* filepath, AnvilPy_Env* canvil_py_env, Koliseo* kls, Spuro logger);
bool lex_anvilpy_from_filepath(const char* filepath, Spuro logger);

#endif // CANVIL_TOML_H_
