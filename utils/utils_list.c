/*
* Copyright (C) 2012-2019 UCloud. All Rights Reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License").
* You may not use this file except in compliance with the License.
* A copy of the License is located at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* or in the "license" file accompanying this file. This file is distributed
* on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
* express or implied. See the License for the specific language governing
* permissions and limitations under the License.
*/

#ifdef __cplusplus
extern "C" {
#endif

#include "utils_list.h"

 /*
 * 创建List. 失败则返回NULL.
 */
List *_list_new(void)
{
    List *self;
    self = (List *)malloc(sizeof(List));
    if (!self) {
        return NULL;
    }
    self->head = NULL;
    self->tail = NULL;
    self->free = NULL;
    self->match = NULL;
    self->len = 0;
    return self;
}

/*
 * 失败List的内存.
 */
void _list_destroy(List *self)
{
    unsigned int len = self->len;
    ListNode *next;
    ListNode *curr = self->head;

    while (len--) {
        next = curr->next;
        if (self->free) {
            self->free(curr->val);
        }
        free(curr);
        curr = next;
    }

    free(self);
}

/*
 * 将给定节点附加到列表并返回该节点，在失败时返回NULL.
 */
ListNode *_list_rpush(List *self, ListNode *node)
{
    if (!node) {
        return NULL;
    }

    if (self->len) {
        node->prev = self->tail;
        node->next = NULL;
        self->tail->next = node;
        self->tail = node;
    } else {
        self->head = self->tail = node;
        node->prev = node->next = NULL;
    }

    ++self->len;
    return node;
}

/*
 * 弹出列表中的最后一个节点, 失败返回NULL.
 */
ListNode *_list_rpop(List *self)
{
    ListNode *node = NULL;
    if (!self->len) {
        return NULL;
    }

    node = self->tail;

    if (--self->len) {
        (self->tail = node->prev)->next = NULL;
    } else {
        self->tail = self->head = NULL;
    }

    node->next = node->prev = NULL;
    return node;
}

/*
 * 弹出列表中的首个节点, 失败返回NULL.
 */
ListNode *_list_lpop(List *self)
{
    ListNode *node = NULL;
    if (!self->len) {
        return NULL;
    }

    node = self->head;

    if (--self->len) {
        (self->head = node->next)->prev = NULL;
    } else {
        self->head = self->tail = NULL;
    }

    node->next = node->prev = NULL;
    return node;
}

/*
 * 预先将给定的节点添加到列表中，并返回该节点，在失败时返回NULL.
 */
ListNode *_list_lpush(List *self, ListNode *node)
{
    if (!node) {
        return NULL;
    }

    if (self->len) {
        node->next = self->head;
        node->prev = NULL;
        self->head->prev = node;
        self->head = node;
    } else {
        self->head = self->tail = node;
        node->prev = node->next = NULL;
    }

    ++self->len;
    return node;
}

/*
 * 根据val返回对应的节点，没有则返回NULL.
 */
ListNode *_list_find(List *self, void *val)
{
    ListIterator *it;
    ListNode *node;

    if (NULL == (it = _list_iterator_new(self, LIST_HEAD))) {
        return NULL;
    }
    node = _list_iterator_next(it);
    while (node) {
        if (self->match) {
            if (self->match(val, node->val)) {
                _list_iterator_destroy(it);
                return node;
            }
        } else {
            if (val == node->val) {
                _list_iterator_destroy(it);
                return node;
            }
        }
        node = _list_iterator_next(it);
    }

    _list_iterator_destroy(it);
    return NULL;
}

/*
 * 根据index返回对应的节点，没有则返回NULL.
 */
ListNode *_list_at(List *self, int index)
{
    ListDirection direction = LIST_HEAD;

    if (index < 0) {
        direction = LIST_TAIL;
        index = ~index;
    }

    if ((unsigned) index < self->len) {
        ListIterator *it;
        ListNode *node;

        if (NULL == (it = _list_iterator_new(self, direction))) {
            return NULL;
        }
        node = _list_iterator_next(it);

        while (index--) {
            node = _list_iterator_next(it);
        }
        _list_iterator_destroy(it);
        return node;
    }

    return NULL;
}

/*
 * 从列表中删除给定的节点，释放它和它的值.
 */
void _list_remove(List *self, ListNode *node)
{
    node->prev ? (node->prev->next = node->next) : (self->head = node->next);

    node->next ? (node->next->prev = node->prev) : (self->tail = node->prev);

    if (self->free) {
        self->free(node->val);
    }

    free(node);
    --self->len;
}

/*
 * 创建一个新的ListIterator，失败返回NULL, 并且设置其ListDirection.
 */
ListIterator *_list_iterator_new(List *list, ListDirection direction)
{
    ListNode *node = direction == LIST_HEAD ? list->head : list->tail;
    return _list_iterator_new_from_node(node, direction);
}

/*
 * 创建一个新的ListIterator, 并设置初始节点. 失败则返回NULL.
 */
ListIterator *_list_iterator_new_from_node(ListNode *node, ListDirection direction)
{
    ListIterator *self;
    self = malloc(sizeof(ListIterator));
    if (!self) {
        return NULL;
    }
    self->next = node;
    self->direction = direction;
    return self;
}

/*
 * 返回下一个节点, 如果没有更多的节点则返回NULL.
 */
ListNode *_list_iterator_next(ListIterator *self)
{
    ListNode *curr = self->next;
    if (curr) {
        self->next = self->direction == LIST_HEAD ? curr->next : curr->prev;
    }
    return curr;
}

/*
 * 释放列表迭代器.
 */
void _list_iterator_destroy(ListIterator *self)
{
    free(self);
    self = NULL;
}

/*
 * 根据预设值来创建新节点, 失败则返回NULL.
 */
ListNode *_list_node_new(void *val)
{
    ListNode *self;
    self = malloc(sizeof(ListNode));
    if (!self) {
        return NULL;
    }

    self->prev = NULL;
    self->next = NULL;
    self->val = val;
    return self;
}

#ifdef __cplusplus
}
#endif
