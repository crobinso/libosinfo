/*
 * libosinfo:
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

#include <osinfo/osinfo.h>
#include <glib/gi18n-lib.h>

/**
 * SECTION:osinfo_install_scriptlist
 * @short_description: A list of installation install_script
 * @see_also: #OsinfoList, #OsinfoInstallScript
 *
 * #OsinfoInstallScriptList is a list specialization that stores
 * only #OsinfoInstallScript objects.
 */

struct _OsinfoInstallScriptListPrivate
{
    gboolean unused;
};

G_DEFINE_TYPE_WITH_PRIVATE(OsinfoInstallScriptList, osinfo_install_scriptlist, OSINFO_TYPE_LIST);

static void
osinfo_install_scriptlist_finalize(GObject *object)
{
    /* Chain up to the parent class */
    G_OBJECT_CLASS(osinfo_install_scriptlist_parent_class)->finalize(object);
}

/* Init functions */
static void
osinfo_install_scriptlist_class_init(OsinfoInstallScriptListClass *klass)
{
    GObjectClass *g_klass = G_OBJECT_CLASS(klass);

    g_klass->finalize = osinfo_install_scriptlist_finalize;
}

static void
osinfo_install_scriptlist_init(OsinfoInstallScriptList *list)
{
    list->priv = osinfo_install_scriptlist_get_instance_private(list);
}

/**
 * osinfo_install_scriptlist_new:
 *
 * Construct a new install_script list that is initially empty.
 *
 * Returns: (transfer full): an empty install_script list
 *
 * Since: 0.2.0
 */
OsinfoInstallScriptList *osinfo_install_scriptlist_new(void)
{
    return g_object_new(OSINFO_TYPE_INSTALL_SCRIPTLIST,
                        "element-type", OSINFO_TYPE_INSTALL_SCRIPT,
                        NULL);
}

/**
 * osinfo_install_scriptlist_new_copy:
 * @source: the install_script list to copy
 *
 * Construct a new install_script list that is filled with install_scripts
 * from @source
 *
 * Returns: (transfer full): a copy of the install_script list
 *
 * Since: 0.2.0
 * Deprecated: 0.2.2: Use osinfo_list_new_copy() instead.
 */
OsinfoInstallScriptList *osinfo_install_scriptlist_new_copy(OsinfoInstallScriptList *source)
{
    OsinfoInstallScriptList *newList = osinfo_install_scriptlist_new();
    osinfo_list_add_all(OSINFO_LIST(newList),
                        OSINFO_LIST(source));
    return newList;
}

/**
 * osinfo_install_scriptlist_new_filtered:
 * @source: the install_script list to copy
 * @filter: the filter to apply
 *
 * Construct a new install_script list that is filled with install_scripts
 * from @source that match @filter
 *
 * Returns: (transfer full): a filtered copy of the install_script list
 *
 * Since: 0.2.0
 * Deprecated: 0.2.2: Use osinfo_list_new_filtered() instead.
 */
OsinfoInstallScriptList *osinfo_install_scriptlist_new_filtered(OsinfoInstallScriptList *source,
                                                                OsinfoFilter *filter)
{
    OsinfoInstallScriptList *newList = osinfo_install_scriptlist_new();
    osinfo_list_add_filtered(OSINFO_LIST(newList),
                             OSINFO_LIST(source),
                             filter);
    return newList;
}

/**
 * osinfo_install_scriptlist_new_intersection:
 * @sourceOne: the first install_script list to copy
 * @sourceTwo: the second install_script list to copy
 *
 * Construct a new install_script list that is filled with only the
 * install_scripts that are present in both @sourceOne and @sourceTwo.
 *
 * Returns: (transfer full): an intersection of the two install_script lists
 *
 * Since: 0.2.0
 * Deprecated: 0.2.2: Use osinfo_list_new_intersection() instead.
 */
OsinfoInstallScriptList *osinfo_install_scriptlist_new_intersection(OsinfoInstallScriptList *sourceOne,
                                                                    OsinfoInstallScriptList *sourceTwo)
{
    OsinfoInstallScriptList *newList = osinfo_install_scriptlist_new();
    osinfo_list_add_intersection(OSINFO_LIST(newList),
                                 OSINFO_LIST(sourceOne),
                                 OSINFO_LIST(sourceTwo));
    return newList;
}

/**
 * osinfo_install_scriptlist_new_union:
 * @sourceOne: the first install_script list to copy
 * @sourceTwo: the second install_script list to copy
 *
 * Construct a new install_script list that is filled with all the
 * install_scripts that are present in either @sourceOne and @sourceTwo.
 *
 * Returns: (transfer full): a union of the two install_script lists
 *
 * Since: 0.2.0
 * Deprecated: 0.2.2: Use osinfo_list_new_union() instead.
 */
OsinfoInstallScriptList *osinfo_install_scriptlist_new_union(OsinfoInstallScriptList *sourceOne,
                                                             OsinfoInstallScriptList *sourceTwo)
{
    OsinfoInstallScriptList *newList = osinfo_install_scriptlist_new();
    osinfo_list_add_union(OSINFO_LIST(newList),
                          OSINFO_LIST(sourceOne),
                          OSINFO_LIST(sourceTwo));
    return newList;
}
