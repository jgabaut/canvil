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
#ifndef CANVIL_ENV_H_
#define CANVIL_ENV_H_

#include "canvil_tag_list.h"
#include "canvil_test_list.h"

typedef struct Anvil_Env {
    Canvil_Tag_List base_tags;
    Canvil_Tag_List git_tags;
    Canvil_Test_List tests;
    Canvil_Test_List errortests;
} Anvil_Env;

void canvil_print_base_tags(Anvil_Env anvil_env); /**< Prints base tags in an Anvil_Env.*/
void canvil_print_git_tags(Anvil_Env anvil_env); /**< Prints git tags in an Anvil_Env.*/
void canvil_print_tags(Anvil_Env anvil_env); /**< Prints all tags in an Anvil_Env.*/

#endif // CANVIL_ENV_H_
