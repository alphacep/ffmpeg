/*
 * DVB Teletext subtitle encoder
 * Copyright (c) 2024 Alpha Cephei Inc.
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "config_components.h"

#include <stdarg.h>
#include "avcodec.h"
#include "ass_split.h"
#include "ass.h"
#include "libavutil/bprint.h"
#include "codec_internal.h"


typedef struct {
    AVCodecContext *avctx;
    ASSSplitContext *ass_ctx;
} DvbTxtContext;


static av_cold int dvbtxt_encode_init(AVCodecContext *avctx)
{
    DvbTxtContext *s = avctx->priv_data;
    s->avctx = avctx;
    s->avctx = avctx;
    s->ass_ctx = ff_ass_split(avctx->subtitle_header);
    return s->ass_ctx ? 0 : AVERROR_INVALIDDATA;
}

static int dvbtxt_encode_frame(AVCodecContext *avctx,
                             unsigned char *buf, int bufsize, const AVSubtitle *sub)
{
    DvbTxtContext *s = avctx->priv_data;
    ASSDialog *dialog;
    int i;

    for (i = 0; i < sub->num_rects; i++) {
        const char *ass = sub->rects[i]->ass;

        if (sub->rects[i]->type != SUBTITLE_ASS) {
            av_log(avctx, AV_LOG_ERROR, "Only SUBTITLE_ASS type supported.\n");
            return AVERROR(EINVAL);
        }

        dialog = ff_ass_split_dialog(s->ass_ctx, ass);
        if (!dialog)
            return AVERROR(ENOMEM);
        printf ("Encoding rec %d text %s\n", i, dialog->text);
        ff_ass_free_dialog(&dialog);
    }

    return 0;
}

static int dvbtxt_encode_close(AVCodecContext *avctx)
{
    DvbTxtContext *s = avctx->priv_data;
    ff_ass_split_free(s->ass_ctx);
    return 0;
}

const FFCodec ff_teletext_encoder = {
    .p.name         = "dvbtxt",
    CODEC_LONG_NAME("DVB Teletext Encoder"),
    .p.type         = AVMEDIA_TYPE_SUBTITLE,
    .p.id           = AV_CODEC_ID_DVB_TELETEXT,
    .priv_data_size = sizeof(DvbTxtContext),
    .init           = dvbtxt_encode_init,
    FF_CODEC_ENCODE_SUB_CB(dvbtxt_encode_frame),
    .close          = dvbtxt_encode_close,
};
