#define NISSE_IMPL
#include "nisse.h"

/*
** You generate a following structure as you need for you application.
** This structure contains enough information for nisse to serialise
*/

nde_t fmt_v3 = nisse_andea(3, nisse_andef(0), nisse_andef(0), nisse_andef(0));

nde_t ndes[] = {
        {
                .type = NISSE_TYPE_ARRAY,
                .new_line_at_end_of_subsequent_elements = 1,
                .nde = (nde_t[]){
                        nisse_andes("player"),
                        nisse_andei(-100),
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
                                        nisse_tndea("knife", 1, nisse_tndei("damage", 4)),
                                        nisse_tndea("bow", 3,
                                                   nisse_tndes("type", "weapon"),
                                                   nisse_tndei("damage", 10),
                                                   nisse_tndei("range", 15)
                                                ),
                                        nisse_tndeanl("crossbow", 4,
                                                     nisse_tndes("type", "ranged weapon"),
                                                     nisse_tndei("damage", 16),
                                                     nisse_tndei("range", 25),
                                                     nisse_tndea("properties", 2,
                                                                nisse_andes("poison"),
                                                                nisse_andes("fire")
                                                             ),
                                                ),
                                        nisse_tndea("sling", 2,
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
                        nisse_andes("items"),
                        nisse_andea(1, nisse_andes("test")),
                        nisse_andea(0, ),
                        nisse_andea(1, nisse_andea(0, )),
                        nisse_tndea("sword", 1, nisse_tndei("damage", 10)),
                        nisse_tndea("knife", 1, nisse_tndei("damage", 4)),
                        nisse_tndea("bow", 2,
                                   nisse_tndei("damage", 10),
                                   nisse_tndei("range", 15)
                                ),
                },
                .nde_len = 5,
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
        nde_t test = {.type = NISSE_TYPE_ARRAY, .nde_len = ARR_SZ(ndes), .nde = ndes};
        nisse_write_to_file("./test.nisse", test);
        nde_t nde = nisse_parse_file("./test.nisse");

        printf("does test fit? %s.\n", nisse_nde_fits_format(nisse_nde_get_index(&nde, 0), *nisse_nde_get_index(&test, 0)) ? "yes" : "no");

        nde_t* v3a = nisse_nde_get_tagged(&nde, "v3 array");
        assert(v3a);
        for (int i = 1; i < v3a->nde_len; i++)
                if (!nisse_nde_fits_format(v3a->nde+ i, fmt_v3))
                        printf("infalid v3 array %d!\n", i);

        nisse_write_to_file("./test2.nisse", nde);
        nisse_free_nde(&nde);
}
