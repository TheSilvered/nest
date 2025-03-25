#include "tests.h"

TestResult test_llist_push(void)
{
    ENTER_TEST;
    Nst_LList llist;
    Nst_llist_init(&llist);

    Nst_llist_push(&llist, (void *)1, false);
    fail_if(llist.len != 1);
    Nst_llist_push(&llist, (void *)2, false);
    fail_if(llist.len != 2);
    Nst_llist_push(&llist, (void *)3, false);
    fail_if(llist.len != 3);

    fail_if(llist.head->value != (void *)3);
    fail_if(llist.head->next->value != (void *)2);
    fail_if(llist.tail->value != (void *)1);
    fail_if(llist.head->next->next != llist.tail);

    Nst_llist_empty(&llist, NULL);
    EXIT_TEST;
}

TestResult test_llist_append(void)
{
    ENTER_TEST;
    Nst_LList llist;
    Nst_llist_init(&llist);

    Nst_llist_append(&llist, (void *)1, false);
    fail_if(llist.len != 1);
    Nst_llist_append(&llist, (void *)2, false);
    fail_if(llist.len != 2);
    Nst_llist_append(&llist, (void *)3, false);
    fail_if(llist.len != 3);

    fail_if(llist.head->value != (void *)1);
    fail_if(llist.head->next->value != (void *)2);
    fail_if(llist.tail->value != (void *)3);
    fail_if(llist.head->next->next != llist.tail);

    Nst_llist_empty(&llist, NULL);
    EXIT_TEST;
}

TestResult test_llist_insert(void)
{
    ENTER_TEST;
    Nst_LList llist;
    Nst_llist_init(&llist);

    Nst_llist_append(&llist, (void *)1, false);
    Nst_llist_append(&llist, (void *)2, false);

    Nst_llist_insert(&llist, (void *)3, false, llist.head);
    fail_if(llist.head->next->value != (void *)3);
    Nst_llist_insert(&llist, (void *)4, false, llist.tail);
    fail_if(llist.tail->value != (void *)4);
    Nst_llist_insert(&llist, (void *)5, false, NULL);
    fail_if(llist.head->value != (void *)5);

    fail_if(llist.len != 5);

    Nst_llist_empty(&llist, NULL);
    EXIT_TEST;
}

TestResult test_llist_pop(void)
{
    ENTER_TEST;
    Nst_LList llist;
    Nst_llist_init(&llist);

    fail_if(Nst_llist_pop(&llist) != NULL);
    Nst_llist_append(&llist, (void *)1, false);
    Nst_llist_append(&llist, (void *)2, false);
    void *value = Nst_llist_pop(&llist);
    fail_if(llist.len != 1);
    fail_if(value != (void *)1);
    Nst_llist_pop(&llist);
    fail_if(llist.head != NULL);
    fail_if(llist.tail != NULL);

    Nst_llist_empty(&llist, NULL);
    EXIT_TEST;
}

TestResult test_llist_peek_front(void)
{
    ENTER_TEST;
    Nst_LList llist;
    Nst_llist_init(&llist);

    fail_if(Nst_llist_peek_front(&llist) != NULL);
    Nst_llist_append(&llist, (void *)1, false);
    Nst_llist_append(&llist, (void *)2, false);
    void *value = Nst_llist_peek_front(&llist);
    fail_if(llist.len != 2);
    fail_if(value != (void *)1);

    Nst_llist_empty(&llist, NULL);
    EXIT_TEST;
}

TestResult test_llist_peek_back(void)
{
    ENTER_TEST;
    Nst_LList llist;
    Nst_llist_init(&llist);

    fail_if(Nst_llist_peek_back(&llist) != NULL);
    Nst_llist_append(&llist, (void *)1, false);
    Nst_llist_append(&llist, (void *)2, false);
    void *value = Nst_llist_peek_back(&llist);
    fail_if(llist.len != 2);
    fail_if(value != (void *)2);

    Nst_llist_empty(&llist, NULL);
    EXIT_TEST;
}

TestResult test_llist_push_llnode(void)
{
    ENTER_TEST;
    Nst_LList llist;
    Nst_llist_init(&llist);

    Nst_LLNode *node1 = Nst_llnode_new((void *)1, false);
    Nst_LLNode *node2 = Nst_llnode_new((void *)2, false);
    Nst_llist_push_llnode(&llist, node1);
    fail_if(llist.len != 1);
    fail_if(llist.head != node1);
    fail_if(llist.tail != node1);
    Nst_llist_push_llnode(&llist, node2);
    fail_if(llist.len != 2);
    fail_if(llist.head != node2);
    fail_if(llist.tail != node1);

    Nst_llist_empty(&llist, NULL);
    EXIT_TEST;
}

TestResult test_llist_append_llnode(void)
{
    ENTER_TEST;
    Nst_LList llist;
    Nst_llist_init(&llist);

    Nst_LLNode *node1 = Nst_llnode_new((void *)1, false);
    Nst_LLNode *node2 = Nst_llnode_new((void *)2, false);
    Nst_llist_append_llnode(&llist, node1);
    fail_if(llist.len != 1);
    fail_if(llist.head != node1);
    fail_if(llist.tail != node1);
    Nst_llist_append_llnode(&llist, node2);
    fail_if(llist.len != 2);
    fail_if(llist.head != node1);
    fail_if(llist.tail != node2);

    Nst_llist_empty(&llist, NULL);
    EXIT_TEST;
}

TestResult test_llist_pop_llnode(void)
{
    ENTER_TEST;
    Nst_LList llist;
    Nst_llist_init(&llist);

    Nst_LLNode *node1 = NULL;
    Nst_LLNode *node2 = NULL;

    fail_if(Nst_llist_pop_llnode(&llist) != NULL);
    Nst_llist_append(&llist, (void *)1, false);
    Nst_llist_append(&llist, (void *)2, false);
    node1 = Nst_llist_pop_llnode(&llist);
    fail_if(llist.len != 1);
    fail_if(node1->value != (void *)1);
    node2 = Nst_llist_pop_llnode(&llist);
    fail_if(llist.head != NULL);
    fail_if(llist.tail != NULL);
    fail_if(node2->value != (void *)2);

    if (node1 != NULL)
        Nst_free(node1);
    if (node2 != NULL)
        Nst_free(node2);
    Nst_llist_empty(&llist, NULL);
    EXIT_TEST;
}

TestResult test_llist_empty(void)
{
    ENTER_TEST;
    Nst_LList llist;
    Nst_llist_init(&llist);

    Nst_llist_append(&llist, (void *)1, false);
    Nst_llist_append(&llist, (void *)2, false);
    Nst_llist_append(&llist, (void *)3, false);
    Nst_llist_empty(&llist, NULL);
    fail_if(llist.len != 0);
    fail_if(llist.head != NULL);
    fail_if(llist.tail != NULL);

    void *alloc_v1 = Nst_raw_malloc(1);
    void *alloc_v2 = Nst_raw_malloc(1);
    void *alloc_v3 = Nst_raw_malloc(1);
    Nst_llist_append(&llist, alloc_v1, true);
    Nst_llist_append(&llist, alloc_v2, true);
    Nst_llist_append(&llist, alloc_v3, true);
    Nst_llist_empty(&llist, Nst_free);
    fail_if(llist.len != 0);
    fail_if(llist.head != NULL);
    fail_if(llist.tail != NULL);

    Nst_llist_empty(&llist, NULL);
    EXIT_TEST;
}

TestResult test_llist_move_nodes(void)
{
    ENTER_TEST;
    Nst_LList from, to;
    Nst_llist_init(&from);
    Nst_llist_init(&to);

    Nst_llist_append(&from, (void *)1, false);
    Nst_llist_append(&from, (void *)2, false);
    Nst_llist_append(&from, (void *)3, false);
    Nst_llist_move_nodes(&from, &to);

    fail_if(from.len != 0);
    fail_if(from.head != NULL);
    fail_if(from.tail != NULL);
    fail_if(to.len != 3);
    fail_if(to.head->value != (void *)1);
    fail_if(to.tail->value != (void *)3);
    fail_if(to.head->next->value != (void *)2);

    Nst_llist_empty(&from, NULL);
    Nst_llist_empty(&to, NULL);
    EXIT_TEST;
}
