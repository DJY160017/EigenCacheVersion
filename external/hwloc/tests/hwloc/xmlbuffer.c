/*
 * Copyright © 2010-2018 Inria.  All rights reserved.
 * Copyright © 2011 Cisco Systems, Inc.  All rights reserved.
 * See COPYING in top-level directory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <hwloc.h>

static int one_test(void)
{
  hwloc_topology_t topology;
  int size1, size2;
  char *buf1, *copy1, *buf2;
  int err = 0, i;
  char s[129];
  char t[10];

  for(i=0; i<128; i++)
    s[i] = ' ';
  s[128] = 0;
  for(i=32; i<=126; i++)
    s[i] = i;
  s['\t'] = '\t';
  s['\n'] = '\n';
  s['\r'] = '\r';

  t[0] = 'x';
  for(i=1; i<=7; i++)
    t[i] = i;
  t[8] = 'y';
  t[9] = '\0';

  hwloc_topology_init(&topology);
  hwloc_topology_set_all_types_filter(topology, HWLOC_TYPE_FILTER_KEEP_ALL);
  hwloc_topology_load(topology);
  assert(hwloc_topology_is_thissystem(topology));
  hwloc_obj_add_info(hwloc_get_root_obj(topology), "UglyString", s);
  hwloc_obj_add_info(hwloc_get_root_obj(topology), "UberUglyString", t);
  hwloc_topology_export_xmlbuffer(topology, &buf1, &size1, 0);
  hwloc_topology_destroy(topology);
  printf("exported to buffer %p length %d\n", buf1, size1);

  /* copy the returned buffer to a newly malloc'd one
   * to check that the returned length is correct (contains ending 0, etc).
   */
  copy1 = malloc(size1);
  assert(copy1);
  memcpy(copy1, buf1, size1);

  hwloc_topology_init(&topology);
  hwloc_topology_set_all_types_filter(topology, HWLOC_TYPE_FILTER_KEEP_ALL);
  assert(!hwloc_topology_set_xmlbuffer(topology, copy1, size1));
  hwloc_topology_load(topology);
  assert(!hwloc_topology_is_thissystem(topology));
  if (strcmp(hwloc_obj_get_info_by_name(hwloc_get_root_obj(topology), "UglyString"), s))
    assert(0);
  if (strcmp(hwloc_obj_get_info_by_name(hwloc_get_root_obj(topology), "UberUglyString"), "xy"))
    assert(0);
  hwloc_topology_export_xmlbuffer(topology, &buf2, &size2, 0);
  printf("re-exported to buffer %p length %d\n", buf2, size2);

  if (strcmp(buf1, buf2)) {
    printf("### First exported buffer is:\n");
    printf("%s", buf1);
    printf("### End of first export buffer\n");
    printf("### Second exported buffer is:\n");
    printf("%s", buf2);
    printf("### End of second export buffer\n");
    err = 1;
  }

  hwloc_free_xmlbuffer(topology, buf1);
  hwloc_free_xmlbuffer(topology, buf2);
  free(copy1);

  hwloc_topology_destroy(topology);

  return err;
}

int main(int argc, char *argv[])
{
  int err;

  if (argc < 3) {
    fprintf(stderr, "Need 0 or 1 twice as arguments for enabling/disabling libxml import and export\n");
    fprintf(stderr, "For instance `xmlbuffer 0 1' enables libxml for export only\n");
    fprintf(stderr, "Those arguments are passed by wrapper.sh during make check\n");
    exit(EXIT_FAILURE);
  }

  putenv("HWLOC_LIBXML_CLEANUP=1");

  if (atoi(argv[1])) {
    putenv("HWLOC_LIBXML_IMPORT=1");
    printf("import=libxml   ");
  } else {
    putenv("HWLOC_LIBXML_IMPORT=0");
    printf("import=nolibxml ");
  }

  if (atoi(argv[2])) {
    putenv("HWLOC_LIBXML_EXPORT=1");
    printf("export=libxml\n");
  } else {
    putenv("HWLOC_LIBXML_EXPORT=0");
    printf("export=nolibxml\n");
  }

  err = one_test();
  if (err < 0)
    return err;

  return 0;
}
