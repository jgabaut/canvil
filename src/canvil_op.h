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
#ifndef CANVIL_OP_H_
#define CANVIL_OP_H_

#ifndef _WIN32
#ifndef CANVIL_NOGIT2
#include <git2.h>
#endif // CANVIL_NOGIT2
#endif

#include "canvil_core.h"
#include "canvil_log.h"
#include "canvil_tag_list.h"
#include "canvil_test_list.h"
#include "command.h"
#include "canvil_py_env.h"
#include "canvil_py.h"
#include "canvil_custom.h"

int canvil_handle_make_call(bool use_autoconf, bool no_rebuild, bool use_configure_arg, const char* configure_arg, char** extra_args, size_t extra_args_len, Spuro logger, Koliseo_Temp* kls_t);
bool canvil_op_delete(const char* targetdir_optarg, const char* tagname, const char* bin_optarg, Spuro logger, Koliseo* kls);
bool canvil_op_build(bool git_mode, bool force, bool no_rebuild, bool use_config_arg, const char* config_optarg, const char* minmake_optarg, const char* minautomake_version, const char* cflags_optarg, const char* targetdir_optarg, const char* builds_dir_optarg, const char* tagname, const char* bin_optarg, const char* source_optarg, const char* kern, AnvilPy_Env anvilpy_env, const char* custom_builder, char** extra_args, size_t extra_args_len, Spuro logger, Koliseo* kls);
bool canvil_op_purge(const char* targetdir_optarg, Canvil_Tag_List tag_list, const char* bin_optarg, Spuro logger, Koliseo* kls);
bool canvil_op_init(bool git_mode, bool force, bool no_rebuild, bool use_config_arg, const char* config_optarg, const char* minmake_optarg, const char* minautomake_version, const char* cflags_optarg, const char* targetdir_optarg, Canvil_Tag_List tag_list, const char* builds_dir_optarg, const char* bin_optarg, const char* source_optarg, const char* kern, AnvilPy_Env anvilpy_env, const char* custom_builder, char** extra_args, size_t extra_args_len, Spuro logger, Koliseo* kls);
bool canvil_op_run(const char* targetdir_optarg, const char* tagname, const char* bin_optarg, Spuro logger, Koliseo* kls);
bool canvil_op_test(Anvil_Args* canvil_args, Canvil_Test query, Spuro logger, Koliseo* kls);
int canvil_op_test_macro(Anvil_Args* canvil_args, Canvil_Test_List test_list, Canvil_Test_List errortest_list, Spuro logger, Koliseo* kls);
#endif // CANVIL_OP_H_
