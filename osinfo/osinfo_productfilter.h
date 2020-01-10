/*
 * libosinfo: a mechanism to filter operating systems
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

#ifndef __OSINFO_PRODUCTFILTER_H__
# define __OSINFO_PRODUCTFILTER_H__

# include "osinfo/osinfo_macros.h"

# define OSINFO_TYPE_PRODUCTFILTER (osinfo_productfilter_get_type ())
OSINFO_DECLARE_TYPE_WITH_PRIVATE_AND_CLASS(OsinfoProductFilter,
                                           osinfo_productfilter,
                                           OSINFO,
                                           PRODUCTFILTER,
                                           OsinfoFilter)

OsinfoProductFilter *osinfo_productfilter_new(void);

gint osinfo_productfilter_add_product_constraint(OsinfoProductFilter *productfilter, OsinfoProductRelationship relshp, OsinfoProduct *product);
void osinfo_productfilter_clear_product_constraint(OsinfoProductFilter *productfilter, OsinfoProductRelationship relshp);
void osinfo_productfilter_clear_product_constraints(OsinfoProductFilter *productfilter);

GList *osinfo_productfilter_get_product_constraint_values(OsinfoProductFilter *productfilter, OsinfoProductRelationship relshp);

void osinfo_productfilter_add_support_date_constraint(OsinfoProductFilter *productfilter, GDate *when);

#endif /* __OSINFO_PRODUCTFILTER_H__ */
