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
#include "canvil_recipe.h"

int recipe_sorter(const Anvil_Recipe** a, const Anvil_Recipe** b)
{
    return -1 * canvil_SemVer_cmp((*a)->vers, (*b)->vers);
}

static da_recipes_cmp_fn da_recipes_sort_cmp;

static int da_recipes_sort_adapter(const void* a, const void* b)
{
    const Anvil_Recipe** lhs = (const Anvil_Recipe**)a;
    const Anvil_Recipe** rhs = (const Anvil_Recipe**)b;
    return da_recipes_sort_cmp(lhs, rhs);
}

void da_recipes_sort(da_recipes* array, da_recipes_cmp_fn cmp)
{
    if (!array || !array->items || array->count <= 1 || !cmp)
        return;

    da_recipes_sort_cmp = cmp;

    qsort(array->items,
          array->count,
          sizeof(Anvil_Recipe*),
          da_recipes_sort_adapter);
}
