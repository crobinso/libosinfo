/*
 * Copyright (C) 2019 Red Hat, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>
 */

#include <config.h>

#include <osinfo/osinfo.h>

typedef const gchar *(*GetURLFunc)(gpointer p);
typedef void (*TestURLFunc)(gconstpointer p);

void test_uri(OsinfoList *list, GetURLFunc get_url_func, GError **error);
void test_uris(OsinfoOsList *oslist, const gchar *prefix, TestURLFunc test_func);
