/*
 *
 *
 *  Copyright (C) 2005 Mike Isely <isely@pobox.com>
 *  Copyright (C) 2004 Aurelien Alleaume <slts@free.fr>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/*

   This source file is specifically designed to interface with the
   cx2584x, in kernels 2.6.16 or newer.

*/

#include "pvrusb2-cx2584x-v4l.h"
#include "pvrusb2-video-v4l.h"


#include "pvrusb2-hdw-internal.h"
#include "pvrusb2-debug.h"
#include <media/cx25840.h>
#include <linux/videodev2.h>
#include <media/v4l2-common.h>
#include <linux/errno.h>
#include <linux/slab.h>


struct routing_scheme_item {
	int vid;
	int aud;
};

struct routing_scheme {
	const struct routing_scheme_item *def;
	unsigned int cnt;
};

static const struct routing_scheme_item routing_scheme0[] = {
	[PVR2_CVAL_INPUT_TV] = {
		.vid = CX25840_COMPOSITE7,
		.aud = CX25840_AUDIO8,
	},
	[PVR2_CVAL_INPUT_RADIO] = { /* Treat the same as composite */
		.vid = CX25840_COMPOSITE3,
		.aud = CX25840_AUDIO_SERIAL,
	},
	[PVR2_CVAL_INPUT_COMPOSITE] = {
		.vid = CX25840_COMPOSITE3,
		.aud = CX25840_AUDIO_SERIAL,
	},
	[PVR2_CVAL_INPUT_SVIDEO] = {
		.vid = CX25840_SVIDEO1,
		.aud = CX25840_AUDIO_SERIAL,
	},
};

/* Specific to gotview device */
static const struct routing_scheme_item routing_schemegv[] = {
	[PVR2_CVAL_INPUT_TV] = {
		.vid = CX25840_COMPOSITE2,
		.aud = CX25840_AUDIO5,
	},
	[PVR2_CVAL_INPUT_RADIO] = {
		/* line-in is used for radio and composite.  A GPIO is
		   used to switch between the two choices. */
		.vid = CX25840_COMPOSITE1,
		.aud = CX25840_AUDIO_SERIAL,
	},
	[PVR2_CVAL_INPUT_COMPOSITE] = {
		.vid = CX25840_COMPOSITE1,
		.aud = CX25840_AUDIO_SERIAL,
	},
	[PVR2_CVAL_INPUT_SVIDEO] = {
		.vid = (CX25840_SVIDEO_LUMA3|CX25840_SVIDEO_CHROMA4),
		.aud = CX25840_AUDIO_SERIAL,
	},
};

static const struct routing_scheme routing_schemes[] = {
	[PVR2_ROUTING_SCHEME_HAUPPAUGE] = {
		.def = routing_scheme0,
		.cnt = ARRAY_SIZE(routing_scheme0),
	},
	[PVR2_ROUTING_SCHEME_GOTVIEW] = {
		.def = routing_schemegv,
		.cnt = ARRAY_SIZE(routing_schemegv),
	},
};

void pvr2_cx25840_subdev_update(struct pvr2_hdw *hdw, struct v4l2_subdev *sd)
{
	pvr2_trace(PVR2_TRACE_CHIPS, "subdev cx2584x update...");
	if (hdw->input_dirty || hdw->force_dirty) {
		struct v4l2_routing route;
		enum cx25840_video_input vid_input;
		enum cx25840_audio_input aud_input;
		const struct routing_scheme *sp;
		unsigned int sid = hdw->hdw_desc->signal_routing_scheme;

		memset(&route, 0, sizeof(route));

		if ((sid < ARRAY_SIZE(routing_schemes)) &&
		    ((sp = routing_schemes + sid) != NULL) &&
		    (hdw->input_val >= 0) &&
		    (hdw->input_val < sp->cnt)) {
			vid_input = sp->def[hdw->input_val].vid;
			aud_input = sp->def[hdw->input_val].aud;
		} else {
			pvr2_trace(PVR2_TRACE_ERROR_LEGS,
				   "*** WARNING *** subdev cx2584x set_input:"
				   " Invalid routing scheme (%u)"
				   " and/or input (%d)",
				   sid, hdw->input_val);
			return;
		}

		pvr2_trace(PVR2_TRACE_CHIPS,
			   "subdev cx2584x set_input vid=0x%x aud=0x%x",
			   vid_input, aud_input);
		route.input = (u32)vid_input;
		sd->ops->video->s_routing(sd, &route);
		route.input = (u32)aud_input;
		sd->ops->audio->s_routing(sd, &route);
	}
}



/*
  Stuff for Emacs to see, in order to encourage consistent editing style:
  *** Local Variables: ***
  *** mode: c ***
  *** fill-column: 70 ***
  *** tab-width: 8 ***
  *** c-basic-offset: 8 ***
  *** End: ***
  */
