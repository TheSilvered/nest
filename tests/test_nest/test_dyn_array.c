#include "tests.h"

static i32 last_num_destroyed = -1;

static void int_dstr(i32 *num) {
    last_num_destroyed = *num;
}

TestResult test_da_init(void)
{
    ENTER_TEST;

    Nst_DynArray arr;
    fail_if(!Nst_da_init(&arr, sizeof(i32), 0));
    fail_if(arr.len != 0);
    fail_if(arr.cap != 0);
    fail_if(arr.unit_size != sizeof(i32));
    fail_if(arr.data != NULL);

    fail_if(!Nst_da_init(&arr, sizeof(i32), 20));
    fail_if(arr.len != 0);
    fail_if(arr.cap < 20);
    fail_if(arr.unit_size != sizeof(i32));

    Nst_da_clear(&arr, NULL);

    EXIT_TEST;
}

TestResult test_da_init_copy(void)
{
    ENTER_TEST;

    Nst_DynArray src;
    fail_if(!Nst_da_init(&src, sizeof(i32), 5));
    for (i32 i = 0; i < 5; i++)
        Nst_da_append(&src, &i);

    Nst_DynArray dst;
    fail_if(!Nst_da_init_copy(&src, &dst));
    fail_if(dst.len != src.len);
    fail_if(dst.unit_size != src.unit_size);
    for (i32 i = 0; i < 5; i++) {
        i32 num1 = *(i32 *)Nst_da_get(&src, i);
        i32 num2 = *(i32 *)Nst_da_get(&dst, i);
        fail_if(num1 != num2);
    }

    Nst_da_clear(&src, NULL);
    Nst_da_clear(&dst, NULL);

    EXIT_TEST;
}

TestResult test_da_reserve(void)
{
    ENTER_TEST;

    Nst_DynArray arr;
    fail_if(!Nst_da_init(&arr, sizeof(i32), 5));
    fail_if(arr.cap < 5);
    fail_if(!Nst_da_reserve(&arr, 10));
    fail_if(arr.cap < 15);
    Nst_da_clear(&arr, NULL);

    EXIT_TEST;
}

TestResult test_da_append(void)
{
    ENTER_TEST;

    Nst_DynArray arr;
    i32 num = 1;
    fail_if(!Nst_da_init(&arr, sizeof(i32), 5));
    fail_if(arr.cap < 5);
    fail_if(arr.len != 0);
    fail_if(!Nst_da_append(&arr, &num));
    fail_if(arr.len != 1);
    fail_if(*(i32 *)Nst_da_get(&arr, 0) != 1);
    num = 2;
    fail_if(!Nst_da_append(&arr, &num));
    fail_if(arr.len != 2);
    fail_if(*(i32 *)Nst_da_get(&arr, 0) != 1);
    fail_if(*(i32 *)Nst_da_get(&arr, 1) != 2);
    num = 3;
    fail_if(!Nst_da_append(&arr, &num));
    fail_if(arr.len != 3);
    fail_if(*(i32 *)Nst_da_get(&arr, 0) != 1);
    fail_if(*(i32 *)Nst_da_get(&arr, 1) != 2);
    fail_if(*(i32 *)Nst_da_get(&arr, 2) != 3);

    Nst_da_clear(&arr, NULL);

    EXIT_TEST;
}

TestResult test_da_remove_swap(void)
{
    ENTER_TEST;

    Nst_DynArray arr;
    fail_if(!Nst_da_init(&arr, sizeof(i32), 4));
    for (i32 i = 0; i < 4; i++)
        Nst_da_append(&arr, &i);
    fail_if(Nst_da_remove_swap(&arr, 10, NULL));
    fail_if(!Nst_da_remove_swap(&arr, 1, NULL));
    fail_if(arr.len != 3);
    fail_if(*(i32 *)Nst_da_get(&arr, 0) != 0);
    fail_if(*(i32 *)Nst_da_get(&arr, 1) != 3);
    fail_if(*(i32 *)Nst_da_get(&arr, 2) != 2);
    fail_if(!Nst_da_remove_swap(&arr, 1, (Nst_Destructor)int_dstr));
    fail_if(arr.len != 2);
    fail_if(last_num_destroyed != 3);
    fail_if(*(i32 *)Nst_da_get(&arr, 0) != 0);
    fail_if(*(i32 *)Nst_da_get(&arr, 1) != 2);

    Nst_da_clear(&arr, NULL);

    EXIT_TEST;
}

TestResult test_da_remove_shift(void)
{
    ENTER_TEST;

    Nst_DynArray arr;
    fail_if(!Nst_da_init(&arr, sizeof(i32), 4));
    for (i32 i = 0; i < 4; i++)
        Nst_da_append(&arr, &i);
    fail_if(Nst_da_remove_shift(&arr, 10, NULL));
    fail_if(!Nst_da_remove_shift(&arr, 1, NULL));
    fail_if(arr.len != 3);
    fail_if(*(i32 *)Nst_da_get(&arr, 0) != 0);
    fail_if(*(i32 *)Nst_da_get(&arr, 1) != 2);
    fail_if(*(i32 *)Nst_da_get(&arr, 2) != 3);
    fail_if(!Nst_da_remove_shift(&arr, 1, (Nst_Destructor)int_dstr));
    fail_if(arr.len != 2);
    fail_if(last_num_destroyed != 2);
    fail_if(*(i32 *)Nst_da_get(&arr, 0) != 0);
    fail_if(*(i32 *)Nst_da_get(&arr, 1) != 3);

    Nst_da_clear(&arr, NULL);

    EXIT_TEST;
}

TestResult test_da_get(void)
{
    ENTER_TEST;

    Nst_DynArray arr;
    fail_if(!Nst_da_init(&arr, sizeof(i32), 5));
    for (i32 i = 0; i < 5; i++)
        Nst_da_append(&arr, &i);
    for (i32 i = 0; i < 5; i++)
        fail_if(*(i32 *)Nst_da_get(&arr, i) != i);
    Nst_da_clear(&arr, NULL);

    EXIT_TEST;
}

TestResult test_da_set(void)
{
    ENTER_TEST;

    last_num_destroyed = -1;

    Nst_DynArray arr;
    i32 num = 10;
    fail_if(!Nst_da_init(&arr, sizeof(i32), 5));
    for (i32 i = 0; i < 5; i++)
        Nst_da_append(&arr, &i);
    Nst_da_set(&arr, 10, &num, (Nst_Destructor)int_dstr);
    fail_if(last_num_destroyed != -1);
    Nst_da_set(&arr, 2, &num, NULL);
    fail_if(*(i32 *)Nst_da_get(&arr, 2) != 10);
    Nst_da_set(&arr, 3, &num, (Nst_Destructor)int_dstr);
    fail_if(*(i32 *)Nst_da_get(&arr, 3) != 10);
    fail_if(last_num_destroyed != 3);
    Nst_da_clear(&arr, NULL);

    EXIT_TEST;
}

TestResult test_da_clear(void)
{
    ENTER_TEST;

    Nst_DynArray arr;
    fail_if(!Nst_da_init(&arr, sizeof(i32), 5));
    for (i32 i = 0; i < 5; i++)
        Nst_da_append(&arr, &i);
    Nst_da_clear(&arr, (Nst_Destructor)int_dstr);
    fail_if(arr.unit_size != sizeof(i32));
    fail_if(arr.data != NULL);
    fail_if(arr.cap != 0);
    fail_if(arr.len != 0);
    fail_if(last_num_destroyed != 4);

    EXIT_TEST;
}

TestResult test_pa_init(void)
{
    ENTER_TEST;

    Nst_PtrArray arr;
    fail_if(!Nst_pa_init(&arr, 0));
    fail_if(arr.len != 0);
    fail_if(arr.cap != 0);
    fail_if(arr.data != NULL);

    fail_if(!Nst_pa_init(&arr, 20));
    fail_if(arr.len != 0);
    fail_if(arr.cap < 20);

    Nst_pa_clear(&arr, NULL);

    EXIT_TEST;
}

TestResult test_pa_init_copy(void)
{
    ENTER_TEST;

    i32 num0 = 0, num1 = 1, num2 = 2;

    Nst_PtrArray src;
    fail_if(!Nst_pa_init(&src, 3));
    Nst_pa_append(&src, &num0);
    Nst_pa_append(&src, &num1);
    Nst_pa_append(&src, &num2);

    Nst_PtrArray dst;
    fail_if(!Nst_pa_init_copy(&src, &dst));
    fail_if(dst.len != src.len);
    for (i32 i = 0; i < 5; i++) {
        i32 *p0 = (i32 *)Nst_pa_get(&src, i);
        i32 *p1 = (i32 *)Nst_pa_get(&dst, i);
        fail_if(p0 != p1);
    }

    Nst_pa_clear(&src, NULL);
    Nst_pa_clear(&dst, NULL);

    EXIT_TEST;
}

TestResult test_pa_reserve(void)
{
    ENTER_TEST;

    Nst_PtrArray arr;
    fail_if(!Nst_pa_init(&arr, 5));
    fail_if(arr.cap < 5);
    fail_if(!Nst_pa_reserve(&arr, 10));
    fail_if(arr.cap < 15);
    Nst_pa_clear(&arr, NULL);

    EXIT_TEST;
}

TestResult test_pa_append(void)
{
    ENTER_TEST;

    i32 num0 = 0, num1 = 1, num2 = 2;
    Nst_PtrArray arr;
    fail_if(!Nst_pa_init(&arr, 5));
    fail_if(arr.cap < 5);
    fail_if(arr.len != 0);
    fail_if(!Nst_pa_append(&arr, &num0));
    fail_if(arr.len != 1);
    fail_if((i32 *)Nst_pa_get(&arr, 0) != &num0);
    fail_if(!Nst_pa_append(&arr, &num1));
    fail_if(arr.len != 2);
    fail_if((i32 *)Nst_pa_get(&arr, 0) != &num0);
    fail_if((i32 *)Nst_pa_get(&arr, 1) != &num1);
    fail_if(!Nst_pa_append(&arr, &num2));
    fail_if(arr.len != 3);
    fail_if((i32 *)Nst_pa_get(&arr, 0) != &num0);
    fail_if((i32 *)Nst_pa_get(&arr, 1) != &num1);
    fail_if((i32 *)Nst_pa_get(&arr, 2) != &num2);

    Nst_pa_clear(&arr, NULL);

    EXIT_TEST;
}

TestResult test_pa_remove_swap(void)
{
    ENTER_TEST;

    i32 num0 = 0, num1 = 1, num2 = 2, num3 = 3;
    Nst_PtrArray arr;
    fail_if(!Nst_pa_init(&arr, 4));
    Nst_pa_append(&arr, &num0);
    Nst_pa_append(&arr, &num1);
    Nst_pa_append(&arr, &num2);
    Nst_pa_append(&arr, &num3);
    fail_if(Nst_pa_remove_swap(&arr, 10, NULL));
    fail_if(!Nst_pa_remove_swap(&arr, 1, NULL));
    fail_if(arr.len != 3);
    fail_if((i32 *)Nst_pa_get(&arr, 0) != &num0);
    fail_if((i32 *)Nst_pa_get(&arr, 1) != &num3);
    fail_if((i32 *)Nst_pa_get(&arr, 2) != &num2);
    fail_if(!Nst_pa_remove_swap(&arr, 1, (Nst_Destructor)int_dstr));
    fail_if(arr.len != 2);
    fail_if(last_num_destroyed != 3);
    fail_if((i32 *)Nst_pa_get(&arr, 0) != &num0);
    fail_if((i32 *)Nst_pa_get(&arr, 1) != &num2);

    Nst_pa_clear(&arr, NULL);

    EXIT_TEST;
}

TestResult test_pa_remove_shift(void)
{
    ENTER_TEST;

    i32 num0 = 0, num1 = 1, num2 = 2, num3 = 3;
    Nst_PtrArray arr;
    fail_if(!Nst_pa_init(&arr, 4));
    Nst_pa_append(&arr, &num0);
    Nst_pa_append(&arr, &num1);
    Nst_pa_append(&arr, &num2);
    Nst_pa_append(&arr, &num3);
    fail_if(Nst_pa_remove_shift(&arr, 10, NULL));
    fail_if(!Nst_pa_remove_shift(&arr, 1, NULL));
    fail_if(arr.len != 3);
    fail_if((i32 *)Nst_pa_get(&arr, 0) != &num0);
    fail_if((i32 *)Nst_pa_get(&arr, 1) != &num2);
    fail_if((i32 *)Nst_pa_get(&arr, 2) != &num3);
    fail_if(!Nst_pa_remove_shift(&arr, 1, (Nst_Destructor)int_dstr));
    fail_if(arr.len != 2);
    fail_if(last_num_destroyed != 2);
    fail_if((i32 *)Nst_pa_get(&arr, 0) != &num0);
    fail_if((i32 *)Nst_pa_get(&arr, 1) != &num3);

    Nst_pa_clear(&arr, NULL);

    EXIT_TEST;
}

TestResult test_pa_get(void)
{
    ENTER_TEST;

    i32 num0 = 0, num1 = 1, num2 = 2, num3 = 3;
    Nst_PtrArray arr;
    fail_if(!Nst_pa_init(&arr, 4));
    Nst_pa_append(&arr, &num0);
    Nst_pa_append(&arr, &num1);
    Nst_pa_append(&arr, &num2);
    Nst_pa_append(&arr, &num3);
    fail_if((i32 *)Nst_pa_get(&arr, 0) != &num0);
    fail_if((i32 *)Nst_pa_get(&arr, 1) != &num1);
    fail_if((i32 *)Nst_pa_get(&arr, 2) != &num2);
    fail_if((i32 *)Nst_pa_get(&arr, 3) != &num3);
    Nst_pa_clear(&arr, NULL);

    EXIT_TEST;
}

TestResult test_pa_set(void)
{
    ENTER_TEST;

    last_num_destroyed = -1;

    Nst_PtrArray arr;
    i32 num0 = 2;
    i32 num = 10;
    fail_if(!Nst_pa_init(&arr, 5));
    for (i32 i = 0; i < 5; i++)
        Nst_pa_append(&arr, &num0);
    Nst_pa_set(&arr, 10, &num, (Nst_Destructor)int_dstr);
    fail_if(last_num_destroyed != -1);
    Nst_pa_set(&arr, 2, &num, NULL);
    fail_if((i32 *)Nst_pa_get(&arr, 2) != &num);
    Nst_pa_set(&arr, 3, &num, (Nst_Destructor)int_dstr);
    fail_if((i32 *)Nst_pa_get(&arr, 3) != &num);
    fail_if(last_num_destroyed != 2);
    Nst_pa_clear(&arr, NULL);

    EXIT_TEST;
}

TestResult test_pa_clear(void)
{
    ENTER_TEST;

    Nst_PtrArray arr;
    fail_if(!Nst_pa_init(&arr, 5));
    i32 num = 8;
    for (i32 i = 0; i < 5; i++)
        Nst_pa_append(&arr, &num);
    Nst_pa_clear(&arr, (Nst_Destructor)int_dstr);
    fail_if(arr.data != NULL);
    fail_if(arr.cap != 0);
    fail_if(arr.len != 0);
    fail_if(last_num_destroyed != 8);

    EXIT_TEST;
}
