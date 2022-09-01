#ifndef _ANALYTICS_H_
#define _ANALYTICS_H_

#include <gst/gst.h>

typedef struct NvDsPeopleNet {
  guint32 count;
  guint32 steam_id;
} NvDsPeopleNet;

/* User defined */
typedef struct {
  guint32 people_counting;
  guint32 source_id;
} AnalyticsUserMeta;

#endif
