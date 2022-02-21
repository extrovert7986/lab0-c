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

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    struct list_head *cur;
    l->prev->next = NULL;

    for (cur = l->next; cur;) {
        element_t *tmp = list_entry(cur, element_t, list);
        cur = cur->next;
        if (tmp->value)
            free(tmp->value);
        free(tmp);
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
    element_t *ele = e_new(s);

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
    element_t *ele = e_new(s);

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
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
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

/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */
void q_sort(struct list_head *head) {}
