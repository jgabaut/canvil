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
#ifndef CANVIL_SEMVER_LIST_H_
#define CANVIL_SEMVER_LIST_H_

#include "canvil_core.h"
#include "../koliseo/src/koliseo.h"

#define LIST_CMP_DEFAULT_FN &canvil_tag_cmp

#define LIST_T Canvil_Tag
#define LIST_NAME Canvil_Tag_List
#include "../koliseo/templates/list.h"
#endif // CANVIL_SEMVER_LIST_H_
