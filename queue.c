#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "harness.h"
#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

#define list_val(l) list_entry(l, element_t, list)->value

/*
 * Create empty queue.
 * Return NULL if could not allocate space.
 */
struct list_head *q_new()
{
    struct list_head *q = malloc(sizeof(struct list_head));

    if (q) {
        q->prev = q;
        q->next = q;
    }
    return q;
}

static inline void q_delete_node(struct list_head *node)
{
    if (!node)
        return;

    element_t *tmp = list_entry(node, element_t, list);
    if (tmp->value)
        free(tmp->value);
    free(tmp);
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    struct list_head *cur;

    if (!l)
        return;
    l->prev->next = NULL;

    for (cur = l->next; cur;) {
        struct list_head *tmp = cur;
        cur = cur->next;
        q_delete_node(tmp);
    }

    free(l);
}

static inline element_t *e_new(char *s)
{
    element_t *ele = malloc(sizeof(element_t));
    size_t bufSiz = sizeof(char) * (strlen(s) + 1);

    if (!ele)
        return NULL;

    if (!(ele->value = malloc(bufSiz))) {
        free(ele);
        return NULL;
    }

    ele->value = strncpy(ele->value, s, bufSiz);
    return ele;
}

/*
 * Attempt to insert element at head of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *ele = e_new(s);

    if (!ele)
        return false;

    list_add(&ele->list, head);

    return true;
}

/*
 * Attempt to insert element at tail of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *ele = e_new(s);

    if (!ele)
        return false;

    list_add_tail(&ele->list, head);

    return true;
}

/*
 * Attempt to remove element from head of queue.
 * Return target element.
 * Return NULL if queue is NULL or empty.
 * If sp is non-NULL and an element is removed, copy the removed string to *sp
 * (up to a maximum of bufsize-1 characters, plus a null terminator.)
 *
 * NOTE: "remove" is different from "delete"
 * The space used by the list element and the string should not be freed.
 * The only thing "remove" need to do is unlink it.
 *
 * REF:
 * https://english.stackexchange.com/questions/52508/difference-between-delete-and-remove
 */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || head->next == head)
        return NULL;

    element_t *tar = list_first_entry(head, element_t, list);

    head->next = tar->list.next;
    tar->list.next->prev = head;

    if (sp && bufsize > 0) {
        strncpy(sp, tar->value, bufsize - 1);
    }
    return tar;
}

/*
 * Attempt to remove element from tail of queue.
 * Other attribute is as same as q_remove_head.
 */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || head->prev == head)
        return NULL;

    element_t *tar = list_last_entry(head, element_t, list);

    head->prev = tar->list.prev;
    tar->list.prev->next = head;

    if (sp && bufsize > 0) {
        strncpy(sp, tar->value, bufsize - 1);
    }

    return tar;
}

/*
 * WARN: This is for external usage, don't modify it
 * Attempt to release element.
 */
void q_release_element(element_t *e)
{
    free(e->value);
    free(e);
}

/*
 * Return number of elements in queue.
 * Return 0 if q is NULL or empty
 */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;

    int len = 0;
    struct list_head *li;

    list_for_each (li, head)
        len++;
    return len;
}

/*
 * Delete the middle node in list.
 * The middle node of a linked list of size n is the
 * ⌊n / 2⌋th node from the start using 0-based indexing.
 * If there're six element, the third member should be return.
 * Return true if successful.
 * Return false if list is NULL or empty.
 */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    element_t *tar;
    struct list_head **slow, **fast;

    if (!head)
        return false;

    for (slow = fast = &(head->next); (*fast) != head && (*fast)->next != head;
         slow = &((*slow)->next), fast = &((*fast)->next->next))
        ;

    tar = list_entry(&(**slow), element_t, list);
    tar->list.prev->next = tar->list.next;
    tar->list.next->prev = tar->list.prev;

    if (tar->value)
        free(tar->value);
    free(tar);
    return true;
}

/*
 * Delete all nodes that have duplicate string,
 * leaving only distinct strings from the original list.
 * Return true if successful.
 * Return false if list is NULL.
 *
 * Note: this function always be called after sorting, in other words,
 * list is guaranteed to be sorted in ascending order.
 */
bool q_delete_dup(struct list_head *head)
{
    if (!head)
        return false;

    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    for (struct list_head *cur = head->next; cur && cur != head;) {
        bool isRemove = false;
        while (cur->next != head &&
               !strcmp(list_val(cur), list_val(cur->next))) {
            isRemove = true;
            cur->next = cur->next->next;
            q_delete_node(cur->next->prev);
        }
        if (isRemove) {
            struct list_head *tmp = cur;
            cur->prev->next = cur->next;
            cur->next->prev = cur->prev;
            cur = cur->next;
            q_delete_node(tmp);
        } else {
            cur = cur->next;
        }
    }
    return true;
}

/*
 * Attempt to swap every two adjacent nodes.
 */
void q_swap(struct list_head *head)
{
    struct list_head *fir, *sec;
    for (fir = head->next, sec = fir->next; fir != head && sec != head;
         fir->prev->next = sec, sec->next->prev = fir, fir->next = sec->next,
        sec->prev = fir->prev, fir->prev = sec, sec->next = fir,
        fir = fir->next, sec = fir->next)
        ;
}

/*
 * Reverse elements in queue
 * No effect if q is NULL or empty
 * This function should not allocate or free any list elements
 * (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
 * It should rearrange the existing ones.
 */
void q_reverse(struct list_head *head)
{
    if (!head || head->next == head) {
        return;
    }

    struct list_head *current = head;
    do {
        struct list_head *tmp = current->next;
        current->next = current->prev;
        current->prev = tmp;
        current = current->prev;
    } while (current != head);
}

struct list_head *q_merge_sort(struct list_head *head);

/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */
void q_sort(struct list_head *head)
{
    struct list_head *last;

    if (!head || head->next == head || head->next->next == head)
        return;

    head->prev->next = NULL;
    head->next = q_merge_sort(head->next);

    for (last = head; last->next; last = last->next) {
        last->next->prev = last;
    }
    head->prev = last;
    last->next = head;
}

struct list_head *q_devide(struct list_head *head)
{
    struct list_head **slow, **fast, *retVal;

    for (slow = fast = &(head); *fast && (*fast)->next;
         slow = &((*slow)->next), fast = &((*fast)->next->next))
        ;
    retVal = *slow;
    *slow = NULL;

    return retVal;
}

struct list_head *q_merge(struct list_head *l1, struct list_head *l2)
{
    struct list_head *indirect, head;

    indirect = &head;

    while (l1 && l2) {
        if (strcmp(list_val(l1), list_val(l2)) < 0) {
            indirect->next = l1;
            l1 = l1->next;
        } else {
            indirect->next = l2;
            l2 = l2->next;
        }
        indirect = indirect->next;
    }
    indirect->next =
        (struct list_head *) ((unsigned long int) l1 | (unsigned long int) l2);

    return head.next;
}

/**
 * sort list_head without include the first member
 */
struct list_head *q_merge_sort(struct list_head *head)
{
    struct list_head *fir, *sec;

    if (!head || !head->next)
        return head;

    fir = head;
    sec = q_devide(fir);

    fir = q_merge_sort(fir);
    sec = q_merge_sort(sec);

    return q_merge(fir, sec);
}