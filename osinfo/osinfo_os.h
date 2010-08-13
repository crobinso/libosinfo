/*
 * libosinfo
 *
 * osinfo_os.h
 * Represents an operating system in libosinfo.
 */

#ifndef __OSINFO_OS_H__
#define __OSINFO_OS_H__

#include <glib-object.h>
#include "osinfo_oslist.h"
#include "osinfo_devicelist.h"

/*
 * Type macros.
 */
#define OSINFO_TYPE_OS                  (osinfo_os_get_type ())
#define OSINFO_OS(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), OSINFO_TYPE_OS, OsinfoOs))
#define OSINFO_IS_OS(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), OSINFO_TYPE_OS))
#define OSINFO_OS_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), OSINFO_TYPE_OS, OsinfoOsClass))
#define OSINFO_IS_OS_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), OSINFO_TYPE_OS))
#define OSINFO_OS_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), OSINFO_TYPE_OS, OsinfoOsClass))

//typedef struct _OsinfoOs        OsinfoOs;
// (defined in osinfo_objects.h)

typedef struct _OsinfoOsClass   OsinfoOsClass;

typedef struct _OsinfoOsPrivate OsinfoOsPrivate;

/* object */
struct _OsinfoOs
{
    OsinfoEntity parent_instance;

    /* public */

    /* private */
    OsinfoOsPrivate *priv;
};

/* class */
struct _OsinfoOsClass
{
    OsinfoEntityClass parent_class;

    /* class members */
};

GType osinfo_os_get_type(void);

OsinfoDevice *osinfo_os_get_preferred_device(OsinfoOs *self, OsinfoHypervisor *hv, gchar *devType, OsinfoFilter *filter, GError **err);
OsinfoOsList *osinfo_os_get_related(OsinfoOs *self, osinfoRelationship relshp, GError **err);
OsinfoDeviceList *osinfo_os_get_devices(OsinfoOs *self, OsinfoHypervisor *hv, gchar *devType, OsinfoFilter *filter, GError **err);

#endif /* __OSINFO_OS_H__ */