#if defined(RX_ENABLE_FFMPEG_INTERFACE)
#include "rx_ffmpeg_wrapper.h"
#include "realx/base/rx_logging.h"
#include "realx/base/rx_checks.h"
#include <dlfcn.h>
#include "realx/base/folder/rx_folder_path_config.h"

namespace realx {
RXFFmpegAPIs* load_api() {
    void* handle = dlopen("libRTCFFmpeg.so", RTLD_LOCAL | RTLD_LAZY);
    if (!handle) {
        const std::string& lib_path = rx_get_lib_path("RTCFFmpeg");
        if (lib_path.empty()) {
            RX_LOG(kRXLSError) << "RTCFFmpeg missing library";
            return nullptr;
        }
        RX_LOG(kRXLSWarning) << "dlopen RTCFFmpeg from: " << lib_path;
        handle = dlopen(lib_path.c_str(), RTLD_LOCAL | RTLD_LAZY);
        if (!handle) {
            RX_LOG(kRXLSError) << "dlopen RTCFFmpeg failed: " << dlerror();
            return nullptr;
        }
    }
    if (handle) {
        RX_LOG(kRXLSInfo) << "dlopen RTCFFmpeg success.";
    }

    auto load_symbol = [handle](auto* pointer, const char* name) -> bool {
        dlerror();  // reset errors
        void* p = dlsym(handle, name);
        const char* dlsym_error = dlerror();
        if (dlsym_error) {
            RX_LOG(kRXLSError) << "load " << name << " failed: " << dlsym_error;
            *pointer = nullptr;
        } else {
            *pointer = reinterpret_cast<decltype(*pointer)>(p);
        }
        return (*pointer != nullptr);
    };
#define LOAD_SYMBOL(pointer, name) (ret &= load_symbol(&(pointer), (#name)))
#define CHECK_RET(ff_apis)                                   \
    if (!ret) {                                              \
        delete ff_apis;                                      \
        ff_apis = nullptr;                                   \
        RX_CHECK(false) << "RXFFmpegAPIs missing symbols";   \
        return nullptr;                                      \
    }
    RXFFmpegAPIs* apis = new RXFFmpegAPIs();
    {
        bool ret = true;
        LOAD_SYMBOL(apis->_avcodec_alloc_context3, avcodec_alloc_context3);
        LOAD_SYMBOL(apis->_avcodec_parameters_to_context, avcodec_parameters_to_context);
        LOAD_SYMBOL(apis->_avcodec_open2, avcodec_open2);
        LOAD_SYMBOL(apis->_av_init_packet, av_init_packet);
        LOAD_SYMBOL(apis->_avcodec_close, avcodec_close);
        LOAD_SYMBOL(apis->_avcodec_free_context, avcodec_free_context);
        LOAD_SYMBOL(apis->_avcodec_decode_video2, avcodec_decode_video2);
        LOAD_SYMBOL(apis->_av_free_packet, av_free_packet);
        LOAD_SYMBOL(apis->_avpicture_fill, avpicture_fill);
        LOAD_SYMBOL(apis->_avpicture_get_size, avpicture_get_size);
        LOAD_SYMBOL(apis->_avcodec_align_dimensions, avcodec_align_dimensions);
        LOAD_SYMBOL(apis->_avcodec_find_decoder, avcodec_find_decoder);
        LOAD_SYMBOL(apis->_avcodec_send_packet, avcodec_send_packet);
        LOAD_SYMBOL(apis->_avcodec_receive_frame, avcodec_receive_frame);
        LOAD_SYMBOL(apis->_avcodec_find_decoder_by_name, avcodec_find_decoder_by_name);
        LOAD_SYMBOL(apis->_avcodec_receive_packet, avcodec_receive_packet);
        LOAD_SYMBOL(apis->_av_packet_unref, av_packet_unref);
        LOAD_SYMBOL(apis->_avcodec_find_encoder_by_name, avcodec_find_encoder_by_name);
        LOAD_SYMBOL(apis->_avcodec_send_frame, avcodec_send_frame);
        LOAD_SYMBOL(apis->_av_packet_alloc, av_packet_alloc);
        LOAD_SYMBOL(apis->_av_packet_free, av_packet_free);
        LOAD_SYMBOL(apis->_avcodec_register_all, avcodec_register_all);
#if defined(RX_ENABLE_RECORDING)
        LOAD_SYMBOL(apis->_av_packet_rescale_ts, av_packet_rescale_ts);
#endif
        LOAD_SYMBOL(apis->_av_base64_decode, av_base64_decode);
        LOAD_SYMBOL(apis->_av_frame_alloc, av_frame_alloc);
        LOAD_SYMBOL(apis->_av_image_check_size, av_image_check_size);
        LOAD_SYMBOL(apis->_av_buffer_create, av_buffer_create);
        LOAD_SYMBOL(apis->_av_buffer_get_opaque, av_buffer_get_opaque);
        LOAD_SYMBOL(apis->_av_frame_unref, av_frame_unref);
        LOAD_SYMBOL(apis->_av_hwdevice_ctx_create, av_hwdevice_ctx_create);
        LOAD_SYMBOL(apis->_av_hwframe_ctx_alloc, av_hwframe_ctx_alloc);
        LOAD_SYMBOL(apis->_av_hwframe_ctx_init, av_hwframe_ctx_init);
        LOAD_SYMBOL(apis->_av_buffer_unref, av_buffer_unref);
        LOAD_SYMBOL(apis->_av_hwframe_get_buffer, av_hwframe_get_buffer);
        LOAD_SYMBOL(apis->_av_opt_set_int, av_opt_set_int);
        LOAD_SYMBOL(apis->_av_frame_free, av_frame_free);
        LOAD_SYMBOL(apis->_av_hwdevice_ctx_alloc, av_hwdevice_ctx_alloc);
        LOAD_SYMBOL(apis->_av_hwdevice_ctx_init, av_hwdevice_ctx_init);
        LOAD_SYMBOL(apis->_av_strerror, av_strerror);
        LOAD_SYMBOL(apis->_av_frame_get_buffer, av_frame_get_buffer);
        LOAD_SYMBOL(apis->_av_buffer_ref, av_buffer_ref);
        LOAD_SYMBOL(apis->_av_frame_copy_props, av_frame_copy_props);
        LOAD_SYMBOL(apis->_av_frame_get_side_data, av_frame_get_side_data);
        LOAD_SYMBOL(apis->_av_frame_new_side_data, av_frame_new_side_data);
        LOAD_SYMBOL(apis->_av_hwframe_transfer_data, av_hwframe_transfer_data);
        LOAD_SYMBOL(apis->_av_free, av_free);
        LOAD_SYMBOL(apis->_av_malloc, av_malloc);
        LOAD_SYMBOL(apis->_av_image_get_buffer_size, av_image_get_buffer_size);
        LOAD_SYMBOL(apis->_av_image_copy_to_buffer, av_image_copy_to_buffer);
#if defined(RX_ENABLE_RECORDING)
        LOAD_SYMBOL(apis->_av_dict_set, av_dict_set);
        LOAD_SYMBOL(apis->_av_freep, av_freep);
        LOAD_SYMBOL(apis->_av_get_default_channel_layout, av_get_default_channel_layout);
        LOAD_SYMBOL(apis->_av_gettime_relative, av_gettime_relative);
        LOAD_SYMBOL(apis->_av_mallocz, av_mallocz);
#endif
#if defined(RX_ENABLE_FFMPEG_AVFORMAT)
        LOAD_SYMBOL(apis->_av_register_all, av_register_all);
        LOAD_SYMBOL(apis->_avformat_network_init, avformat_network_init);
        LOAD_SYMBOL(apis->_avformat_open_input, avformat_open_input);
        LOAD_SYMBOL(apis->_avformat_find_stream_info, avformat_find_stream_info);
        LOAD_SYMBOL(apis->_avformat_close_input, avformat_close_input);
        LOAD_SYMBOL(apis->_av_find_best_stream, av_find_best_stream);
        LOAD_SYMBOL(apis->_av_read_frame, av_read_frame);
        LOAD_SYMBOL(apis->_av_seek_frame, av_seek_frame);
#if defined(RX_ENABLE_RECORDING)
        LOAD_SYMBOL(apis->_av_interleaved_write_frame, av_interleaved_write_frame);
        LOAD_SYMBOL(apis->_av_write_trailer, av_write_trailer);
        LOAD_SYMBOL(apis->_avformat_alloc_output_context2, avformat_alloc_output_context2);
        LOAD_SYMBOL(apis->_avformat_free_context, avformat_free_context);
        LOAD_SYMBOL(apis->_avformat_new_stream, avformat_new_stream);
        LOAD_SYMBOL(apis->_avformat_write_header, avformat_write_header);
        LOAD_SYMBOL(apis->_avio_closep, avio_closep);
        LOAD_SYMBOL(apis->_avio_open, avio_open);
#endif
#endif
#if defined(RX_ENABLE_FFMPEG_SWSCALE)
        LOAD_SYMBOL(apis->_sws_getContext, sws_getContext);
        LOAD_SYMBOL(apis->_sws_scale, sws_scale);
        LOAD_SYMBOL(apis->_sws_freeContext, sws_freeContext);
#endif
        CHECK_RET(apis);
    }
    return apis;
}

const RXFFmpegAPIs* ffmpeg_api() {
    static const auto* apis = load_api();
    return apis;
}
}  // namespace realx
#endif