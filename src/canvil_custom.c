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
#include "canvil_custom.h"

int canvil_custom_handle_build(const char* custom_builder, const char* target_dir, const char* builds_dir_optarg, const char* bin, const char* tag, char** extra_args, size_t extra_args_len, Spuro logger, Koliseo_Temp* k_tmp) {
    char target_tag_path[FILENAME_MAX] = {0};
    sprintf(target_tag_path, "%s/v%s", target_dir, tag);
    const char** cmd_args = KLS_PUSH_ARR_T(k_tmp, const char*, 7+extra_args_len);
    if (extra_args_len > 0) {
        spr_logf_to(logger, SPR_INFO, "Extra args:");
        for (size_t i=0; i < extra_args_len; i++) {
            spr_logf_to(logger, SPR_INFO, "{%s}", extra_args[i]);
        }
    }
    cmd_args[0] = custom_builder;
    cmd_args[1] = target_tag_path;
    cmd_args[2] = builds_dir_optarg;
    cmd_args[3] = bin;
    cmd_args[4] = tag;
    cmd_args[5] = ".";
    for (size_t i=6; i<extra_args_len+6; i++) cmd_args[i] = extra_args[i-6];
    cmd_args[extra_args_len+6] = NULL;

    Komando cmd = new_command_kls_t(6+extra_args_len, cmd_args, k_tmp);
    bool run_res = run_command(cmd);
    if (run_res) return 0;
    return -1;
}
