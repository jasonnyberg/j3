/*
 * j2 - A simple concatenative programming language that can understand
 *      the structure of and dynamically link with compatible C binaries.
 *
 * Copyright (C) 2011 Jason Nyberg <jasonnyberg@gmail.com>
 * Copyright (C) 2018 Jason Nyberg <jasonnyberg@gmail.com> (dual-licensed)
 * (C) Copyright 2019 Hewlett Packard Enterprise Development LP.
 *
 * This file is part of j2.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either
 *
 *   * the GNU Lesser General Public License as published by the Free
 *     Software Foundation; either version 3 of the License, or (at
 *     your option) any later version
 *
 * or
 *
 *   * the GNU General Public License as published by the Free
 *     Software Foundation; either version 3 of the License, or (at
 *     your option) any later version
 *
 * or both in parallel, as here.
 *
 * j2 is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received copies of the GNU General Public License and
 * the GNU Lesser General Public License along with this program.  If
 * not, see <http://www.gnu.org/licenses/>.
 */

//////////////////////////////////////////////////
// Circular linked-list (sentinel implementation)
//////////////////////////////////////////////////

#pragma once

#include <functional>

#include "alloc.h"

// a template defining a circular linked list class that uses CompressedPointers as its links
template <typename T> class CLL;
// continue the template definition
template <typename T> class CLL<T> {
    enum { HEAD = 0, TAIL = 1, FWD = 0, REV = 1, KEEP = 0, POP = 1 };

    // a private class that defines the links in the list
    class Link {
    public:
      // the links in the list
      CompressedPointer<CLL<T>> lnk[2];
      // the data in the link
      T data;
    };

  public:
    // member functions that are analagous to the CLL declared below
    static CLL *init(CLL *lst) { return lst; }
    static void release(CLL *lst, void (*op)(CLL *cll)) {}

    static CLL *splice(CLL *a, CLL *b, int end) { return nullptr; }
    static CLL *cut(CLL *lnk) { return nullptr; }
    static CLL *get(CLL *lst, int pop, int end) { return nullptr; }
    static CLL *put(CLL *lst, CLL *lnk, int end) { return nullptr; }
    static CLL *next(CLL *lst, CLL *lnk, int end) { return nullptr; }
    static void *mapfrom(CLL *sentinel, CLL *ff, int dir, void *(*op)(CLL *cll)) { return nullptr; }
    static void *map(CLL *sentinel, int dir, void *(*op)(CLL *cll)) { return nullptr; }
    static int   len(CLL *sentinel) { return 0; }

    inline static CLL *getSib(Link *x, int end) { return x->lnk[end!=0]; }
    inline static CLL *getHead(CLL *sentinel) { return get(sentinel, KEEP, HEAD); }
    inline static CLL *getTail(CLL *sentinel) { return get(sentinel, KEEP, TAIL); }
    inline static bool isEmpty(CLL *sentinel) { return !getHead(sentinel); }
    inline static CLL *rotate(CLL *sentinel, int dir) { return put(sentinel, get(sentinel, POP, dir), !dir); }
    inline static CLL *merge(CLL *dst, CLL *src, int dir) { return put(sentinel, CLL_get(sentinel, POP, dir), !dir); }
};


class CLL;
typedef struct CLL CLL;
struct CLL { CLL *lnk[2]; };

enum { HEAD=0,TAIL=1,FWD=0,REV=1,KEEP=0,POP=1 };

extern CLL *CLL_init(CLL *lst);                         // init and return lst
extern void CLL_release(CLL *lst,void (*op)(CLL *cll)); // pop each list item and call op on it

extern CLL *CLL_splice(CLL *a,CLL *b,int end);   // convert a<->a' and b<->b' to a<->b and a'<->b'
extern CLL *CLL_cut(CLL *lnk);                   // cut lnk from list it's in and return it/NULL
extern CLL *CLL_get(CLL *lst,int pop,int end);   // get/pop lst's head or tail, return it/NULL
extern CLL *CLL_put(CLL *lst,CLL *lnk,int end);  // add lnk to lst's head or tail, return it
extern CLL *CLL_next(CLL *lst,CLL *lnk,int end); // return next (or prev) lnk in list, or NULL if none.

// calls op(lnk,data) for each lnk in lst until op returns non-zero; returns what last op returns
typedef std::function<void *(CLL *)> CLL_OP;
extern void *CLL_mapfrom(CLL *sentinel,CLL *ff,int dir,CLL_OP op); // map with fast-forward
extern void *CLL_map(CLL *sentinel,int dir,CLL_OP op);
extern int CLL_len(CLL *sentinel);
