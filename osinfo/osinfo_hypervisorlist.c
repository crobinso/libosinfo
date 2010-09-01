/*
 * libosinfo:
 *
 * Copyright (C) 2009-2010 Red Hat, Inc
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
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 *
 * Authors:
 *   Arjun Roy <arroy@redhat.com>
 *   Daniel P. Berrange <berrange@redhat.com>
 */

#include <osinfo/osinfo.h>

G_DEFINE_TYPE (OsinfoHypervisorList, osinfo_hypervisorlist, OSINFO_TYPE_LIST);

#define OSINFO_HYPERVISORLIST_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), OSINFO_TYPE_HYPERVISORLIST, OsinfoHypervisorListPrivate))

/**
 * SECTION:osinfo_hypervisorlist
 * @short_description: A list of hypervisor platforms
 * @see_also: #OsinfoList, #OsinfoHypervisor
 *
 * #OsinfoHypervisorList is a list specialization that stores
 * only #OsinfoHypervisor objects.
 */

struct _OsinfoHypervisorListPrivate
{
    gboolean unused;
};

static void
osinfo_hypervisorlist_finalize (GObject *object)
{
    /* Chain up to the parent class */
    G_OBJECT_CLASS (osinfo_hypervisorlist_parent_class)->finalize (object);
}

/* Init functions */
static void
osinfo_hypervisorlist_class_init (OsinfoHypervisorListClass *klass)
{
    GObjectClass *g_klass = G_OBJECT_CLASS (klass);

    g_klass->finalize = osinfo_hypervisorlist_finalize;
    g_type_class_add_private (klass, sizeof (OsinfoHypervisorListPrivate));
}

static void
osinfo_hypervisorlist_init (OsinfoHypervisorList *list)
{
    OsinfoHypervisorListPrivate *priv;
    list->priv = priv = OSINFO_HYPERVISORLIST_GET_PRIVATE(list);

}


/**
 * osinfo_hypervisorlist_new:
 *
 * Construct a new hypervisor list that is initially empty.
 *
 * Returns: (transfer full): an empty hypervisor list
 */
OsinfoHypervisorList *osinfo_hypervisorlist_new(void)
{
    return g_object_new(OSINFO_TYPE_HYPERVISORLIST,
			"element-type", OSINFO_TYPE_HYPERVISOR,
			NULL);
}

/**
 * osinfo_hypervisorlist_new_copy:
 * @source: the hypervisor list to copy
 *
 * Construct a new hypervisor list that is filled with hypervisors
 * from @source
 *
 * Returns: (transfer full): a copy of the hypervisor list
 */
OsinfoHypervisorList *osinfo_hypervisorlist_new_copy(OsinfoHypervisorList *source)
{
    OsinfoHypervisorList *newList = osinfo_hypervisorlist_new();
    osinfo_list_add_all(OSINFO_LIST(newList),
			OSINFO_LIST(source));
    return newList;
}

/**
 * osinfo_hypervisorlist_new_filtered:
 * @source: the hypervisor list to copy
 * @filter: the filter to apply
 *
 * Construct a new hypervisor list that is filled with hypervisors
 * from @source that match @filter
 *
 * Returns: (transfer full): a filtered copy of the hypervisor list
 */
OsinfoHypervisorList *osinfo_hypervisorlist_new_filtered(OsinfoHypervisorList *source, OsinfoFilter *filter)
{
    OsinfoHypervisorList *newList = osinfo_hypervisorlist_new();
    osinfo_list_add_filtered(OSINFO_LIST(newList),
			     OSINFO_LIST(source),
			     filter);
    return newList;
}

/**
 * osinfo_hypervisorlist_new_intersection:
 * @sourceOne: the first hypervisor list to copy
 * @sourceTwo: the second hypervisor list to copy
 *
 * Construct a new hypervisor list that is filled with only the
 * hypervisors that are present in both @sourceOne and @sourceTwo.
 *
 * Returns: (transfer full): an intersection of the two hypervisor lists
 */
OsinfoHypervisorList *osinfo_hypervisorlist_new_intersection(OsinfoHypervisorList *sourceOne, OsinfoHypervisorList *sourceTwo)
{
    OsinfoHypervisorList *newList = osinfo_hypervisorlist_new();
    osinfo_list_add_intersection(OSINFO_LIST(newList),
				 OSINFO_LIST(sourceOne),
				 OSINFO_LIST(sourceTwo));
    return newList;
}

/**
 * osinfo_hypervisorlist_new_union:
 * @sourceOne: the first hypervisor list to copy
 * @sourceTwo: the second hypervisor list to copy
 *
 * Construct a new hypervisor list that is filled with all the
 * hypervisors that are present in either @sourceOne and @sourceTwo.
 *
 * Returns: (transfer full): a union of the two hypervisor lists
 */
OsinfoHypervisorList *osinfo_hypervisorlist_new_union(OsinfoHypervisorList *sourceOne, OsinfoHypervisorList *sourceTwo)
{
    OsinfoHypervisorList *newList = osinfo_hypervisorlist_new();
    osinfo_list_add_union(OSINFO_LIST(newList),
			  OSINFO_LIST(sourceOne),
			  OSINFO_LIST(sourceTwo));
    return newList;
}

/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 * End:
 */
