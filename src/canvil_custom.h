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
#ifndef CANVIL_CUSTOM_H_
#define CANVIL_CUSTOM_H_
#include "canvil_core.h"
#include "command.h"
int canvil_custom_handle_build(const char* custom_builder, const char* target_dir, const char* builds_dir_optarg, const char* bin, const char* tag, char** extra_args, size_t extra_args_len, Spuro logger, Koliseo_Temp* k_tmp);
#endif // CANVIL_CUSTOM_H_
