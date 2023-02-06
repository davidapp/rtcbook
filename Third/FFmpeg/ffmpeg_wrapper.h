#pragma once
#if defined(RX_ENABLE_FFMPEG_INTERFACE)
#include "proxy_interface_pre.h"
#endif

extern "C" {
#if defined(RX_ENABLE_FFMPEG_PREFIX)
#include "rx_ffmpeg.h"
#else
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/base64.h>
#include <libavutil/hwcontext.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
#include <libavutil/parseutils.h>
#include <libavutil/pixfmt.h>
#include <libavutil/time.h>
#if defined(RX_ENABLE_FFMPEG_AVFORMAT)
#include <libavformat/avformat.h>
#endif
#if defined(RX_ENABLE_FFMPEG_SWSCALE)
#include <libswscale/swscale.h>
#endif
#endif
}

#if defined(RX_ENABLE_FFMPEG_INTERFACE)
#include "proxy_interface_post.h"
#endif

#include "proxy_func_name.h"

#if defined(RX_ENABLE_FFMPEG_INTERFACE)
#include <mutex>
namespace realx {
    // APIs of ffmpeg
    struct RXFFmpegAPIs {
       // APIs of ffmpeg::avcodec
       decltype(&undef_avcodec_alloc_context3) _avcodec_alloc_context3;
       decltype(&undef_avcodec_parameters_to_context) _avcodec_parameters_to_context;
       decltype(&undef_avcodec_open2) _avcodec_open2;
       decltype(&undef_av_init_packet) _av_init_packet;
       decltype(&undef_avcodec_close) _avcodec_close;
       decltype(&undef_avcodec_free_context) _avcodec_free_context;
       decltype(&undef_avcodec_decode_video2) _avcodec_decode_video2;
       decltype(&undef_av_free_packet) _av_free_packet;
       decltype(&undef_avpicture_fill) _avpicture_fill;
       decltype(&undef_avpicture_get_size) _avpicture_get_size;
       decltype(&undef_avcodec_align_dimensions) _avcodec_align_dimensions;
       decltype(&undef_avcodec_find_decoder) _avcodec_find_decoder;
       decltype(&undef_avcodec_send_packet) _avcodec_send_packet;
       decltype(&undef_avcodec_receive_frame) _avcodec_receive_frame;
       decltype(&undef_avcodec_find_decoder_by_name) _avcodec_find_decoder_by_name;
       decltype(&undef_avcodec_receive_packet) _avcodec_receive_packet;
       decltype(&undef_av_packet_unref) _av_packet_unref;
       decltype(&undef_avcodec_find_encoder_by_name) _avcodec_find_encoder_by_name;
       decltype(&undef_avcodec_send_frame) _avcodec_send_frame;
       decltype(&undef_av_packet_alloc) _av_packet_alloc;
       decltype(&undef_av_packet_free) _av_packet_free;
       decltype(&undef_avcodec_register_all) _avcodec_register_all;
#if defined(RX_ENABLE_RECORDING)
       decltype(&undef_av_packet_rescale_ts) _av_packet_rescale_ts;
#endif
       // APIs of ffmpeg::avutil
       decltype(&undef_av_base64_decode) _av_base64_decode;
       decltype(&undef_av_frame_alloc) _av_frame_alloc;
       decltype(&undef_av_image_check_size) _av_image_check_size;
       decltype(&undef_av_buffer_create) _av_buffer_create;
       decltype(&undef_av_buffer_get_opaque) _av_buffer_get_opaque;
       decltype(&undef_av_frame_unref) _av_frame_unref;
       decltype(&undef_av_hwdevice_ctx_create) _av_hwdevice_ctx_create;
       decltype(&undef_av_hwframe_ctx_alloc) _av_hwframe_ctx_alloc;
       decltype(&undef_av_hwframe_ctx_init) _av_hwframe_ctx_init;
       decltype(&undef_av_buffer_unref) _av_buffer_unref;
       decltype(&undef_av_hwframe_get_buffer) _av_hwframe_get_buffer;
       decltype(&undef_av_opt_set_int) _av_opt_set_int;
       decltype(&undef_av_frame_free) _av_frame_free;
       decltype(&undef_av_hwdevice_ctx_alloc) _av_hwdevice_ctx_alloc;
       decltype(&undef_av_hwdevice_ctx_init) _av_hwdevice_ctx_init;
       decltype(&undef_av_strerror) _av_strerror;
       decltype(&undef_av_frame_get_buffer) _av_frame_get_buffer;
       decltype(&undef_av_buffer_ref) _av_buffer_ref;
       decltype(&undef_av_frame_copy_props) _av_frame_copy_props;
       decltype(&undef_av_frame_get_side_data) _av_frame_get_side_data;
       decltype(&undef_av_frame_new_side_data) _av_frame_new_side_data;
       decltype(&undef_av_hwframe_transfer_data) _av_hwframe_transfer_data;
       decltype(&undef_av_free) _av_free;
       decltype(&undef_av_malloc) _av_malloc;
       decltype(&undef_av_image_get_buffer_size) _av_image_get_buffer_size;
       decltype(&undef_av_image_copy_to_buffer) _av_image_copy_to_buffer;
#if defined(RX_ENABLE_RECORDING)
       decltype(&undef_av_dict_set) _av_dict_set;
       decltype(&undef_av_freep) _av_freep;
       decltype(&undef_av_get_default_channel_layout) _av_get_default_channel_layout;
       decltype(&undef_av_gettime_relative) _av_gettime_relative;
       decltype(&undef_av_mallocz) _av_mallocz;
#endif
#if defined(RX_ENABLE_FFMPEG_AVFORMAT)
       // APIs of ffmpeg::avformat
       decltype(&undef_av_register_all) _av_register_all;
       decltype(&undef_avformat_network_init) _avformat_network_init;
       decltype(&undef_avformat_open_input) _avformat_open_input;
       decltype(&undef_avformat_find_stream_info) _avformat_find_stream_info;
       decltype(&undef_avformat_close_input) _avformat_close_input;
       decltype(&undef_av_find_best_stream) _av_find_best_stream;
       decltype(&undef_av_read_frame) _av_read_frame;
       decltype(&undef_av_seek_frame) _av_seek_frame;
#if defined(RX_ENABLE_RECORDING)
       decltype(&undef_av_interleaved_write_frame) _av_interleaved_write_frame;
       decltype(&undef_av_write_trailer) _av_write_trailer;
       decltype(&undef_avformat_alloc_output_context2) _avformat_alloc_output_context2;
       decltype(&undef_avformat_free_context) _avformat_free_context;
       decltype(&undef_avformat_new_stream) _avformat_new_stream;
       decltype(&undef_avformat_write_header) _avformat_write_header;
       decltype(&undef_avio_closep) _avio_closep;
       decltype(&undef_avio_open) _avio_open;
#endif
#endif
#if defined(RX_ENABLE_FFMPEG_SWSCALE)
       // APIs of ffmpeg::swscale
       decltype(&undef_sws_getContext) _sws_getContext;
       decltype(&undef_sws_scale) _sws_scale;
       decltype(&undef_sws_freeContext) _sws_freeContext;
#endif
    };
    const RXFFmpegAPIs*  ffmpeg_api();
}  // namespace realx
#endif