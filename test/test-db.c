#include <stdlib.h>
#include <osinfo/osinfo.h>
#include <check.h>

START_TEST(test_basic)
{
  OsinfoDb *db = osinfo_db_new("../data");

  fail_unless(OSINFO_IS_DB(db), "Db is not a DB");

  gchar *dir = NULL;
  g_object_get(db, "backing-dir", &dir, NULL);
  fail_unless(g_strcmp0(dir, "../data") == 0, "Backing dir missing");
  g_free(dir);

  g_object_unref(db);
}
END_TEST


START_TEST(test_device)
{
  OsinfoDb *db = osinfo_db_new("../data");
  OsinfoDevice *dev1 = osinfo_device_new("dev1");
  OsinfoDevice *dev2 = osinfo_device_new("dev2");
  OsinfoDevice *dev3 = osinfo_device_new("dev3");

  OsinfoDeviceList *list = osinfo_db_get_device_list(db);

  osinfo_list_add(OSINFO_LIST(list), OSINFO_ENTITY(dev1));
  osinfo_list_add(OSINFO_LIST(list), OSINFO_ENTITY(dev2));
  osinfo_list_add(OSINFO_LIST(list), OSINFO_ENTITY(dev3));

  OsinfoDevice *dev = osinfo_db_get_device(db, "dev2");
  fail_unless(dev != NULL, "Device is NULL");
  fail_unless(dev == dev2, "Device was not dev2");

  g_object_unref(dev1);
  g_object_unref(dev2);
  g_object_unref(dev3);
  g_object_unref(db);
}
END_TEST


START_TEST(test_hypervisor)
{
  OsinfoDb *db = osinfo_db_new("../data");
  OsinfoHypervisor *dev1 = osinfo_hypervisor_new("dev1");
  OsinfoHypervisor *dev2 = osinfo_hypervisor_new("dev2");
  OsinfoHypervisor *dev3 = osinfo_hypervisor_new("dev3");

  OsinfoHypervisorList *list = osinfo_db_get_hypervisor_list(db);

  osinfo_list_add(OSINFO_LIST(list), OSINFO_ENTITY(dev1));
  osinfo_list_add(OSINFO_LIST(list), OSINFO_ENTITY(dev2));
  osinfo_list_add(OSINFO_LIST(list), OSINFO_ENTITY(dev3));

  OsinfoHypervisor *dev = osinfo_db_get_hypervisor(db, "dev2");
  fail_unless(dev != NULL, "Hypervisor is NULL");
  fail_unless(dev == dev2, "Hypervisor was not dev2");

  g_object_unref(dev1);
  g_object_unref(dev2);
  g_object_unref(dev3);
  g_object_unref(db);
}
END_TEST


START_TEST(test_os)
{
  OsinfoDb *db = osinfo_db_new("../data");
  OsinfoOs *dev1 = osinfo_os_new("dev1");
  OsinfoOs *dev2 = osinfo_os_new("dev2");
  OsinfoOs *dev3 = osinfo_os_new("dev3");

  OsinfoOsList *list = osinfo_db_get_os_list(db);

  osinfo_list_add(OSINFO_LIST(list), OSINFO_ENTITY(dev1));
  osinfo_list_add(OSINFO_LIST(list), OSINFO_ENTITY(dev2));
  osinfo_list_add(OSINFO_LIST(list), OSINFO_ENTITY(dev3));

  OsinfoOs *dev = osinfo_db_get_os(db, "dev2");
  fail_unless(dev != NULL, "Os is NULL");
  fail_unless(dev == dev2, "Os was not dev2");

  g_object_unref(dev1);
  g_object_unref(dev2);
  g_object_unref(dev3);
  g_object_unref(db);
}
END_TEST



START_TEST(test_prop_device)
{
  OsinfoDb *db = osinfo_db_new("../data");
  OsinfoDevice *dev1 = osinfo_device_new("dev1");
  OsinfoDevice *dev2 = osinfo_device_new("dev2");
  OsinfoDevice *dev3 = osinfo_device_new("dev3");

  osinfo_entity_add_param(OSINFO_ENTITY(dev1), "class", "network");
  osinfo_entity_add_param(OSINFO_ENTITY(dev1), "class", "audio");
  osinfo_entity_add_param(OSINFO_ENTITY(dev2), "class", "input");
  osinfo_entity_add_param(OSINFO_ENTITY(dev3), "class", "display");
  osinfo_entity_add_param(OSINFO_ENTITY(dev3), "fruit", "apple");

  OsinfoDeviceList *list = osinfo_db_get_device_list(db);

  osinfo_list_add(OSINFO_LIST(list), OSINFO_ENTITY(dev1));
  osinfo_list_add(OSINFO_LIST(list), OSINFO_ENTITY(dev2));
  osinfo_list_add(OSINFO_LIST(list), OSINFO_ENTITY(dev3));

  GList *uniq = osinfo_db_unique_values_for_property_in_dev(db, "class");
  GList *tmp = uniq;
  gboolean hasNetwork = FALSE;
  gboolean hasAudio = FALSE;
  gboolean hasInput = FALSE;
  gboolean hasDisplay = FALSE;
  gboolean hasBad = FALSE;
  while (tmp) {
    if (g_strcmp0(tmp->data, "network") == 0)
      hasNetwork = TRUE;
    else if (g_strcmp0(tmp->data, "audio") == 0)
      hasAudio = TRUE;
    else if (g_strcmp0(tmp->data, "input") == 0)
      hasInput = TRUE;
    else if (g_strcmp0(tmp->data, "display") == 0)
      hasDisplay = TRUE;
    else
      hasBad = TRUE;
    tmp = tmp->next;
  }
  fail_unless(hasNetwork, "Missing network");
  fail_unless(hasAudio, "Missing audio");
  fail_unless(hasInput, "Missing input");
  fail_unless(hasDisplay, "Missing display");
  fail_unless(!hasBad, "Unexpected property");

  g_list_free(uniq);

  g_object_unref(dev1);
  g_object_unref(dev2);
  g_object_unref(dev3);
  g_object_unref(db);
}
END_TEST


START_TEST(test_prop_hypervisor)
{
  OsinfoDb *db = osinfo_db_new("../data");
  OsinfoHypervisor *hv1 = osinfo_hypervisor_new("hv1");
  OsinfoHypervisor *hv2 = osinfo_hypervisor_new("hv2");
  OsinfoHypervisor *hv3 = osinfo_hypervisor_new("hv3");

  osinfo_entity_add_param(OSINFO_ENTITY(hv1), "vendor", "acme");
  osinfo_entity_add_param(OSINFO_ENTITY(hv1), "vendor", "frog");
  osinfo_entity_add_param(OSINFO_ENTITY(hv2), "vendor", "acme");
  osinfo_entity_add_param(OSINFO_ENTITY(hv3), "vendor", "dog");
  osinfo_entity_add_param(OSINFO_ENTITY(hv3), "arch", "x86");

  OsinfoHypervisorList *list = osinfo_db_get_hypervisor_list(db);

  osinfo_list_add(OSINFO_LIST(list), OSINFO_ENTITY(hv1));
  osinfo_list_add(OSINFO_LIST(list), OSINFO_ENTITY(hv2));
  osinfo_list_add(OSINFO_LIST(list), OSINFO_ENTITY(hv3));

  GList *uniq = osinfo_db_unique_values_for_property_in_hv(db, "vendor");
  GList *tmp = uniq;
  gboolean hasAcme = FALSE;
  gboolean hasFrog = FALSE;
  gboolean hasDog = FALSE;
  gboolean hasBad = FALSE;
  while (tmp) {
    if (g_strcmp0(tmp->data, "acme") == 0)
      hasAcme = TRUE;
    else if (g_strcmp0(tmp->data, "frog") == 0)
      hasFrog = TRUE;
    else if (g_strcmp0(tmp->data, "dog") == 0)
      hasDog = TRUE;
    else
      hasBad = TRUE;
    tmp = tmp->next;
  }
  fail_unless(hasAcme, "Missing acme");
  fail_unless(hasFrog, "Missing frog");
  fail_unless(hasDog, "Missing dog");
  fail_unless(!hasBad, "Unexpected property");

  g_list_free(uniq);

  g_object_unref(hv1);
  g_object_unref(hv2);
  g_object_unref(hv3);
  g_object_unref(db);
}
END_TEST


START_TEST(test_prop_os)
{
  OsinfoDb *db = osinfo_db_new("../data");
  OsinfoOs *os1 = osinfo_os_new("os1");
  OsinfoOs *os2 = osinfo_os_new("os2");
  OsinfoOs *os3 = osinfo_os_new("os3");

  osinfo_entity_add_param(OSINFO_ENTITY(os1), "vendor", "acme");
  osinfo_entity_add_param(OSINFO_ENTITY(os1), "vendor", "frog");
  osinfo_entity_add_param(OSINFO_ENTITY(os2), "vendor", "acme");
  osinfo_entity_add_param(OSINFO_ENTITY(os3), "vendor", "dog");
  osinfo_entity_add_param(OSINFO_ENTITY(os3), "arch", "x86");


  OsinfoOsList *list = osinfo_db_get_os_list(db);

  osinfo_list_add(OSINFO_LIST(list), OSINFO_ENTITY(os1));
  osinfo_list_add(OSINFO_LIST(list), OSINFO_ENTITY(os2));
  osinfo_list_add(OSINFO_LIST(list), OSINFO_ENTITY(os3));

  GList *uniq = osinfo_db_unique_values_for_property_in_os(db, "vendor");
  GList *tmp = uniq;
  gboolean hasAcme = FALSE;
  gboolean hasFrog = FALSE;
  gboolean hasDog = FALSE;
  gboolean hasBad = FALSE;
  while (tmp) {
    if (g_strcmp0(tmp->data, "acme") == 0)
      hasAcme = TRUE;
    else if (g_strcmp0(tmp->data, "frog") == 0)
      hasFrog = TRUE;
    else if (g_strcmp0(tmp->data, "dog") == 0)
      hasDog = TRUE;
    else
      hasBad = TRUE;
    tmp = tmp->next;
  }
  fail_unless(hasAcme, "Missing acme");
  fail_unless(hasFrog, "Missing frog");
  fail_unless(hasDog, "Missing dog");
  fail_unless(!hasBad, "Unexpected property");

  g_list_free(uniq);

  g_object_unref(os1);
  g_object_unref(os2);
  g_object_unref(os3);
  g_object_unref(db);
}
END_TEST



START_TEST(test_rel_os)
{
  OsinfoDb *db = osinfo_db_new("../data");
  OsinfoOs *os1 = osinfo_os_new("os1");
  OsinfoOs *os2 = osinfo_os_new("os2");
  OsinfoOs *os3 = osinfo_os_new("os3");
  OsinfoOs *os4 = osinfo_os_new("os4");
  OsinfoOs *os5 = osinfo_os_new("os5");

  OsinfoOsList *list = osinfo_db_get_os_list(db);

  osinfo_list_add(OSINFO_LIST(list), OSINFO_ENTITY(os1));
  osinfo_list_add(OSINFO_LIST(list), OSINFO_ENTITY(os2));
  osinfo_list_add(OSINFO_LIST(list), OSINFO_ENTITY(os3));
  osinfo_list_add(OSINFO_LIST(list), OSINFO_ENTITY(os4));
  osinfo_list_add(OSINFO_LIST(list), OSINFO_ENTITY(os5));

  osinfo_os_add_related_os(os1, OSINFO_OS_RELATIONSHIP_DERIVES_FROM, os2);
  osinfo_os_add_related_os(os1, OSINFO_OS_RELATIONSHIP_DERIVES_FROM, os3);
  osinfo_os_add_related_os(os2, OSINFO_OS_RELATIONSHIP_CLONES, os4);
  osinfo_os_add_related_os(os3, OSINFO_OS_RELATIONSHIP_UPGRADES, os5);

  OsinfoOsList *sublist = osinfo_db_unique_values_for_os_relationship(db, OSINFO_OS_RELATIONSHIP_DERIVES_FROM);
  gboolean hasOs1 = FALSE;
  gboolean hasOs2 = FALSE;
  gboolean hasOs3 = FALSE;
  gboolean hasOs4 = FALSE;
  gboolean hasOs5 = FALSE;
  gboolean hasBad = FALSE;
  int i;
  for (i = 0 ; i < osinfo_list_get_length(OSINFO_LIST(sublist)) ; i++) {
    OsinfoOs *ent = OSINFO_OS(osinfo_list_get_nth(OSINFO_LIST(sublist), i));

    if (ent == os1)
      hasOs1 = TRUE;
    else if (ent == os2)
      hasOs2 = TRUE;
    else if (ent == os3)
      hasOs3 = TRUE;
    else if (ent == os4)
      hasOs4 = TRUE;
    else if (ent == os5)
      hasOs5 = TRUE;
    else
      hasBad = TRUE;

  }
  fail_unless(!hasOs1, "Unexpected OS 1");
  fail_unless(hasOs2, "Missing OS 2");
  fail_unless(hasOs3, "Missing OS 3");
  fail_unless(!hasOs4, "Unexpected OS 4");
  fail_unless(!hasOs5, "Unexpected OS 5");
  fail_unless(!hasBad, "Unexpected property");

  g_object_unref(sublist);

  sublist = osinfo_db_unique_values_for_os_relationship(db, OSINFO_OS_RELATIONSHIP_UPGRADES);
  hasOs1 = hasOs2 = hasOs3 = hasOs4 = hasOs5 = hasBad = FALSE;
  for (i = 0 ; i < osinfo_list_get_length(OSINFO_LIST(sublist)) ; i++) {
    OsinfoOs *ent = OSINFO_OS(osinfo_list_get_nth(OSINFO_LIST(sublist), i));

    if (ent == os1)
      hasOs1 = TRUE;
    else if (ent == os2)
      hasOs2 = TRUE;
    else if (ent == os3)
      hasOs3 = TRUE;
    else if (ent == os4)
      hasOs4 = TRUE;
    else if (ent == os5)
      hasOs5 = TRUE;
    else
      hasBad = TRUE;

  }
  fail_unless(!hasOs1, "Unexpected OS 1");
  fail_unless(!hasOs2, "Unexpected OS 2");
  fail_unless(!hasOs3, "Unexpected OS 3");
  fail_unless(!hasOs4, "Unexpected OS 4");
  fail_unless(hasOs5, "Missing OS 5");
  fail_unless(!hasBad, "Unexpected property");

  g_object_unref(sublist);

  sublist = osinfo_db_unique_values_for_os_relationship(db, OSINFO_OS_RELATIONSHIP_CLONES);
  hasOs1 = hasOs2 = hasOs3 = hasOs4 = hasOs5 = hasBad = FALSE;
  for (i = 0 ; i < osinfo_list_get_length(OSINFO_LIST(sublist)) ; i++) {
    OsinfoOs *ent = OSINFO_OS(osinfo_list_get_nth(OSINFO_LIST(sublist), i));

    if (ent == os1)
      hasOs1 = TRUE;
    else if (ent == os2)
      hasOs2 = TRUE;
    else if (ent == os3)
      hasOs3 = TRUE;
    else if (ent == os4)
      hasOs4 = TRUE;
    else if (ent == os5)
      hasOs5 = TRUE;
    else
      hasBad = TRUE;

  }
  fail_unless(!hasOs1, "Unexpected OS 1");
  fail_unless(!hasOs2, "Unexpected OS 2");
  fail_unless(!hasOs3, "Unexpected OS 3");
  fail_unless(hasOs4, "Missing OS 4");
  fail_unless(!hasOs5, "Unexpected OS 5");
  fail_unless(!hasBad, "Unexpected property");

  g_object_unref(sublist);


  g_object_unref(os1);
  g_object_unref(os2);
  g_object_unref(os3);
  g_object_unref(os4);
  g_object_unref(os5);
  g_object_unref(db);
}
END_TEST




static Suite *
list_suite(void)
{
  Suite *s = suite_create("List");
  TCase *tc = tcase_create("Core");
  tcase_add_test(tc, test_basic);
  tcase_add_test(tc, test_device);
  tcase_add_test(tc, test_hypervisor);
  tcase_add_test(tc, test_os);
  tcase_add_test(tc, test_prop_device);
  tcase_add_test(tc, test_prop_hypervisor);
  tcase_add_test(tc, test_prop_os);
  tcase_add_test(tc, test_rel_os);
  suite_add_tcase(s, tc);
  return s;
}

int main(void)
{
  int number_failed;
  Suite *s = list_suite ();
  SRunner *sr = srunner_create (s);

  g_type_init();

  /* Upfront so we don't confuse valgrind */
  osinfo_entity_get_type();
  osinfo_db_get_type();
  osinfo_device_get_type();
  osinfo_hypervisor_get_type();
  osinfo_os_get_type();
  osinfo_list_get_type();
  osinfo_devicelist_get_type();
  osinfo_hypervisorlist_get_type();
  osinfo_oslist_get_type();
  osinfo_filter_get_type();

  srunner_run_all (sr, CK_ENV);
  number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);

  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
