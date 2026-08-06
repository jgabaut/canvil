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

bool da_recipes_validate(da_recipes* array, Spuro logger)
{
    if (!array) return false;
    for (size_t i=0; i < array->count; i++) {
        Anvil_Recipe* r = array->items[i];
        if (!r) return false;
        if (!r->build) {
            spr_logf_to(logger, SPR_ERROR, "Failed checking anvil_recipe build value {%i}", i);
            return false;
        }
        if (strlen(r->build) == 0) {
            spr_logf_to(logger, SPR_ERROR, "Failed checking anvil_recipe build value {%i}", i);
            return false;
        }
        if (!r->vers) {
            spr_logf_to(logger, SPR_ERROR, "Failed checking anvil_recipe vers value {%i}", i);
            return false;
        }
    }

    return true;
}

int recipe_sorter(const Anvil_Recipe** a, const Anvil_Recipe** b)
{
    return -1 * canvil_SemVer_cmp(*((*a)->vers), *((*b)->vers));
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

bool find_recipe(da_recipes* recipes, SemVer target, Anvil_Recipe* out)
{
    if (!recipes || !out) return false;
    for (int i = 0; i < recipes->count; i++) {
        Anvil_Recipe* r = recipes->items[i];
        if (canvil_SemVer_cmp(*(out->vers), *(r->vers)) >= 0) {
            *out = *r;
            return true;
        }
    }
    return false;
}
