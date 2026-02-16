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
#ifndef CANVIL_RECIPE_H_
#define CANVIL_RECIPE_H_

#include "canvil_core.h"
typedef struct Anvil_Recipe {
    char* build;
    char* conf;
    SemVer vers;
} Anvil_Recipe;

#define DARRAY_T Anvil_Recipe*
#define DARRAY_NAME da_recipes
#include "../koliseo/templates/darray.h"

int recipe_sorter(const Anvil_Recipe** a, const Anvil_Recipe** b);
typedef int (*da_recipes_cmp_fn)(const Anvil_Recipe**, const Anvil_Recipe**);
void da_recipes_sort(da_recipes* array, da_recipes_cmp_fn cmp);
bool find_recipe(da_recipes* recipes, SemVer target, Anvil_Recipe* out);
#endif // CANVIL_RECIPE_H_
