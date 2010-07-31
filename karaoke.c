/*
 * Karaoke Mode (Core part)
 * Use Gstreamer as backend to mix voice and music.
 */

#include "karaoke.h"

void kara_initial()
{
    GstElement *voice_input;
    GstElement *audio_convert;
    GstElement *input_queue;
    GstElement *input_mix;
    GstElement *file_sink;
    GstElement *ogg_mux;
    GstElement *input_vol;
    GstElement *music_vol;
    GstElement *audio_sink;
    GstElement *vorbis_enc;
    GstElement *recorder_pipeline;
    GstPad *input_src_pad;

    audio_sink = gst_element_factory_make("autoaudiosink", "audio_sink");
    voice_input = gst_element_factory_make("alsasrc", "voice_input");
    input_mix = gst_element_factory_make("adder", "mix");
    input_queue = gst_element_factory_make("queue", "input_queue");
    audio_convert = gst_element_factory_make("audioconvert", "audio_convert");
    input_vol =  gst_element_factory_make("volume", "input_volume");
    music_vol =  gst_element_factory_make("volume", "music_volume");
    recorder_pipeline = gst_pipeline_new("recorder_pipeline");

    gst_bin_add_many(GST_BIN(recorder_pipeline), voice_input, input_queue, 
        input_vol, input_mix, audio_sink, NULL);

    gst_element_link_many(voice_input, input_queue, input_vol, NULL);

    input_src_pad = gst_element_get_request_pad(input_mix, "sink%d");
    gst_pad_link(gst_element_get_pad(input_vol, "src"), input_src_pad);

    gst_element_link(input_mix, audio_sink);

    gst_element_set_state(recorder_pipeline, GST_STATE_NULL);
    gst_element_set_state(recorder_pipeline, GST_STATE_READY);
    //gst_element_set_state(recorder_pipeline, GST_STATE_PLAYING);
}
