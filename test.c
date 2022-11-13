#define NISSE_IMPL
#include "nisse.h"

/*
** You generate a following structure as you need for you application.
** This structure contains enough information for nisse to serialise
*/

nde_t fmt_v3 = nisse_andea(3, nisse_andef(0), nisse_andef(0), nisse_andef(0));
nde_t fmt_item = nisse_andea(1, nisse_tndea("type", 0,));
nde_t fmt_ranged = nisse_andea(2, nisse_andea(1, nisse_andes("damage")),
                                  nisse_andea(1, nisse_andes("range")));
nde_t fmt_weapon = nisse_andea(1, nisse_tndea("damage", 0,));

nde_t nde_arr[] = {
        {
                .type = NISSE_TYPE_ARRAY,
                .new_line_at_end_of_subsequent_elements = 1,
                .nde = (nde_t[]){
                        nisse_andes("player"),
                        nisse_andei(-100), // just here for example purposes
                        nisse_tndef("hp", 2.0f),
                        {
                                .type = NISSE_TYPE_ARRAY,
                                .new_line_at_end_of_subsequent_elements = 1,
                                .new_line_at_start = 1,
                                .nde = (nde_t[]){
                                        nisse_andes("items"),
                                        nisse_tndea("sword", 2,
                                                   nisse_tndes("type", "weapon"),
                                                   nisse_tndei("damage", 10)
                                                ),
                                        nisse_tndea("knife", 2,
                                                    nisse_tndes("type", "weapon"),
                                                    nisse_tndei("damage", 4)
                                                ),
                                        nisse_tndea("bow", 3,
                                                   nisse_tndes("type", "ranged"),
                                                   nisse_tndei("damage", 10),
                                                   nisse_tndei("range", 15)
                                                ),
                                        nisse_tndeanl("crossbow", 4,
                                                     nisse_tndes("type", "ranged"),
                                                     nisse_tndei("damage", 16),
                                                     nisse_tndei("range", 25),
                                                     nisse_tndea("properties", 2,
                                                                nisse_andes("poison"),
                                                                nisse_andes("fire")
                                                             ),
                                                ),
                                        nisse_tndea("sling", 3,
                                                   nisse_tndes("type", "ranged"),
                                                   nisse_tndei("damage", 4),
                                                   nisse_tndei("range", 3)
                                                ),
                                },
                                .nde_len = 6,
                        }
                },
                .nde_len = 4,
        },
        {
                .type = NISSE_TYPE_ARRAY,
                .new_line_at_end_of_subsequent_elements = 1,
                .nde = (nde_t[]){
                        nisse_andes("test-array"),
                        nisse_andea(1, nisse_andes("test")),
                        nisse_andea(0, ),
                        nisse_andea(1, nisse_andea(0, )),
                        nisse_tndea("sword", 1, nisse_tndei("damage", 10)),
                        nisse_andea(3, nisse_andes("test 1"), nisse_andes("2 test"), nisse_andes("test3")),
                },
                .nde_len = 6,
        },
        nisse_tndeanl("v3 array", 4,
                    nisse_andea(3,
                                nisse_andef(1.0f),
                                nisse_andef(2.0f),
                                nisse_andef(3.0f),
                            ),
                    nisse_andea(3,
                                nisse_andef(4.0f),
                                nisse_andef(5.0f),
                                nisse_andef(6.0f),
                            ),
                    nisse_andea(3,
                                nisse_andef(7.0f),
                                nisse_andef(8.0f),
                                nisse_andef(9.0f),
                            ),
                    nisse_andea(3,
                                nisse_andef(10.0f),
                                nisse_andef(0.00001f),
                                nisse_andef(420.69f),
                            ),
                ),
};

#define ARR_SZ(__ARR) sizeof(__ARR) / sizeof(__ARR[0])

int main()
{
        nde_t test = {.type = NISSE_TYPE_ARRAY, .nde_len = ARR_SZ(nde_arr), .nde = nde_arr};
#if 1
        nisse_write_to_file("./test.nisse", test);
#endif
        nde_t nde = nisse_parse_file("./test.nisse");

#if 1
        nde_t* player_true = nisse_nde_get_index(&test, 0);
        nde_t* player_parsed = nisse_nde_get_index(&nde, 0);
        assert(player_true);
        assert(player_parsed);

        printf("test->player has some of player? %s.\n", nisse_nde_fits_format(player_parsed, player_true) ? "yes" : "no");
        printf("test->player has at least all of player? %s.\n", nisse_nde_fits_format(player_true, player_parsed) ? "yes" : "no");

        // check if items are correct

        nde_t* items_parsed = nisse_nde_get_tagged(player_parsed, "items");
        assert(items_parsed && items_parsed->type == NISSE_TYPE_ARRAY);

        for (int i = 1; i < items_parsed->nde_len; i++) {
                if (!nisse_nde_fits_format(&fmt_item, items_parsed->nde + i)) {
                        printf("item does not define a type %s:%d\n", items_parsed->nde[i].nde->type == NISSE_TYPE_STRING ? items_parsed->nde[i].nde->str : NULL, i);
                        continue;
                }

                nde_t* type_parsed = nisse_nde_get_tagged(items_parsed->nde + i, "type");
                if (!type_parsed || type_parsed->type != NISSE_TYPE_ARRAY || type_parsed->nde_len != 2 || type_parsed->nde[1].type != NISSE_TYPE_STRING) {
                        printf("invalid item, wrong type value %s:%d\n", items_parsed->nde[i].nde->type == NISSE_TYPE_STRING ? items_parsed->nde[i].nde->str : NULL, i);
                } else {
                        const char* item_type = nisse_nde_get_value(type_parsed, NULL)->str;
                        if (strcmp(item_type, "weapon") == 0 && !nisse_nde_fits_format(&fmt_weapon, items_parsed->nde + i))
                                printf("invalid weapon %s:%d\n", items_parsed->nde[i].nde->type == NISSE_TYPE_STRING ? items_parsed->nde[i].nde->str : NULL, i);
                        else if (strcmp(item_type, "ranged") == 0 && !nisse_nde_fits_format(&fmt_ranged, items_parsed->nde + i))
                                printf("invalid ranged %s:%d\n", items_parsed->nde[i].nde->type == NISSE_TYPE_STRING ? items_parsed->nde[i].nde->str : NULL, i);
                }
        }
#endif


        nde_t* v3a = nisse_nde_get_tagged(&nde, "v3 array");
        assert(v3a);
        for (int i = 1; i < v3a->nde_len; i++)
                if (!nisse_nde_fits_format(v3a->nde+ i, &fmt_v3))
                        printf("infalid v3 array %d!\n", i);

        nisse_write_to_file("./test2.nisse", nde);
        nisse_free_nde(&nde);
}
