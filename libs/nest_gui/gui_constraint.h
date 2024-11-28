#ifndef GUI_CONSTRAINTS
#define GUI_CONSTRAINTS

#include "nest.h"
#include "gui_position.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct _GUI_FillBetweenXData {
    GUI_XSidePos left;
    GUI_XSidePos right;
    bool pad_l, pad_r;
} GUI_FillBetweenXData;

typedef struct _GUI_FillBetweenYData {
    GUI_YSidePos top;
    GUI_YSidePos bottom;
    bool pad_t, pad_b;
} GUI_FillBetweenYData;

typedef union _GUI_ConstraintData {
    void *_any;
    GUI_FillBetweenXData *fill_between_x;
    GUI_FillBetweenYData *fill_between_y;
} GUI_ConstraintData;

typedef void (*GUI_ConstraintDataDstr)(void *);
typedef void (*GUI_ConstraintDataTrav)(void *);

struct _GUI_Constraint;
typedef bool (*GUI_ConstraintUpdate)(struct _GUI_Constraint *);

typedef struct _GUI_Constraint {
    Nst_OBJ_HEAD;
    Nst_GGC_HEAD;
    GUI_Element *element;
    GUI_ConstraintData data;
    GUI_ConstraintDataDstr data_dstr;
    GUI_ConstraintDataTrav data_trav;
    GUI_ConstraintUpdate update;
} GUI_Constraint;

void GUI_Constraint_Destroy(GUI_Constraint *constr);
void GUI_Constraint_Traverse(GUI_Constraint *constr);

bool GUI_Constraint_Update(GUI_Constraint *constr);

bool GUI_Element_AddConstraintBefore(GUI_Constraint *constr);
bool GUI_Element_AddConstraintAfter(GUI_Constraint *constr);

GUI_Constraint *GUI_FillBetweenX_New(GUI_Element *element,
                                     GUI_XSidePos left, GUI_XSidePos right,
                                     bool pad_l, bool pad_r);

GUI_Constraint *GUI_FillBetweenY_New(GUI_Element *element,
                                     GUI_YSidePos top, GUI_YSidePos bottom,
                                     bool pad_t, bool pad_b);

GUI_Constraint *GUI_MatchWindowSize_New(GUI_Element *element);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !GUI_CONSTRAINTS
