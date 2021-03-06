/*
 * libosinfo: An installation media for a (guest) OS
 *
 * Copyright (C) 2009-2020 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <osinfo/osinfo_media.h>

#ifndef __OSINFO_MEDIA_PRIVATE_H__
# define __OSINFO_MEDIA_PRIVATE_H__

void osinfo_media_set_languages(OsinfoMedia *media, GList *languages);
void osinfo_media_set_os(OsinfoMedia *media, OsinfoOs *os);

#endif /* __OSINFO_MEDIA_PRIVATE_H__ */
