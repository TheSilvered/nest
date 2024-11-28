#ifndef GUI_POSITION_H
#define GUI_POSITION_H

#include "gui_element.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef enum _GUI_Point {
    GUI_TL   = (0<<3) | 0, GUI_TC   = (1<<3) | 0, GUI_TR   = (2<<3) | 0,
    GUI_ML   = (0<<3) | 1, GUI_MC   = (1<<3) | 1, GUI_MR   = (2<<3) | 1,
    GUI_BL   = (0<<3) | 2, GUI_BC   = (1<<3) | 2, GUI_BR   = (2<<3) | 2,

    GUI_TLP  = (3<<3) | 0, GUI_TCP  = (4<<3) | 0, GUI_TRP  = (5<<3) | 0,
    GUI_MLP  = (3<<3) | 1, GUI_MCP  = (4<<3) | 1, GUI_MRP  = (5<<3) | 1,
    GUI_BLP  = (3<<3) | 2, GUI_BCP  = (4<<3) | 2, GUI_BRP  = (5<<3) | 2,

    GUI_TPL  = (0<<3) | 3, GUI_TPC  = (1<<3) | 3, GUI_TPR  = (2<<3) | 3,
    GUI_MPL  = (0<<3) | 4, GUI_MPC  = (1<<3) | 4, GUI_MPR  = (2<<3) | 4,
    GUI_BPL  = (0<<3) | 5, GUI_BPC  = (1<<3) | 5, GUI_BPR  = (2<<3) | 5,

    GUI_TPLP = (3<<3) | 3, GUI_TPCP = (4<<3) | 3, GUI_TPRP = (5<<3) | 3,
    GUI_MPLP = (3<<3) | 4, GUI_MPCP = (4<<3) | 4, GUI_MPRP = (5<<3) | 4,
    GUI_BPLP = (3<<3) | 5, GUI_BPCP = (4<<3) | 5, GUI_BPRP = (5<<3) | 5
} GUI_Point;

typedef enum _GUI_XSide {
    GUI_L  = 0, GUI_C  = 1, GUI_R  = 2,
    GUI_LP = 3, GUI_CP = 4, GUI_RP = 5
} GUI_XSide;

typedef enum _GUI_YSide {
    GUI_T  = 0, GUI_M  = 1, GUI_B  = 2,
    GUI_TP = 3, GUI_MP = 4, GUI_BP = 5
} GUI_YSide;

typedef struct _GUI_PointPos {
    union {
        struct { int x, y; } abs;
        struct {
            GUI_Element *element;
            GUI_Point point;
        } rel;
    };
    bool is_abs;
} GUI_PointPos;

typedef struct _GUI_XSidePos {
    union {
        struct { int x; } abs;
        struct {
            GUI_Element *element;
            GUI_XSide x_side;
        } rel;
    };
    bool is_abs;
} GUI_XSidePos;

typedef struct _GUI_YSidePos {
    union {
        struct { int y; } abs;
        struct {
            GUI_Element *element;
            GUI_YSide y_side;
        } rel;
    };
    bool is_abs;
} GUI_YSidePos;

void GUI_PointToSides(GUI_Point point, GUI_XSide *x_side, GUI_YSide *y_side);
GUI_Point GUI_SidesToPoint(GUI_XSide x_side, GUI_YSide y_side);

int GUI_Element_GetXSide(GUI_Element *element, GUI_XSide x_side);
int GUI_Element_GetYSide(GUI_Element *element, GUI_YSide y_side);
void GUI_Element_SetXSide(GUI_Element *element, GUI_XSide x_side, int value);
void GUI_Element_SetYSide(GUI_Element *element, GUI_YSide y_side, int value);

void GUI_Element_GetPoint(GUI_Element *element, GUI_Point point,
                          int *x, int *y);
void GUI_Element_SetPoint(GUI_Element *element, GUI_Point point,
                          int x, int y);

void GUI_PointPos_InitAbs(GUI_PointPos *pos, int x, int y);
void GUI_PointPos_InitRel(GUI_PointPos *pos, GUI_Element *element,
                          GUI_Point point);
void GUI_PointPos_Destroy(GUI_PointPos *pos);
void GUI_PointPos_Traverse(GUI_PointPos *pos);
void GUI_PointPos_Value(GUI_PointPos *pos, int *x, int *y);

void GUI_XSidePos_InitAbs(GUI_XSidePos *pos, int x);
void GUI_XSidePos_InitRel(GUI_XSidePos *pos, GUI_Element *element,
                          GUI_XSide x_side);
void GUI_XSidePos_Destroy(GUI_XSidePos *pos);
void GUI_XSidePos_Traverse(GUI_XSidePos *pos);
int GUI_XSidePos_Value(GUI_XSidePos *pos);

void GUI_YSidePos_InitAbs(GUI_YSidePos *pos, int y);
void GUI_YSidePos_InitRel(GUI_YSidePos *pos, GUI_Element *element,
                          GUI_YSide y_side);
void GUI_YSidePos_Destroy(GUI_YSidePos *pos);
void GUI_YSidePos_Traverse(GUI_YSidePos *pos);
int GUI_YSidePos_Value(GUI_YSidePos *pos);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !GUI_POSITION_H
