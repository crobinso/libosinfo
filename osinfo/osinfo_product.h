/*
 * osinfo: an software product
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPPRODUCTE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <glib-object.h>
#include <osinfo/osinfo_productlist.h>

#include "osinfo/osinfo_macros.h"

#define OSINFO_TYPE_PRODUCT (osinfo_product_get_type ())
OSINFO_DECLARE_TYPE_WITH_PRIVATE_AND_CLASS(OsinfoProduct,
                                           osinfo_product,
                                           OSINFO,
                                           PRODUCT,
                                           OsinfoEntity)

#define OSINFO_PRODUCT_PROP_VENDOR       "vendor"
#define OSINFO_PRODUCT_PROP_VERSION      "version"
#define OSINFO_PRODUCT_PROP_SHORT_ID     "short-id"
#define OSINFO_PRODUCT_PROP_NAME         "name"
#define OSINFO_PRODUCT_PROP_RELEASE_DATE "release-date"
#define OSINFO_PRODUCT_PROP_EOL_DATE     "eol-date"
#define OSINFO_PRODUCT_PROP_CODENAME     "codename"
#define OSINFO_PRODUCT_PROP_LOGO         "logo"

typedef enum {
    OSINFO_PRODUCT_RELATIONSHIP_DERIVES_FROM,
    OSINFO_PRODUCT_RELATIONSHIP_UPGRADES,
    OSINFO_PRODUCT_RELATIONSHIP_CLONES,
} OsinfoProductRelationship;

OsinfoProductList *osinfo_product_get_related(OsinfoProduct *product, OsinfoProductRelationship relshp);

void osinfo_product_add_related(OsinfoProduct *product, OsinfoProductRelationship relshp, OsinfoProduct *otherproduct);

const gchar *osinfo_product_get_vendor(OsinfoProduct *product);
const gchar *osinfo_product_get_version(OsinfoProduct *product);
const gchar *osinfo_product_get_short_id(OsinfoProduct *product);
const gchar *osinfo_product_get_name(OsinfoProduct *product);
const gchar *osinfo_product_get_codename(OsinfoProduct *product);

const gchar *osinfo_product_get_release_date_string(OsinfoProduct *product);
const gchar *osinfo_product_get_eol_date_string(OsinfoProduct *product);
GDate *osinfo_product_get_release_date(OsinfoProduct *product);
GDate *osinfo_product_get_eol_date(OsinfoProduct *product);
const gchar *osinfo_product_get_logo(OsinfoProduct *product);

GList *osinfo_product_get_short_id_list(OsinfoProduct *product);
