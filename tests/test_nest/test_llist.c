#include "tests.h"

TestResult test_llist_push(void)
{
    TEST_ENTER;
    Nst_LList llist;
    Nst_llist_init(&llist);

    Nst_llist_push(&llist, (void *)1, false);
    test_assert(llist.len == 1);
    Nst_llist_push(&llist, (void *)2, false);
    test_assert(llist.len == 2);
    Nst_llist_push(&llist, (void *)3, false);
    test_assert(llist.len == 3);

    test_assert(llist.head->value == (void *)3);
    test_assert(llist.head->next->value == (void *)2);
    test_assert(llist.tail->value == (void *)1);
    test_assert(llist.head->next->next == llist.tail);

    Nst_llist_empty(&llist, NULL);
    TEST_EXIT;
}

TestResult test_llist_append(void)
{
    TEST_ENTER;
    Nst_LList llist;
    Nst_llist_init(&llist);

    Nst_llist_append(&llist, (void *)1, false);
    test_assert(llist.len == 1);
    Nst_llist_append(&llist, (void *)2, false);
    test_assert(llist.len == 2);
    Nst_llist_append(&llist, (void *)3, false);
    test_assert(llist.len == 3);

    test_assert(llist.head->value == (void *)1);
    test_assert(llist.head->next->value == (void *)2);
    test_assert(llist.tail->value == (void *)3);
    test_assert(llist.head->next->next == llist.tail);

    Nst_llist_empty(&llist, NULL);
    TEST_EXIT;
}

TestResult test_llist_insert(void)
{
    TEST_ENTER;
    Nst_LList llist;
    Nst_llist_init(&llist);

    Nst_llist_append(&llist, (void *)1, false);
    Nst_llist_append(&llist, (void *)2, false);

    Nst_llist_insert(&llist, (void *)3, false, llist.head);
    test_assert(llist.head->next->value == (void *)3);
    Nst_llist_insert(&llist, (void *)4, false, llist.tail);
    test_assert(llist.tail->value == (void *)4);
    Nst_llist_insert(&llist, (void *)5, false, NULL);
    test_assert(llist.head->value == (void *)5);

    test_assert(llist.len == 5);

    Nst_llist_empty(&llist, NULL);
    TEST_EXIT;
}

TestResult test_llist_pop(void)
{
    TEST_ENTER;
    Nst_LList llist;
    Nst_llist_init(&llist);

    test_assert(Nst_llist_pop(&llist) == NULL);
    Nst_llist_append(&llist, (void *)1, false);
    Nst_llist_append(&llist, (void *)2, false);
    void *value = Nst_llist_pop(&llist);
    test_assert(llist.len == 1);
    test_assert(value == (void *)1);
    Nst_llist_pop(&llist);
    test_assert(llist.head == NULL);
    test_assert(llist.tail == NULL);

    Nst_llist_empty(&llist, NULL);
    TEST_EXIT;
}

TestResult test_llist_peek_front(void)
{
    TEST_ENTER;
    Nst_LList llist;
    Nst_llist_init(&llist);

    test_assert(Nst_llist_peek_front(&llist) == NULL);
    Nst_llist_append(&llist, (void *)1, false);
    Nst_llist_append(&llist, (void *)2, false);
    void *value = Nst_llist_peek_front(&llist);
    test_assert(llist.len == 2);
    test_assert(value == (void *)1);

    Nst_llist_empty(&llist, NULL);
    TEST_EXIT;
}

TestResult test_llist_peek_back(void)
{
    TEST_ENTER;
    Nst_LList llist;
    Nst_llist_init(&llist);

    test_assert(Nst_llist_peek_back(&llist) == NULL);
    Nst_llist_append(&llist, (void *)1, false);
    Nst_llist_append(&llist, (void *)2, false);
    void *value = Nst_llist_peek_back(&llist);
    test_assert(llist.len == 2);
    test_assert(value == (void *)2);

    Nst_llist_empty(&llist, NULL);
    TEST_EXIT;
}

TestResult test_llist_push_llnode(void)
{
    TEST_ENTER;
    Nst_LList llist;
    Nst_llist_init(&llist);

    Nst_LLNode *node1 = Nst_llnode_new((void *)1, false);
    Nst_LLNode *node2 = Nst_llnode_new((void *)2, false);
    Nst_llist_push_llnode(&llist, node1);
    test_assert(llist.len == 1);
    test_assert(llist.head == node1);
    test_assert(llist.tail == node1);
    Nst_llist_push_llnode(&llist, node2);
    test_assert(llist.len == 2);
    test_assert(llist.head == node2);
    test_assert(llist.tail == node1);

    Nst_llist_empty(&llist, NULL);
    TEST_EXIT;
}

TestResult test_llist_append_llnode(void)
{
    TEST_ENTER;
    Nst_LList llist;
    Nst_llist_init(&llist);

    Nst_LLNode *node1 = Nst_llnode_new((void *)1, false);
    Nst_LLNode *node2 = Nst_llnode_new((void *)2, false);
    Nst_llist_append_llnode(&llist, node1);
    test_assert(llist.len == 1);
    test_assert(llist.head == node1);
    test_assert(llist.tail == node1);
    Nst_llist_append_llnode(&llist, node2);
    test_assert(llist.len == 2);
    test_assert(llist.head == node1);
    test_assert(llist.tail == node2);

    Nst_llist_empty(&llist, NULL);
    TEST_EXIT;
}

TestResult test_llist_pop_llnode(void)
{
    TEST_ENTER;
    Nst_LList llist;
    Nst_llist_init(&llist);

    Nst_LLNode *node1 = NULL;
    Nst_LLNode *node2 = NULL;

    test_assert(Nst_llist_pop_llnode(&llist) == NULL);
    Nst_llist_append(&llist, (void *)1, false);
    Nst_llist_append(&llist, (void *)2, false);
    node1 = Nst_llist_pop_llnode(&llist);
    test_assert(llist.len == 1);
    test_assert(node1->value == (void *)1);
    node2 = Nst_llist_pop_llnode(&llist);
    test_assert(llist.head == NULL);
    test_assert(llist.tail == NULL);
    test_assert(node2->value == (void *)2);

    if (node1 != NULL)
        Nst_free(node1);
    if (node2 != NULL)
        Nst_free(node2);
    Nst_llist_empty(&llist, NULL);
    TEST_EXIT;
}

TestResult test_llist_empty(void)
{
    TEST_ENTER;
    Nst_LList llist;
    Nst_llist_init(&llist);

    Nst_llist_append(&llist, (void *)1, false);
    Nst_llist_append(&llist, (void *)2, false);
    Nst_llist_append(&llist, (void *)3, false);
    Nst_llist_empty(&llist, NULL);
    test_assert(llist.len == 0);
    test_assert(llist.head == NULL);
    test_assert(llist.tail == NULL);

    void *alloc_v1 = Nst_raw_malloc(1);
    void *alloc_v2 = Nst_raw_malloc(1);
    void *alloc_v3 = Nst_raw_malloc(1);
    Nst_llist_append(&llist, alloc_v1, true);
    Nst_llist_append(&llist, alloc_v2, true);
    Nst_llist_append(&llist, alloc_v3, true);
    Nst_llist_empty(&llist, Nst_free);
    test_assert(llist.len == 0);
    test_assert(llist.head == NULL);
    test_assert(llist.tail == NULL);

    Nst_llist_empty(&llist, NULL);
    TEST_EXIT;
}

TestResult test_llist_move_nodes(void)
{
    TEST_ENTER;
    Nst_LList from, to;
    Nst_llist_init(&from);
    Nst_llist_init(&to);

    Nst_llist_append(&from, (void *)1, false);
    Nst_llist_append(&from, (void *)2, false);
    Nst_llist_append(&from, (void *)3, false);
    Nst_llist_move_nodes(&from, &to);

    test_assert(from.len == 0);
    test_assert(from.head == NULL);
    test_assert(from.tail == NULL);
    test_assert(to.len == 3);
    test_assert(to.head->value == (void *)1);
    test_assert(to.tail->value == (void *)3);
    test_assert(to.head->next->value == (void *)2);

    Nst_llist_empty(&from, NULL);
    Nst_llist_empty(&to, NULL);
    TEST_EXIT;
}
