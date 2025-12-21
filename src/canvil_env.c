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
#include "canvil_env.h"

void canvil_print_base_tags(Anvil_Env anvil_env)
{
    Canvil_Tag_List base_tagl = anvil_env.base_tags;

    while (! Canvil_Tag_List_isEmpty(base_tagl)) {
        Canvil_Tag* node_pt = Canvil_Tag_List_head(base_tagl);
        SemVer node_value = *(node_pt->version);
        printf("Base Tag: {" SemVer_Fmt "}\n", SemVer_Arg(node_value));
        base_tagl = Canvil_Tag_List_tail(base_tagl);
    }
}

void canvil_print_git_tags(Anvil_Env anvil_env)
{
    Canvil_Tag_List git_tagl = anvil_env.git_tags;

    while (! Canvil_Tag_List_isEmpty(git_tagl)) {
        Canvil_Tag* node_pt = Canvil_Tag_List_head(git_tagl);
        SemVer node_value = *(node_pt->version);
        printf("Base Tag: {" SemVer_Fmt "}\n", SemVer_Arg(node_value));
        git_tagl = Canvil_Tag_List_tail(git_tagl);
    }
}

void canvil_print_tags(Anvil_Env anvil_env)
{
    canvil_print_base_tags(anvil_env);
    canvil_print_git_tags(anvil_env);
}
