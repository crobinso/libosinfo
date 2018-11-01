
EXTRA_DIST = isodata dbdata

check_PROGRAMS = \
	test-entity \
	test-device \
	test-list \
	test-devicelist \
	test-devicelinklist \
	test-filter \
	test-product \
	test-os \
	test-oslist \
	test-productfilter \
	test-platform \
	test-platformlist \
	test-db \
	test-loader \
	test-isodetect \
	test-install-script \
	test-image \
        $(NULL)

if HAVE_CURL
check_PROGRAMS += \
	test-mediauris \
	test-treeuris \
	$(NULL)
endif

COMMON_LDADD = \
	$(COVERAGE_LDFLAGS) \
	$(GLIB_LIBS) \
	$(GOBJECT_LIBS) \
	../osinfo/libosinfo-impl.la
COMMON_CFLAGS = \
	$(WARN_CFLAGS) \
	$(COVERAGE_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(GOBJECT_CFLAGS) \
	-I$(top_srcdir) \
	-DSRCDIR="\"$(abs_top_srcdir)\"" \
	-DBUILDDIR="\"$(abs_top_builddir)\"" \
	$(NULL)

test_entity_LDADD = $(COMMON_LDADD)
test_entity_CFLAGS = $(COMMON_CFLAGS)
test_entity_SOURCES = test-entity.c

test_device_LDADD = $(COMMON_LDADD)
test_device_CFLAGS = $(COMMON_CFLAGS)
test_device_SOURCES = test-device.c

test_filter_LDADD = $(COMMON_LDADD)
test_filter_CFLAGS = $(COMMON_CFLAGS)
test_filter_SOURCES = test-filter.c

test_product_LDADD = $(COMMON_LDADD)
test_product_CFLAGS = $(COMMON_CFLAGS)
test_product_SOURCES = test-product.c

test_os_LDADD = $(COMMON_LDADD)
test_os_CFLAGS = $(COMMON_CFLAGS)
test_os_SOURCES = test-os.c

test_productfilter_LDADD = $(COMMON_LDADD)
test_productfilter_CFLAGS = $(COMMON_CFLAGS)
test_productfilter_SOURCES = test-productfilter.c

test_platform_LDADD = $(COMMON_LDADD)
test_platform_CFLAGS = $(COMMON_CFLAGS)
test_platform_SOURCES = test-platform.c

test_list_LDADD = $(COMMON_LDADD)
test_list_CFLAGS = $(COMMON_CFLAGS)
test_list_SOURCES = test-list.c

test_devicelist_LDADD = $(COMMON_LDADD)
test_devicelist_CFLAGS = $(COMMON_CFLAGS)
test_devicelist_SOURCES = test-devicelist.c

test_devicelinklist_LDADD = $(COMMON_LDADD)
test_devicelinklist_CFLAGS = $(COMMON_CFLAGS)
test_devicelinklist_SOURCES = test-devicelinklist.c

test_platformlist_LDADD = $(COMMON_LDADD)
test_platformlist_CFLAGS = $(COMMON_CFLAGS)
test_platformlist_SOURCES = test-platformlist.c

test_oslist_LDADD = $(COMMON_LDADD)
test_oslist_CFLAGS = $(COMMON_CFLAGS)
test_oslist_SOURCES = test-oslist.c

test_db_LDADD = $(COMMON_LDADD)
test_db_CFLAGS = $(COMMON_CFLAGS)
test_db_SOURCES = test-db.c

test_loader_LDADD = $(COMMON_LDADD)
test_loader_CFLAGS = $(COMMON_CFLAGS)
test_loader_SOURCES = test-loader.c

test_isodetect_LDADD = $(COMMON_LDADD)
test_isodetect_CFLAGS = $(COMMON_CFLAGS)
test_isodetect_SOURCES = test-isodetect.c

if HAVE_CURL
test_mediauris_LDADD = $(COMMON_LDADD) $(CURL_LIBS)
test_mediauris_CFLAGS = $(COMMON_CFLAGS) $(CURL_CFLAGS)
test_mediauris_SOURCES = test-mediauris.c

test_treeuris_LDADD = $(COMMON_LDADD) $(CURL_LIBS)
test_treeuris_CFLAGS = $(COMMON_CFLAGS) $(CURL_CFLAGS)
test_treeuris_SOURCES = test-treeuris.c
endif

test_install_script_LDADD = $(COMMON_LDADD)
test_install_script_CFLAGS = $(COMMON_CFLAGS)
test_install_script_SOURCES = test-install-script.c

test_image_LDADD = $(COMMON_LDADD)
test_image_CFLAGS = $(COMMON_CFLAGS)
test_image_SOURCES = test-image.c

TESTS = $(check_PROGRAMS) \
	$(NULL)

EXTRA_DIST += \
	test-lib.sh \
	install-script.xsl \
	$(NULL)

TESTS_ENVIRONMENT =						\
	abs_top_builddir=$(lv_abs_top_builddir)			\
	abs_top_srcdir=`cd '$(top_srcdir)'; pwd`		\
	abs_builddir=`pwd`					\
	abs_srcdir=`cd '$(srcdir)'; pwd`			\
	CONFIG_HEADER="`cd '$(top_builddir)'; pwd`/config.h"	\
	PATH="$(path_add)$(PATH_SEPARATOR)$$PATH"		\
	SHELL="$(SHELL)"					\
	LIBVIRT_DRIVER_DIR="$(abs_top_builddir)/src/.libs"	\
	LC_ALL=C						\
	G_SLICE=always-malloc G_DEBUG=gc-friendly		\
	$(VG)

valgrind:
	$(MAKE) check VG="libtool --mode=execute valgrind --quiet --tool=memcheck --leak-check=full --leak-resolution=high --num-callers=20 --suppressions=osinfo.suppression"
